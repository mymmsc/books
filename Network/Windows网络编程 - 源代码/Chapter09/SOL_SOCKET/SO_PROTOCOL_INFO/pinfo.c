// Module: pinfo.c
//
// Description:
//    This short sample illustrates how to call the SO_PROTOCOL_INFO
//    option to retrieve the Winsock catalog entry corresponding
//    to the socket. This function only shows how to call the option
//    and does not print out the whole WSAPROTOCOL_INFO structure.
//    For that see the enum.c sample under Chapter 6.
//
// Compile:
//    cl pinfo.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    pinfo.exe
//
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
//
// Description:
//    Load Winsock, create a socket, and call SO_PROTOCOL_INFO to
//    retrieve the catalog entry corresponding to our socket.
//
int main(int argc, char **argv)
{
    WSADATA          wsd;
    SOCKET           s;
    WSAPROTOCOL_INFOA pinf;
    int              iSize,
                     ret;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Create our TCP socket
    //
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Get the protocol information. You can either use WSA_PROTOCOL_INFO
    // or SO_PROTOCOL_INFOA since we didn't #define UNICODE.
    //
    iSize = sizeof(pinf);
    ZeroMemory(&pinf, iSize);
    ret = getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFOA, (char *)&pinf,
            &iSize);
    if (ret == SOCKET_ERROR)
    {
        printf("getsockopt(SO_PROTOCOL_INFO) failed: %d\n", 
            WSAGetLastError());
        return -1;
    }
    // Print a couple fields out
    //
    printf("Protocol: [%s]\n", pinf.szProtocol);
    printf("Flags:    0x%x\n", pinf.dwServiceFlags1);

    closesocket(s);

    WSACleanup();
    return 0;
}
