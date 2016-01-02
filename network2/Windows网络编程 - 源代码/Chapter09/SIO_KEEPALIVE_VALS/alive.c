// Module: alive.c
//
// Description:
//    This sample shows you how to set the SO_KEEPALIVE_VALS
//    on a socket. This option allows you to set the keepalive
//    interval on a per socket basis as opposed to the rather
//    useless SO_KEEPALIVE option.
//
// Compile:
//    cl alive.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    alive.exe
//
#include <winsock2.h>
#include <ws2tcpip.h>
#include "mstcpip.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    DWORD         dwBytesRet=0;
    struct tcp_keepalive   alive;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Create a socket
    //
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Set the keepalive values
    //
    alive.onoff = TRUE;
    alive.keepalivetime = 7200000;
    alive.keepaliveinterval = 6000;

    if (WSAIoctl(s, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
            NULL, 0, &dwBytesRet, NULL, NULL) == SOCKET_ERROR)
    {
        printf("WSAIotcl(SIO_KEEPALIVE_VALS) failed; %d\n",
            WSAGetLastError());
        return -1;
    }
    printf("SIO_KEEPALIVE_VALS set:\n");
    printf("   Keepalive Time     = %lu\n", alive.keepalivetime);
    printf("   Keepalive Interval = %lu\n", alive.keepaliveinterval);

    closesocket(s);
    WSACleanup();
    return 0;
}
