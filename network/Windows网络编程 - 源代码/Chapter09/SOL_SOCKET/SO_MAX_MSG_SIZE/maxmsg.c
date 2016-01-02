// Module: maxmsg.c
//
// Description:
//    This is a very short sample that simply calls the
//    SO_MAX_MSG_SIZE option to determine the largest datagram
//    size possible.
//
// Compile:
//    cl maxmsg.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    None
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
// 
// Description:
//    Load Winsock, creat a UDP socket, and then call the option.
//    Print the results.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    int           iSize,
                  iVal,
                  ret;

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
    // Retrieve the option and print the result
    //
    iSize = sizeof(iVal);
    ret = getsockopt(s, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&iVal,
            &iSize);
    if (ret == SOCKET_ERROR)
    {
        printf("getsockopt(SO_MAX_MSG_SIZE): %d\n", WSAGetLastError());
        return -1;
    }

    printf("Max message size = %d\n", iVal);

    closesocket(s);

    WSACleanup();
    return 0;
}
