// Module: bcast.c
//
// Description:
//    This sample illustrates how to send and receive
//    broadcast data. The SO_BROADCASt option only needs
//    to be set on the sending socket; however in reality,
//    the Microsoft provider's don't check to see if the 
//    option is set. In other words you could send data
//    to INADDR_BROADCAST without the option set and
//    no error would occur.  Of course, this could change
//    in the future!
//
// Compile:
//    cl bcast.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    bcast.exe s|r
//      s      Send broadcast data
//      r      Receive data
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
//
// Description:
//    The main function does everything: loads the Winsock library,
//    parses the arguments, sets the broadcast option, and either
//    sends or receives data depending on the command line options.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    SOCKADDR_IN   to;

    int           ret,
                  fromsz;
    BOOL          bOpt,
                  bSender;
    char         *msg="This is a test",
                  rcvbuf[1024];

    // Check for arguments
    //
    if (argc != 2)
    {
        printf("usage: %s s|r\n", argv[0]);
        printf("    s  =  send\n");
        printf("    r  =  receive\n");
        return -1;
    }
    if (tolower(argv[1][0]) == 's')
        bSender = TRUE;
    else
        bSender = FALSE;
    //
    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Create a UDP socket
    //
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Set the broadcast option (really only necessary for the 
    // sender.
    //
    bOpt = TRUE;
    ret = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&bOpt,
            sizeof(bOpt));
    if (ret == SOCKET_ERROR)
    {
        printf("setsockopt(SO_BROADCAST) failed: %d\n",
            WSAGetLastError());
        return -1;
    }

    if (bSender)
    {
        // Send some data
        //
        to.sin_family = AF_INET;
        to.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        to.sin_port = htons(5150);

        ret = sendto(s, msg, strlen(msg), 0, (SOCKADDR *)&to, sizeof(to));
        if (ret == SOCKET_ERROR)
        {
            printf("sendto() failed: %d\n", WSAGetLastError());
            return -1;
        }
    }
    else
    {
        // Receive data so we must bind first
        //
        to.sin_family = AF_INET;
        to.sin_addr.s_addr = htonl(INADDR_ANY);
        to.sin_port = htons(5150);
        
        ret = bind(s, (SOCKADDR *)&to, sizeof(to));
        if (ret == SOCKET_ERROR)
        {
            printf("bind() failed: %d\n", WSAGetLastError());
            return -1;
        }
        // Now we can read data
        //
        fromsz = sizeof(to);
        ret = recvfrom(s, rcvbuf, 1024, 0, (SOCKADDR *)&to, &fromsz);
        if (ret == SOCKET_ERROR)
        {
            printf("recvfrom() failed: %d\n", WSAGetLastError());
            return -1;
        }
        rcvbuf[ret] = 0;
        printf("Read: '%s' from %s\n", rcvbuf, inet_ntoa(to.sin_addr));
    }
    closesocket(s);

    WSACleanup();
    return 0;
}
