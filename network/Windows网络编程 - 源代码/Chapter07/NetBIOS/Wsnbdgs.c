// Module Name: wsnbdgs.c
//
// Description:
//    This sample is a Winsock NetBIOS datagram sample. This
//    sample includes sending and receiving datagrams using
//    unique names, group names, and broadcast.
//
// Compile:
//    cl -o wsnbdgs wsnbdgs.c ws2_32.lib
//
// Command Line Arguments/Options
//    usage: wsnbdgs [-n:str] [-g:str] [-p:int] [-s] [-c:int] [-b] [-d:int]
//           -n:str        Unique name to send to or register as
//           -g:str        Group name to send to or register as
//           -p:int        The 16th byte value for our name
//           -s            Act as sender (by default we receive
//                         If this flag is present -n or -g is the
//                          NetBIOS name to send to
//           -c:int        Number of packets to send
//           -b            Use broadcast packets
//           -d:int        Delay (in milliseconds) between sends
//
#include <winsock2.h>
#include <wsnetbs.h>

#include <stdio.h>
#include <stdlib.h>

#define INVALID_LANA         0x80000000
#define MAX_BUFFER                 1024
#define TEST_MESSAGE        "This is a test"

//
// Global variables for the name of our server and its
// 16th byte qualifier.
//
char    szServerName[NETBIOS_NAME_LENGTH];
int     iPort=0;
BOOL    bGroupName=FALSE,
        bSender=FALSE,
        bBroadcast=FALSE;
DWORD   dwNumIterations=20,
        dwInterval;

// 
// Function: usage
//
// Description:
//    Print usage information.
//
void usage()
{
    printf("usage: wsnbdgs [-n:str] [-g:str] [-p:int] [-s] [-c:int] [-b] [-d:int]\n");
    printf("       -n:str        Unique name to send to or register as\n");
    printf("       -g:str        Group name to send to or register as\n");
    printf("       -p:int        The 16th byte value for our name\n");
    printf("       -s            Act as sender (by default we receive\n");
    printf("                     If this flag is present -n or -g is the\n");
    printf("                      NetBIOS name to send to\n");
    printf("       -c:int        Number of packets to send\n");
    printf("       -b            Use broadcast packets\n");
    printf("       -d:int        Delay (in milliseconds) between sends\n");
    ExitProcess(0);
}

//
// Function: ValidateArgs
//
// Description:
//    Validate the command line arguments and set the approprate 
//    global flags dpending on them.
//
void ValidateArgs(int argc, char **argv)
{
    int                i;

    for(i=1; i < argc ;i++)
    {
        if (strlen(argv[i]) < 2)
            continue;
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'n':        // use a unique name
                    bGroupName = FALSE;
                    strncpy(szServerName, &argv[i][3], 
                        NETBIOS_NAME_LENGTH);
                    break;
                case 'g':        // usa a broadcast name
                    bGroupName = TRUE;
                    strncpy(szServerName, &argv[i][3],
                        NETBIOS_NAME_LENGTH);
                    break;
                case 'p':        // set the 16th byte
                    iPort = atoi(&argv[i][3]);
                    break;
                case 's':        // send datagrams
                    bSender = TRUE;
                    break;
                case 'c':        // number of iterations
                    dwNumIterations = atoi(&argv[i][3]);
                    break;
                case 'b':        // use broadcasts
                    bBroadcast = TRUE;
                    break;
                case 'd':        // delay between sends 
                    dwInterval = atoi(&argv[i][3]);
                    break;
                default:
                    usage();
                    break;
            }
        }
    }
}

//
// Function: FindProtocol
// 
// Description:
//    Search through the available network service providers for
//    AF_NETBIOS compatible protocols. The number of providers 
//    returned will be equal to 2 times the number of LANAs we
//    would have in NetBIOS. This is because there is two providers
//    for each LANA: one datagram and one session oriented provider.
//
BOOL FindProtocol(WSAPROTOCOL_INFO **wsapi, DWORD *dwCount)
{
    WSAPROTOCOL_INFO *lpProtocolBuf=NULL;
    DWORD             dwErr,
                      dwRet,
                      dwBufLen=0;
    int               i;

    *dwCount = 0;
    if (SOCKET_ERROR != WSAEnumProtocols(NULL, lpProtocolBuf, 
        &dwBufLen))
    {
        // This should never happen as there is a NULL buffer
        //
	printf("WSAEnumProtocols failed!\n");
	return FALSE;
    }
    else if (WSAENOBUFS != (dwErr = WSAGetLastError()))
    {
	// We failed for some reason not relating to buffer size - 
        // also odd
        //
	printf("WSAEnumProtocols failed: %d\n", dwErr);
	return FALSE;
    }
    // Allocate the correct buffer size for WSAEnumProtocols as
    // well as the buffer to return
    //
    lpProtocolBuf = (WSAPROTOCOL_INFO *)GlobalAlloc(GMEM_FIXED, 
        dwBufLen);
    *wsapi = (WSAPROTOCOL_INFO *)GlobalAlloc(GMEM_FIXED, dwBufLen);

    if ((lpProtocolBuf == NULL) || (*wsapi == NULL))
    {
	printf("GlobalAlloc failed: %d\n", GetLastError());
	return FALSE;
    }
    dwRet = WSAEnumProtocols(NULL, lpProtocolBuf, &dwBufLen);
    if (dwRet == SOCKET_ERROR)
    {
	printf("WSAEnumProtocols failed: %d\n", WSAGetLastError());
	GlobalFree(lpProtocolBuf);
	return FALSE;
    }
    // Loop through the returned protocol information looking for those
    // that are in the AF_NETBIOS address family.
    //
    for (i=0; i < dwRet ;i++)
    {
	if (lpProtocolBuf[i].iAddressFamily == AF_NETBIOS)
	{
            if (((lpProtocolBuf[i].dwServiceFlags1 & 
                        XP1_CONNECTIONLESS) == 1) &&
                ((lpProtocolBuf[i].dwServiceFlags1 & 
                        XP1_GUARANTEED_ORDER) == 0) &&
                ((lpProtocolBuf[i].dwServiceFlags1 & 
                        XP1_GUARANTEED_DELIVERY) == 0) &&
                 (lpProtocolBuf[i].iSocketType == SOCK_DGRAM))
            {
                (*wsapi)[(*dwCount)++] = lpProtocolBuf[i];
            }
	}
    }
    GlobalFree(lpProtocolBuf);
    return TRUE;
}

//
// Function: main
//
// Description:
//    This is the main function for the Winsock NetBIOS datagram
//    app. First parse the arguments and load Winsock. If we're
//    the receiver create a socket and register the given name 
//    (unique or group). And prepare to receive datagrams. If 
//    the -b option is set we will listen for broadcasts.  For 
//    senders, create a socket and attempt to connect. If we're
//    sending broadcasts we don't connect (just do straight sendto
//    calls). Then send the appropriate number of messages.
//
int main(int argc, char **argv)
{
    WSADATA           wsd;
    DWORD             dwNumProtocols,
                      dwRet,
                      dwSize,
                      dwFlags=0,
                      dwIndex,
                      dwErr;
    int               i, j, iOptVal;
    WSAPROTOCOL_INFO *wsapi=NULL;

    SOCKADDR_NB       nbaddr, 
                      addrClient;
    SOCKET           *socks=NULL;
    char              szRecvBuff[MAX_BUFFER];
    WSAEVENT         *hEvents=NULL;


    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("Unable to load Winsock!\n");
        return 1;
    }
    // Setup the SOCKADDR_NB structure for the given name
    //
    if (bGroupName)
    {
        SET_NETBIOS_SOCKADDR(&nbaddr, NETBIOS_GROUP_NAME, szServerName,
            iPort); 
    }
    else
    {
        SET_NETBIOS_SOCKADDR(&nbaddr, NETBIOS_UNIQUE_NAME, szServerName,
            iPort); 
    }
    // Find the NetBIOS providers that support datagrams
    //
    if (FindProtocol(&wsapi, &dwNumProtocols) != TRUE)
    {
        printf("Unable to find any AF_NETBIOS protocols!\n");
        return 1;
    }
    if (dwNumProtocols == 0)
    {
        printf("No NetBIOS capable providers found\n");
        return 1;
    }
    //
    // Allocate buffers for event handles and sockets
    //
    hEvents = (WSAEVENT *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                sizeof(WSAEVENT) * dwNumProtocols);
    if (hEvents == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    socks = (SOCKET *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                sizeof(SOCKET) * dwNumProtocols);
    if (socks == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    // Create the handles and each socket for each transport protocol
    //
    for(i=0; i < dwNumProtocols ;i++)
    {
        hEvents[i] = WSACreateEvent();
        if (hEvents[i] == NULL)
        {
            printf("WSACreatEvent() failed: %d\n", WSAGetLastError());
            return 1;
        }
        printf("Creating a socket for: '%s'\n", wsapi[i].szProtocol);

        socks[i] = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
             FROM_PROTOCOL_INFO, &wsapi[i], 0, WSA_FLAG_OVERLAPPED);
        if (socks[i] == INVALID_SOCKET)
        {
            printf("socket() failed: %d\n", WSAGetLastError());
            continue;
        }
        if (bBroadcast)
        {
            iOptVal = 1;
            if (setsockopt(socks[i], SOL_SOCKET, SO_BROADCAST, 
                    (char *)&iOptVal, sizeof(int)) == SOCKET_ERROR)
            {
                printf("setsockopt(SO_BROADCAST) failed: %d\n", WSAGetLastError());
                return 1;
            }
        }
        if ((bSender) && (!bBroadcast))
        {
            if (connect(socks[i], (SOCKADDR *)&nbaddr, sizeof(nbaddr)) == SOCKET_ERROR)
            {
                printf("connect() failed: %d\n", WSAGetLastError());
                return 1;
            }
            else
                printf("connect()\n");
            dwFlags = FD_WRITE;
            printf("FD_WRITE\n");
        }
        else if ((!bSender) && (!bBroadcast))
        {
            if ((dwRet = bind(socks[i], (SOCKADDR *)&nbaddr, 
                    sizeof(nbaddr))) == SOCKET_ERROR)
                printf("bind() failed: %d\n", WSAGetLastError());
            else 
                printf("bind()\n");
            dwFlags = FD_READ;
            printf("FD_READ\n");
        }
        else if (!bSender)
        {
            if ((dwRet = bind(socks[i], (SOCKADDR *)&nbaddr, 
                    sizeof(nbaddr))) == SOCKET_ERROR)
                printf("bind() failed: %d\n", WSAGetLastError());
            else 
                printf("bind()\n");
            dwFlags = FD_READ;
            printf("FD_READ\n");
        }
        else
        {
            dwFlags = FD_WRITE;
            printf("FD_WRITE\n");
        }
        if (WSAEventSelect(socks[i], hEvents[i], dwFlags) == SOCKET_ERROR)
        {
            printf("WSAEventSelect failed: %d\n", WSAGetLastError());
            return 1;
        }
        else
            printf("WSAEventSelect succeeded\n");
    }
    printf("after socket creation\n");
    if (bSender)
    {
        for(i=0; i < dwNumProtocols ;i++)
        {
            printf("Sending messages for: '%s'\n", wsapi[i].szProtocol);
            for(j=0; j < dwNumIterations ;j++)
            {
                if (bBroadcast)
                {
                    printf("%d ", j);
                    dwSize = sizeof(nbaddr);
                    dwRet = sendto(socks[i], TEST_MESSAGE, strlen(TEST_MESSAGE), 0,
                                (SOCKADDR *)&nbaddr, dwSize);
                    if (dwRet == SOCKET_ERROR)
                    {
                        if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                        {
                            j--;
                            Sleep(1000);
                            continue;
                        }
                        printf("sendto failed: %d\n", dwErr);
                        continue;
                    }
                }
                else
                {
                    printf("%d ", j);
                    dwRet = send(socks[i], TEST_MESSAGE, strlen(TEST_MESSAGE), 0);
                    if (dwRet == SOCKET_ERROR)
                    {
                        if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                        {
                            j--;
                            Sleep(1000);
                            continue;
                        }
                        printf("send() failed: %d\n", WSAGetLastError());
                        break;
                    }
                }
                Sleep(dwInterval);
            }
            printf("\n");
        }
    }
    else
    {
        for(i=0; i < dwNumIterations ;i++)
        {
            dwIndex = WSAWaitForMultipleEvents(dwNumProtocols, hEvents, FALSE,
                        WSA_INFINITE, FALSE);
            if (dwIndex == WSA_WAIT_FAILED)
            {
                if ((dwErr = WSAGetLastError()) != WSAEWOULDBLOCK)
                {
                    printf("Wait failed: %d\n", dwErr);
                    return 1;
                }
            }
            dwSize = sizeof(addrClient);

            if (bBroadcast)
                dwRet = recvfrom(socks[dwIndex], szRecvBuff, MAX_BUFFER, 0, 
                                (SOCKADDR *)&addrClient, &dwSize);
            else
                dwRet = recv(socks[dwIndex], szRecvBuff, MAX_BUFFER, 0);
            if (dwRet == SOCKET_ERROR)
            {
                if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                {
                    i--;
                    continue;
                }
                printf("recv() failed with: %d\n", dwErr);
            }
            else
            { 
                szRecvBuff[dwRet] = 0;
                printf("%03d READ [%d]: '%s'\n", i, dwIndex, szRecvBuff);
            }
        }
    }

    for(i=0; i < dwNumProtocols ;i++)
    {
        closesocket(socks[i]);
        WSACloseEvent(hEvents[i]);
    }
    GlobalFree(socks);
    GlobalFree(hEvents);
    GlobalFree(wsapi);

    WSACleanup();

    return 0;
}
