// Module Name: wsnbsvr.c
//
// Description:
//    This sample illustrates creating a Winsock server using 
//    the AF_NETBIOS address family.  The server creates a listening
//    socket for each LANA number of the given socket type.
//    For datagrams, each listening socket waits for incoming data.
//    For connection-oriented communication, the server waits
//    for client connections at which point a thread is created
//    to service the connection.
//
// Compile:
//    cl /c wsnbdef.c
//    cl -o wsnbsvr wsnbsvr.c wsnbdef.obj ws2_32.lib
//
// Command Line Parameters/Options:
//    wsnbsvr [-n:str] [-p:int] [-l:int] [-t:char] [-c:int] [-b:int]
//            -n:NAME        Our NetBIOS name\n");
//            -p:PORT        The 16th byte qualifier of our name
//            -l:LANA        Specifies to listen on this LANA only
//                           By default listen on all LANAs
//            -t:TYPE        Specifies datagram (d) or seqpacket (s)
//            -c:COUNT       Number of types to receive per client
//            -b:SIZE        Size of buffer to receive
//
#include <winsock2.h>
#include <wsnetbs.h>

#include <stdio.h>
#include <stdlib.h>

//
// A couple common definitions used between client and server
//
#include "wsnbdef.h"

//
// Global variables for the name of our server and its
// 16th byte qualifier.
//
char    szServerName[NETBIOS_NAME_LENGTH];   // Our NetBIOS name
int     iPort,                               // Our 16th byte
        iLana,                               // LANA to listen on
        iSocketType=SOCK_SEQPACKET;          // Socket type
DWORD   dwCount=DEFAULT_COUNT,               // How many packets
        dwSize=MAX_BUFFER;                   // Receive buffer size
BOOL    bOneLana=FALSE;                      // Listen on one lana only

//
// Function: usage
// 
// Description:
//    Print out usage information.
//
void usage()
{
    printf("usage: wsnbsvr -n:str -p:int -l:int -t:char -c:int -b:int\n");
    printf("       -n:NAME        Our NetBIOS name\n");
    printf("       -p:PORT        The 16th byte qualifier of our name\n");
    printf("       -l:LANA        Specifies to listen on this LANA only\n");
    printf("                       By default listen on all LANAs\n");
    printf("       -t:TYPE        Specifies datagram (d) or seqpacket (s)\n");
    printf("       -c:COUNT       Number of types to receive per client\n");
    printf("       -b:SIZE        Size of buffer to receive\n");
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
                case 'l':        // listen on one lana only
                    if (strlen(argv[i]) > 3)
                        iLana = atoi(&argv[i][3]);
                    bOneLana = TRUE;
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
                case 'c':        // number of messages to send
                    if (strlen(argv[i]) > 3)
                        dwCount = atol(&argv[i][3]);
                    break;
                case 'b':        // size of data to send
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
// Function: ClientThread
//
// Description:
//    This thread is spawned for each incoming client connection to
//    handle the echo server responsiblities.
//
DWORD WINAPI ClientThread(LPVOID lpParam)
{
    SOCKET        sock = (SOCKET)lpParam;
    char          szRecvBuff[MAX_BUFFER];
    DWORD         dwRet,
                  dwErr;
    unsigned long iOptVal = 1L;

    // Make the client connection non-blocking
    //
    if (ioctlsocket(sock, FIONBIO, &iOptVal) == SOCKET_ERROR)
    {
        printf("ioctlsocket(FIONBIO) failed: %d\n", WSAGetLastError());
    }
    // In a loop read and write the data from and to the client
    //
    while (1)
    {
        dwRet = recv(sock, szRecvBuff, MAX_BUFFER, 0);
        if (dwRet == 0)
        {
            printf("Graceful close\n");
            return 0;
        }
        else if (dwRet == SOCKET_ERROR)
        {
            // If we get a WSAEWOULDBLOCK just keep going
            //
            if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                continue;
            else if (dwErr == WSAECONNRESET)
            {
                printf("Client aborted connection...\n");
                return 0;
            }
            else
            {
                printf("recv() failed: %d\n", dwErr);
                return 1;
            }
        }
        szRecvBuff[dwRet] = 0;
        printf("Read %d bytes\n", dwRet);

        while (1)
        {
            dwRet = send(sock, szRecvBuff, dwRet, 0);
            if (dwRet == SOCKET_ERROR)
            {
                if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                    continue;
                else if (dwErr == WSAECONNRESET)
                    break;
                printf("send() failed: %d\n", dwErr);
                return 1;
            }
            break;
        }
    }
    closesocket(sock);

    return 0;
}

//
// Function: main
//
// Description:
//    Parse the command line parameters, load the Winsock library,
//    enumerate the LANAs, and listen according to what is specified
//    by the command line. By default, a listen will be posted on
//    every available LANA. Once a client connection is made, spawn
//    a thread to handle that connection.
//
int main(int argc, char **argv)
{
    WSADATA           wsd;
    HANDLE            hThread;
    DWORD             dwThreadId,
                      dwNumProtocols,
                      dwIndex,
                      dwErr;
    int               i, 
                      iLastByte,
                      addrlen = sizeof(SOCKADDR_NB),
                      iEvents;
    WSAPROTOCOL_INFO *wsapi=NULL;
    SOCKET           *sockListen=NULL,       // array of listening sockets
                      sockClient; 
    WSAEVENT         *hEvents=NULL;          // one event for each LANA
    SOCKADDR_NB       nbaddr,                // our NetBIOS address
                      nbclient;              // client's NetBIOS address
    BOOL              bDone;

    // Parse command line and load Winsock library
    //
    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("Unable to load Winsock!\n");
        return 1;
    }
    // If we are listening on all LANAs enumerate them
    //
    if (!bOneLana)
    {
        if (FindProtocol(&wsapi, &dwNumProtocols) != TRUE)
        {
            printf("Unable to find correct protocol!\n");
            return 1;
        }
        if (dwNumProtocols == 0)
        {
            printf("No NetBIOS capable providers found\n");
            return 1;
        }
    }
    else
        dwNumProtocols = 1;

    SET_NETBIOS_SOCKADDR(&nbaddr, NETBIOS_UNIQUE_NAME, szServerName, 
        iPort); 
    //
    // Allocate a SOCKET handle for each LANA we are going to listen on
    //
    sockListen = (SOCKET *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                        sizeof(SOCKET) * dwNumProtocols);
    if (sockListen == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    //
    // Allocate an event handle for each LANA we are going to listen on
    //
    hEvents = (WSAEVENT *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                        sizeof(WSAEVENT) * dwNumProtocols);
    if (hEvents == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    // For each service provider create a ServerThread except for 
    // the last provider which we'll start in this main thread.
    //
    for(i=0; i < dwNumProtocols ;i++)
    {
        if (!bOneLana)
            printf("Transport: '%s'\n", wsapi[i].szProtocol);
        else
            printf("Transport: LANA: %d\n", iLana);

        hEvents[i] = WSACreateEvent();
        if (hEvents[i] == NULL)
        {
            printf("WSACreateEvent failed: %d\n", WSAGetLastError());
            continue;
        }
        // Create socket(s)
        //
        if (!bOneLana)
            sockListen[i] = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
                    FROM_PROTOCOL_INFO, &wsapi[i], 0, WSA_FLAG_OVERLAPPED);
        else
            sockListen[i] = WSASocket(AF_NETBIOS, SOCK_SEQPACKET, -iLana,
                    NULL, 0, WSA_FLAG_OVERLAPPED);
        if (sockListen[i] == SOCKET_ERROR)
        {
            printf("WSASocket failed: %d\n", WSAGetLastError());
            continue;
        }
        // Bind and listen each socket to our server name
        //
        if (bind(sockListen[i], (SOCKADDR *)&nbaddr, 
            sizeof(nbaddr)) == SOCKET_ERROR)
        {
            printf("bind() failed: %d\n", WSAGetLastError());
            return 1;
        }
        listen(sockListen[i], 10);
        //
        // We're only interested in FD_ACCEPT events for our server
        //  sockets
        //
        if (iSocketType == SOCK_SEQPACKET)
            iEvents = FD_ACCEPT;
        else
            iEvents = FD_READ;
        if (WSAEventSelect(sockListen[i], hEvents[i], iEvents) == 
                SOCKET_ERROR)
        {
            printf("WSAEventSelect failed: %d\n", WSAGetLastError());
            return 1;
        }
    }

    bDone = FALSE;
    while (!bDone)
    {
        // Wait until a client connection is pending
        //
        dwIndex = WSAWaitForMultipleEvents(dwNumProtocols, hEvents, FALSE, 
                        WSA_INFINITE, FALSE);
        if (dwIndex == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForEvents failed: %d\n", WSAGetLastError());
            return 1;
        }
        addrlen = sizeof(nbclient);
        if (iSocketType == SOCK_SEQPACKET)
        {
            sockClient = accept(sockListen[dwIndex], (SOCKADDR *)&nbclient, 
                                &addrlen);
            if (sockClient == INVALID_SOCKET)
            {
                if ((dwErr = WSAGetLastError()) != WSAEWOULDBLOCK)
                {
                    printf("accept() failed: %d\n", dwErr);
                    return 1;
                }
                else
                    continue;
            }
            // Print out the client name who connected
            //
            iLastByte = nbclient.snb_name[NETBIOS_NAME_LENGTH-1];
            nbclient.snb_name[NETBIOS_NAME_LENGTH-1];
            printf("Client '%s<%02d>' connected\n", nbclient.snb_name, iLastByte);
            //
            // Create a thread to handle the connection
            //
            hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)sockClient,
                    0, &dwThreadId);
            if (hThread == NULL)
            {
                printf("CreateThread() failed: %d\n", GetLastError());
                return 1;
            }
            CloseHandle(hThread);

            sockClient = INVALID_SOCKET;
        }
        else
        {
            char    recvBuff[MAX_BUFFER];
            DWORD   dwRet;

            dwRet = recvfrom(sockListen[dwIndex], recvBuff, MAX_BUFFER, 0,
                        (SOCKADDR *)&nbclient, &addrlen);
            if (dwRet == SOCKET_ERROR)
            {
                if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                    continue;
                printf("recvfrom() failed: %d\n", dwErr);
                return 1;
            }
            iLastByte = nbclient.snb_name[NETBIOS_NAME_LENGTH-1];
            nbclient.snb_name[NETBIOS_NAME_LENGTH-1];

            printf("Read %d bytes from '%s'<%02d>\n", dwRet, nbclient.snb_name,
                iLastByte);

            dwRet = sendto(sockListen[dwIndex], recvBuff, dwRet, 0,
                        (SOCKADDR *)&nbclient, sizeof(nbclient));
            if (dwRet == SOCKET_ERROR)
            {
                if ((dwErr = WSAGetLastError()) == WSAEWOULDBLOCK)
                    continue;
                printf("sendto() failed: %d\n", dwErr);
                return 1;
            }
            printf("Wrote %d bytes\n", dwRet);
        }
        WSAResetEvent(hEvents[dwIndex]);
    }
    // Clean up things
    //
    for(i=0; i < dwNumProtocols ;i++)
    {
        closesocket(sockListen[i]);
        WSACloseEvent(hEvents[i]);
    }

    GlobalFree(wsapi);
    GlobalFree(hEvents);
    GlobalFree(sockListen);

    WSACleanup();
    return 0;
}
