// Module: Mcastws1.c
//
// Purpose:
//    This sample illustrates IP multicasting using the Winsock 1
//    method of joining and leaving an multicast group.  This sample
//    may be invoked as either a sender or a receiver. In both cases,
//    the specified multicast group is joined.
//
// Compile:
//    cl -o Mcastws1 Mcastws1.c Wsock32.lib
//
// Command Line Options/Parameters
//    mcastws1.exe [-s] [-m:str] [-p:int] [-i:str] [-l] [-n:int]
//       -s        Act as sender; otherwise receive data
//       -m:str    Dotted decimal IP multicast address to join
//       -p:int    Port number to use
//       -i:str    Local interface to use
//       -l        Disable the loopback 
//       -n:int    Number of messages to send or receive
//
#include <windows.h>
#include <winsock.h>

#include <stdio.h>
#include <stdlib.h>

#define MCASTADDR     "234.5.6.7"
#define MCASTPORT      25000
#define BUFSIZE        1024
#define DEFAULT_COUNT  500

BOOL  bSender = FALSE,      // Act as sender?
      bLoopBack = FALSE;    // Disable loopback?

DWORD dwInterface,          // Local interface to bind to
      dwMulticastGroup,     // Multicast group to join
      dwCount;              // Number of messages to send/receive

short iPort;                // Port number to use

// 
// Function: usage
//
// Description:
//    Print usage information and exit
//
void usage(char *progname)
{
    printf("usage: %s -s -m:str -p:int -i:str -l -n:int\n",
        progname);
    printf(" -s     Act as server (send data); otherwise\n");
    printf("          receive data.\n");
    printf(" -m:str Dotted decimal multicast IP addres to join\n");
    printf("          The default group is: %s\n", MCASTADDR);
    printf(" -p:int Port number to use\n");
    printf("          The default port is: %d\n", MCASTPORT);
    printf(" -i:str Local interface to bind to; by default \n");
    printf("          use INADDRY_ANY\n");
    printf(" -l     Disable loopback\n");
    printf(" -n:int Number of messages to send/receive\n");
    ExitProcess(-1);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments and set some global flags,
//    depending on the values
//
void ValidateArgs(int argc, char **argv)
{
    int      i;

    dwInterface = INADDR_ANY;
    dwMulticastGroup = inet_addr(MCASTADDR);
    iPort = MCASTPORT;
    dwCount = DEFAULT_COUNT;

    for(i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 's':        // Sender
                    bSender = TRUE;
                    break;
                case 'm':        // Multicast group to join
                    if (strlen(argv[i]) > 3)
                        dwMulticastGroup = inet_addr(&argv[i][3]);
                    break;
                case 'i':        // Local interface to use
                    if (strlen(argv[i]) > 3)
                        dwInterface = inet_addr(&argv[i][3]);
                    break;
                case 'p':        // Port number to use
                    if (strlen(argv[i]) > 3)
                        iPort = atoi(&argv[i][3]);
                    break;
                case 'l':        // Disable loopback?
                    bLoopBack = TRUE;
                    break;
                case 'n':        // Number of messages to send/recv
                    dwCount = atoi(&argv[i][3]);
                    break;
                default:
                    usage(argv[0]);
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
//    Parse the command line arguments, load the Winsock library, 
//    create a socket and join the multicast group. If this program
//    is started as a sender then begin sending messages to the 
//    multicast group; otherwise, call recvfrom() to read messages 
//    sent to the group.
//    
int main(int argc, char **argv)
{
    WSADATA             wsd;
    struct sockaddr_in  local,
                        remote,
                        from;
    struct ip_mreq      mcast;
    SOCKET              sockM;
    TCHAR               recvbuf[BUFSIZE],
                        sendbuf[BUFSIZE];
    int                 len = sizeof(struct sockaddr_in),
                        optval,
                        ret;
    DWORD               i=0;

    // Parse the command line and load Winsock
    //
    ValidateArgs(argc, argv);
     
    if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0)
    {
        printf("WSAStartup failed\n");
        return -1;
    }
    // Create the socket. In Winsock 1 you don't need any special
    // flags to indicate multicasting.
    //
    if ((sockM = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("socket failed with: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    // Bind the socket to the local interface. This is done so 
	 // that we can receive data.
    //
    local.sin_family = AF_INET;
    local.sin_port   = htons(iPort);
    local.sin_addr.s_addr = dwInterface;

    if (bind(sockM, (struct sockaddr *)&local, 
        sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind failed with: %d\n", WSAGetLastError());
        closesocket(sockM);
        WSACleanup();
        return -1;
    }

    // Setup the im_req structure to indicate what group we want
    // to join as well as the interface
    //
    remote.sin_family      = AF_INET;
    remote.sin_port        = htons(iPort);
    remote.sin_addr.s_addr = dwMulticastGroup;

    mcast.imr_multiaddr.s_addr = dwMulticastGroup;
    mcast.imr_interface.s_addr = dwInterface;

    if (setsockopt(sockM, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        (char *)&mcast, sizeof(mcast)) == SOCKET_ERROR)
    {
        printf("setsockopt(IP_ADD_MEMBERSHIP) failed: %d\n",
            WSAGetLastError());
        closesocket(sockM);
        WSACleanup();
        return -1;
    }
    // Set the TTL to something else. The default TTL is 1.
    //
    optval = 8;
    if (setsockopt(sockM, IPPROTO_IP, IP_MULTICAST_TTL, 
        (char *)&optval, sizeof(int)) == SOCKET_ERROR)
    {
        printf("setsockopt(IP_MULTICAST_TTL) failed: %d\n",
            WSAGetLastError());
        closesocket(sockM);
        WSACleanup();
        return -1;
    }
    // Disable the loopback if selected. Note that on NT4 and Win95
    // you cannot disable it.
    //
    if (bLoopBack)
    {
        optval = 0;
        if (setsockopt(sockM, IPPROTO_IP, IP_MULTICAST_LOOP,
            (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
        {
            printf("setsockopt(IP_MULTICAST_LOOP) failed: %d\n",
                WSAGetLastError());
            closesocket(sockM);
            WSACleanup();
            return -1;
        }
    }

    if (!bSender)           // Receiver
    {
        // Receive some data
        //
        for(i = 0; i < dwCount; i++)
        {
            if ((ret = recvfrom(sockM, recvbuf, BUFSIZE, 0,
                (struct sockaddr *)&from, &len)) == SOCKET_ERROR)
            {
                printf("recvfrom failed with: %d\n", 
                    WSAGetLastError());
                closesocket(sockM);
                WSACleanup();
                return -1;
            }
            recvbuf[ret] = 0;
            printf("RECV: '%s' from <%s>\n", recvbuf, 
                inet_ntoa(from.sin_addr));
        }
    }
    else                    // Sender
    {
        // Send some data
        //
        for(i = 0; i < dwCount; i++)
        {
            sprintf(sendbuf, "server 1: This is a test: %d", i);
            if (sendto(sockM, (char *)sendbuf, strlen(sendbuf), 0,
                (struct sockaddr *)&remote, 
                sizeof(remote)) == SOCKET_ERROR)
            {
                printf("sendto failed with: %d\n", 
                    WSAGetLastError());
                closesocket(sockM);
                WSACleanup();
                return -1;
            }
            Sleep(500);
        }
    }
    // Drop group membership
    //
    if (setsockopt(sockM, IPPROTO_IP, IP_DROP_MEMBERSHIP,
        (char *)&mcast, sizeof(mcast)) == SOCKET_ERROR)
    {
        printf("setsockopt(IP_DROP_MEMBERSHIP) failed: %d\n",
            WSAGetLastError());
    }
    closesocket(sockM);

    WSACleanup();
    return 0;
}
