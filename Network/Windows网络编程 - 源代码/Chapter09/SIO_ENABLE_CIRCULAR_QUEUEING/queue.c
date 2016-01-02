// Module: queue.c
//
// Description:
//    This sample shows how to set the ioctl SIO_ENABLE_CIRCULAR_QUEUEING.
//    This ioctl is very difficult to prove that it works. You need to
//    start blasting datagram packets as quickly as possible and capture
//    the results with Microsoft Network Monitor to find out which packets
//    got dropped.
//
// Compile:
//    cl queue.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    queue.exe
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
//
// Description:
//    Load Winsock, create a socket, and set the ioctl. Afterwards
//    call the ioctl again to retrieve its value.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    int           ret;
    DWORD         dwBytesRet;
    BOOL          bOpt;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Creat a socket
    //
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Set the ioctl
    //
    bOpt = TRUE;
    ret = WSAIoctl(s, SIO_ENABLE_CIRCULAR_QUEUEING, &bOpt, sizeof(bOpt),
            NULL, 0, &dwBytesRet, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_ENABLE_CIRCULAR_QUEUEING) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    // Get the ioctl
    //
    ret = WSAIoctl(s, SIO_ENABLE_CIRCULAR_QUEUEING, NULL, 0,
            &bOpt, sizeof(bOpt), &dwBytesRet, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_ENABLE_CIRCULAR_QUEUEING) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    if (bOpt == TRUE)
        printf("Circular queueing is TRUE\n");
    else
        printf("Circular queueing is FALSE\n");

    closesocket(s);

    WSACleanup();
    return 0;
}
