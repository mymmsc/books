// Module: socktype.c
//
// Description:
//    This sample creates a socket and then calls the SO_TYPE option
//    to retrieve the socket type (SOCK_STREAM, SOCK_DGRAM, etc) for
//    that socket.
//
// Compile:
//    cl socktype.c ws2_32.lib
// 
// Command Line Arguments/Parameters
//    socktype.exe
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
//
// Description:
//    Load Winsock, create a socket, and then call the SO_TYPE
//    option and print the results.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    int           ret,
                  iVal,
                  iSize;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // In this case create a UDP SOCK_DGRAM socket
    //
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Retrieve the type and print its value
    //
    iSize = sizeof(iVal);
    ret = getsockopt(s, SOL_SOCKET, SO_TYPE, (char *)&iVal,
            &iSize);
    if (ret == SOCKET_ERROR)
    {
        printf("getsockopt(SO_TYPE) failed: %d\n", iVal);
        return -1;
    }
    if (iVal == SOCK_STREAM)
        printf("SOCK_STREAM\n"); 
    else if (iVal == SOCK_DGRAM)
        printf("SOCK_DGRAM\n"); 
    else if (iVal == SOCK_RDM)
        printf("SOCK_RDM\n"); 
    else if (iVal == SOCK_SEQPACKET)
        printf("SOCK_SEQPKACKET\n"); 
    else if (iVal == SOCK_RAW)
        printf("SOCK_RAW\n"); 
    else
        printf("unknown\n");


    closesocket(s);

    WSACleanup();
    return 0;
}
