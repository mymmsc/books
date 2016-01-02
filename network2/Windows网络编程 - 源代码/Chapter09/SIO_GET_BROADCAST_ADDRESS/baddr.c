// Module: baddr.c
//
// Description:
//    This sample shows how to obtain the broadcast address
//    for a UDP socket.
//
// Compile:
//    cl baddr.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    baddr.exe
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
//
// Description:
//    Load Winsock, create a socket and find the broadcast
//    address.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    SOCKADDR_IN   bcast;
    int           ret;
    DWORD         dwBytesRet;

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
    // Call the ioctl 
    //
    ret = WSAIoctl(s, SIO_GET_BROADCAST_ADDRESS, NULL, 0,
            &bcast, sizeof(bcast), &dwBytesRet, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoclt(SIO_GET_BROADCAST_ADDRESS) failed: %d\n",
            WSAGetLastError());
        return -1;
    }

    printf("Address is: '%s'\n", inet_ntoa(bcast.sin_addr));

    closesocket(s);

    WSACleanup();
    return 0;
}
