// Module Name: wsnbclnt.c
//
// Description:
//    This sample illustrates how to use the AF_NETBIOS protocol family
//    from a Winsock application. This particular sample is a client 
//    that communicates with to the server (wsnbsvr.c) sample.
//
// Compile:
//    cl -o wsnbclnt wsnbclnt.c wsnbdef.obj ws2_32.lib
//
// Command Line Parameters/Options:
//    wsnbclnt.exe options
//
//    -n:str  - Server's Netbios name 
//    -p:int  - Use this integer value as the 16th byte of server name
//    -l:int  - Attempt a connection on this LANA number only, the
//              default behavior is to attempt a connection on all
//              LANA numbers
//    -t:char - Specifies datagram (d) or seqpacket (s)
//    -c:int  - Number of types to send the message
//    -b:int  - Size of buffer to send
//
#include <winsock2.h>
#include <wsnetbs.h>

#include <stdio.h>
#include <stdlib.h>

//
// A couple common definitions used between client and server
//
#include "wsnbdef.h"

char    szServerName[NETBIOS_NAME_LENGTH];   // Our NetBIOS name
int     iPort,                               // Our 16th byte
        iLana,                               // LANA to connect on
        iSocketType=SOCK_SEQPACKET;          // Datagram or stream?
DWORD   dwCount=DEFAULT_COUNT,               // How many packets
        dwSize=MAX_BUFFER;                   // Size of buffer to send
BOOL    bOneLana=FALSE;                      // Connect on one LANA

//
// Function: usage
// 
// Description:
//    Print out usage information.
//
void usage()
{
    printf("usage: wsnbclnt -n:str -p:int -l:int -t:char\n");
    printf("       -n:NAME        Our NetBIOS name\n");
    printf("       -p:PORT        The 16th byte qualifier of our name\n");
    printf("       -l:LANA        Specifies to connect on this LANA only\n");
    printf("                       By default attempt connection on all LANAs\n");
    printf("       -t:TYPE        Specifies datagram (d) or seqpacket (s)\n");
    printf("       -c:COUNT       Number of types to send the message\n");
    printf("       -b:SIZE        Size of buffer to send\n");
    ExitProcess(1);
}

//
// Function: ValidateArgs
//
// Description
//    Parse the argument list for our NetBIOS name and whether
//    we want to operate on all LANAs or just one.
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
                    if (strlen(argv[i]) > 3)
                        strncpy(szServerName, &argv[i][3], 
                            NETBIOS_NAME_LENGTH);
                    break;
                case 'p':        // set the 16th byte
                    if (strlen(argv[i]) > 3)
                        iPort = atoi(&argv[i][3]);
                    break;
                case 'l':        // connect on one lana only
                    bOneLana = TRUE;
                    if (strlen(argv[i]) > 3)
                        iLana = atoi(&argv[i][3]);
                    break;
                case 't':        // datagram or stream socket?
                    if (strlen(argv[i]) > 3)
                    {
                        if (tolower(argv[i][3]) == 's')
                            iSocketType = SOCK_SEQPACKET;
                        else if (tolower(argv[i][3]) == 'd')
                            iSocketType = SOCK_DGRAM;
                        else
                            usage();
                    }
                    break;
                case 'c':
                    if (strlen(argv[i]) > 3)
                        dwCount = atol(&argv[i][3]);
                    break;
                case 'b':
                    if (strlen(argv[i]) > 3)
                        dwSize = (atol(&argv[i][3]) > MAX_BUFFER ? 
                                MAX_BUFFER : atol(&argv[i][3]));
                    break;
                default:
                    usage();
                    break;
            }
        }
    }
    return;
}

//
// Function: main
//
// Description:
//    This function parses the command line arguments, loads the
//    Winsock library, and connects to a server. By default, a
//    connection is attempted on all available LANA numbers. The
//    first one to succeed is used and the others are cancelled.
//
int main(int argc, char **argv)
{
    WSADATA           wsd;
    SOCKET           *socks=NULL;           // array of socket handles
    WSAEVENT         *hEvents=NULL;         // events for each socket
    SOCKADDR_NB       nbaddr;               // NetBIOS addr of server
    WSAPROTOCOL_INFO *wsapi=NULL;
    DWORD             dwRet,
                      dwNumProtocols,
                      dwIndex,
                      dwErr;
    char              szMessage[MAX_BUFFER];   // Data buffer
    int               i;
    unsigned long     iOptVal;
    struct fd_set     fdread;

    // Validate arguments
    //
    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("Unable to load Winsock!\n");
        return 1;
    }
    // If we're connecting on all LANAs enumerate all AF_NETBIOS
    //  protocols; if not we can just specify the LANA we want
    //  when creating the socket.
    //
    if (bOneLana == FALSE)
    {
        // This function will return an array of WSAPROTOCOL_INFO
        //  structures that match our socket type
        //
        if (FindProtocol(&wsapi, &dwNumProtocols) != TRUE)
        {
            printf("Unable to find correct protocol!\n");
            return 1;
        }
        if (dwNumProtocols == 0)
        {
            printf("No NetBIOS capable providers found!\n");
            return 1;
        }
    }
    else
    {
        dwNumProtocols = 1;
    }
    // Setup the NetBIOS address name
    //
    SET_NETBIOS_SOCKADDR(&nbaddr, NETBIOS_UNIQUE_NAME, szServerName, 
        iPort); 
    //
    // Allocate some global structures. 
    //  socks : array of SOCKET handles for each transport we connect on
    //  hEvents : array of WSAEVENT handles for event notification
    //
    socks = (SOCKET *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                sizeof(SOCKET) * dwNumProtocols);
    if (socks == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    hEvents = (WSAEVENT *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                sizeof(WSAEVENT) * dwNumProtocols);
    if (hEvents == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    // For each LANA, create a WSAEVENT, create the SOCKET, and
    //  register it for what events we want to receive.
    //
    for(i=0; i < dwNumProtocols ;i++)
    {
        if (bOneLana)
            printf("Transport LANA: %d\n", iLana);
        else
            printf("Transport: '%s'\n", wsapi[i].szProtocol);
        hEvents[i] = WSACreateEvent();
        if (hEvents[i] == NULL)
        {
            printf("WSACreateEvent failed: %d\n", WSAGetLastError());
            return 1;
        }
        if (bOneLana == FALSE)
            socks[i] = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
                        FROM_PROTOCOL_INFO, &wsapi[i], 0, WSA_FLAG_OVERLAPPED);
        else
            socks[i] = WSASocket(AF_NETBIOS, SOCK_SEQPACKET, -iLana, NULL,
                         0, WSA_FLAG_OVERLAPPED); 

        if (socks[i] == INVALID_SOCKET)
        {
            printf("socket() failed: %d\n", WSAGetLastError());
            return 1; 
        }

        if (WSAConnect(socks[i], (SOCKADDR *)&nbaddr, sizeof(nbaddr),
                NULL, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("WSACOnnect failed: %d\n", WSAGetLastError());
            continue;
        }
        if (WSAEventSelect(socks[i], hEvents[i], FD_CONNECT) == 
            SOCKET_ERROR)
        {
            printf("WSAEventSelect failed: %d\n", WSAGetLastError());
            return 1;
        } 
    }
    // Wait for one of the connects to succeed
    //
    dwIndex = WSAWaitForMultipleEvents(dwNumProtocols, hEvents, FALSE, 
                  WSA_INFINITE, FALSE);
    if (dwIndex == WSA_WAIT_FAILED)
    {
        printf("WSAWaitForMultipleEvents failed: %d\n", WSAGetLastError());
        return 1;
    }
    // Close the sockets of all other pending connections other than the
    //  one that completed first
    //
    dwIndex -= WAIT_OBJECT_0;
    for(i=0; i < dwNumProtocols ;i++)
    {
        if (i != dwIndex)
            closesocket(socks[i]);
    }
    // Put the socket in non-blocking mode
    //
    iOptVal = 1L;
    if (ioctlsocket(socks[dwIndex], FIONBIO, &iOptVal) == SOCKET_ERROR)
    {
        printf("ioctlsocket(FIONBIO) failed: %d\n", WSAGetLastError());
    }
    memset(szMessage, 'a', dwSize);
    for(i=0; i < dwCount ;i++)
    {
        // Send the message to the server
        //
        dwRet = send(socks[dwIndex], szMessage, dwSize, 0);
        if (dwRet == 0)
        {
            printf("graceful close\n");
            closesocket(socks[dwIndex]);
            return 0;
        }
        else if (dwRet == SOCKET_ERROR)
        {
            if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
            {
                i--;
                continue;
            }
            else
            {
                printf("send failed: %d\n", dwErr);
                return 1;
            }
        }
        printf("Wrote %d bytes\n", dwRet);
        //
        // Wait until the server echoes the data back. This really doesn't 
        //  matter when using SOCK_SEQPACKET, but if we're using SOCK_DGRAM
        //  then our recv() would fail with WSAEWOULDBLOCK and we'd skip
        //  the returned data (as the server might not have sent it yet)
        //
        FD_ZERO(&fdread);
        FD_SET(socks[dwIndex], &fdread);
        select(0, &fdread, NULL, NULL, NULL);

        //
        // Read the message back
        //
        dwRet = recv(socks[dwIndex], szMessage, MAX_BUFFER, 0);
        if (dwRet == 0)
        {
            printf("graceful close\n");
            closesocket(socks[dwIndex]);
            return 0;
        }
        else if (dwRet == SOCKET_ERROR)
        {
            if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
            {
                i--;
                continue;
            }
            else
            {
                printf("recv() failed: %d\n", dwErr);
                return 1;
            }
        }
        printf("Read %d bytes\n", dwRet);
    }

    closesocket(socks[dwIndex]);

    for(i=0; i < dwNumProtocols ;i++)
        WSACloseEvent(hEvents[i]);

    GlobalFree(wsapi);
    GlobalFree(socks);
    GlobalFree(hEvents);

    WSACleanup();
    return 0;
}
