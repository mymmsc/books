// Module: accept.c
//
// Description:
//    This module illustrates the SO_ACCEPTCONN option to find 
//    out whether a socket is listening for connections.
//
// Compile:
//    cl accept.c ws2_32.lib
//
// Command Line Arguments/Paramters
//    accept.exe
//
#include <winsock2.h>

#include <stdio.h>
#include <stdlib.h>

// 
// Function: main
// 
// Description:
//    Loads Winsock, creates a listening socket and get the state
//    of SO_ACCEPTCONN.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    SOCKADDR_IN   local;
    int           ret,
                  iSize;
    BOOL          bOpt;

    // Load Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Creat a socket, bind to it, and listen
    //
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(5150);

    iSize = sizeof(bOpt);
    ret = getsockopt(s, SOL_SOCKET, SO_ACCEPTCONN, (char *)&bOpt, &iSize);
    if (ret == SOCKET_ERROR)
    {
        printf("getsockopt(SO_ACCEPTCONN) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    if (bOpt == TRUE)
        printf("returned TRUE\n");
    else
        printf("returned FALSE\n");


    if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return -1;
    }

    listen(s, 7);
    //
    // Get the option value
    //
    iSize = sizeof(bOpt);
    ret = getsockopt(s, SOL_SOCKET, SO_ACCEPTCONN, (char *)&bOpt, &iSize);
    if (ret == SOCKET_ERROR)
    {
        printf("getsockopt(SO_ACCEPTCONN) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    if (bOpt == TRUE)
        printf("returned TRUE\n");
    else
        printf("returned FALSE\n");

    closesocket(s);

    WSACleanup();
    return 0;
}
