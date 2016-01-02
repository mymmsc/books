// Module: tos.c
//
// Description:
//    This sample sets the Type-of-Service bit and sends
//    some datagrams. To really see the results of this
//    option you need to capture the packets with Microsoft
//    Network Monitor (netmon) to view the actual packets.
//
// Compile:
//    cl tos.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    tos.exe ip
//      ip		IP Address
//		
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

// 
// Function: main
//
// Description:
//    Load Winsock, create a datagram socket, set the IP_TOS
//    option and send some packets.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    SOCKADDR_IN   remote;
    char         *str="This is a test";
    int           ret,
                  iTos;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Create a socket
    //
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Set the TOS bit
    //
    iTos = 1;
    ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&iTos, sizeof(iTos));
    if (ret == SOCKET_ERROR)
    {
        printf("setsockopt(IP_TOS) failed; %d\n", WSAGetLastError());
        return -1;
    }
    // Perform a sendto
    //
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(argv[1]);
    remote.sin_port = htons(5150);

    ret = sendto(s, str, strlen(str), 0, (SOCKADDR *)&remote, sizeof(remote));
    if (ret == SOCKET_ERROR)
    {
        printf("sendto() failed; %d\n", WSAGetLastError());
        return -1;
    }
    printf("Wrote %d bytes\n", ret);
    //
    // Cleanup
    //
    closesocket(s);

    WSACleanup();
    return 0;
}
