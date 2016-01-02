// Module: Mcastws2.c
//
// Purpose:
//    This sample illustrates IP multicasting using the Winsock 2
//    method of joining and leaving a multicast group.  This sample
//    may be invoked as either a sender or a receiver.  In both 
//    cases, the specified multicast group is joined.
//
// Compile:
//    cl -o Mcastws2 Mcastws2.c Ws2_32.lib
//
// Command Line Options/Parameters
//    Mcastws2.exe [-s] [-m:str] [-p:int] [-i:str] [-l] [-n:int]
//       -s        Act as sender; otherwise receive data
//       -m:str    Dotted decimal IP multicast address to join
//       -p:int    Port number to use
//       -i:str    Local interface to use
//       -l        Disable the loopback 
//       -n:int    Number of messages to send or receive
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

#define MCASTADDR     "234.5.6.7"
#define MCASTPORT      25000
#define BUFSIZE        1024
#define DEFAULT_COUNT  500

BOOL  bSender = FALSE,      // Act as a sender?
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
    printf("   -s        Act as server (send data); otherwise\n");
    printf("              receive data.\n");
    printf("   -m:str    Dotted decimal multicast IP addres "
		 "to join\n");
    printf("              The default group is: %s\n", MCASTADDR);
    printf("   -p:int    Port number to use\n");
    printf("              The default port is: %d\n", MCASTPORT);
    printf("   -i:str    Local interface to bind to; by default \n");
    printf("              use INADDRY_ANY\n");
    printf("   -l        Disable loopback\n");
    printf("   -n:int    Number of messages to send/receive\n");
    ExitProcess(-1);
}

//
// Function: ValidateArgs
//
// Description
//    Parse the command line arguments and set some global flags
//    depeding on the values
//
void ValidateArgs(int argc, char **argv)
{
    int      i;

    dwInterface = INADDR_ANY;
    dwMulticastGroup = inet_addr(MCASTADDR);
    iPort = MCASTPORT;
    dwCount = DEFAULT_COUNT;

    for(i=1; i < argc ;i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 's':  // Sender
                    bSender = TRUE;
                    break;
                case 'm':  // Multicast group to join
                    if (strlen(argv[i]) > 3)
                        dwMulticastGroup = inet_addr(&argv[i][3]);
                    break;
                case 'i':  // Local interface to use
                    if (strlen(argv[i]) > 3)
                        dwInterface = inet_addr(&argv[i][3]);
                    break;
                case 'p':  // Port to use
                    if (strlen(argv[i]) > 3)
                        iPort = atoi(&argv[i][3]);
                    break;
                case 'l':  // Disable loopback
                    bLoopBack = TRUE;
                    break;
                case 'n':  // Number of messages to send/recv
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
//	   is run as a sender then begin sending messages to the multicast 
//    group; otherwise, call recvfrom() to read messages sent to the 
//    group.
// 
int main(int argc, char **argv)
{
    WSADATA             wsd;
    struct sockaddr_in  local,
                        remote,
                        from;
    SOCKET              sock, sockM;
    TCHAR               recvbuf[BUFSIZE],
                        sendbuf[BUFSIZE];
    int                 len = sizeof(struct sockaddr_in),
                        optval,
                        ret;
    DWORD               i=0;

    // Parse the command line and load Winsock
    //
    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("WSAStartup() failed\n");
        return -1;
    }
    // Create the socket. In Winsock 2 you do have to specify the
    // multicast attributes that this socket will be used with.
    //
    if ((sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
                  WSA_FLAG_MULTIPOINT_C_LEAF 
				  | WSA_FLAG_MULTIPOINT_D_LEAF 
				  | WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("socket failed with: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    // Bind to the local interface. This is done to receive data.
    local.sin_family = AF_INET;
    local.sin_port   = htons(iPort);
    local.sin_addr.s_addr = dwInterface;
    
    if (bind(sock, (struct sockaddr *)&local, 
        sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind failed with: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    // Setup the SOCKADDR_IN structure describing the multicast 
    // group we want to join
    //
    remote.sin_family      = AF_INET;
    remote.sin_port        = htons(iPort);
    remote.sin_addr.s_addr = dwMulticastGroup;
    //
    // Change the TTL to something more appropriate
    //
    optval = 8;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, 
        (char *)&optval, sizeof(int)) == SOCKET_ERROR)
    {
        printf("setsockopt(IP_MULTICAST_TTL) failed: %d\n",
            WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    // Disable loopback if needed
    //
    if (bLoopBack)
    {
        optval = 0;
        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
            (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
        {
            printf("setsockopt(IP_MULTICAST_LOOP) failed: %d\n",
                WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return -1;
        }
    }
    // Join the multicast group.  Note that sockM is not used 
    // to send or receive data. It is used when you want to 
    // leave the multicast group. You simply call closesocket() 
    // on it.
    //
    if ((sockM = WSAJoinLeaf(sock, (SOCKADDR *)&remote, 
        sizeof(remote), NULL, NULL, NULL, NULL, 
        JL_BOTH)) == INVALID_SOCKET)
    {
        printf("WSAJoinLeaf() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    if (!bSender)           // Receiver
    {
        // Receive data
        //
        for(i = 0; i < dwCount; i++)
        {
            if ((ret = recvfrom(sock, recvbuf, BUFSIZE, 0,
                (struct sockaddr *)&from, &len)) == SOCKET_ERROR)
            {
                printf("recvfrom failed with: %d\n", 
                    WSAGetLastError());
                closesocket(sockM);
                closesocket(sock);
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
        // Send data
        //
        for(i = 0; i < dwCount; i++)
        {
            sprintf(sendbuf, "server 1: This is a test: %d", i);
            if (sendto(sock, (char *)sendbuf, strlen(sendbuf), 0,
                (struct sockaddr *)&remote, 
                sizeof(remote)) == SOCKET_ERROR)
            {
                printf("sendto failed with: %d\n", 
                    WSAGetLastError());
                closesocket(sockM);
                closesocket(sock);
                WSACleanup();
                return -1;
            }
            Sleep(500);
        }
    }
    // Leave the multicast group by closing sock
    // For non-rooted control and data plane schemes, WSAJoinLeaf
    // returns the same socket handle that you pass into it.
    //
    closesocket(sock);

    WSACleanup();
    return -1;
}
