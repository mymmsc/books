// Module: rcvtimeo.c
//
// Description:
//    This is a trivial sample that sets the receive timeout
//    option and then attempts to receive a datagram which will
//    fail with a timeout message.
//
// Compile:
//    cl rcvtimeo.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    rcvtimeo.exe
//
#include <windows.h>
#include <winsock.h>

#include <stdio.h>
#include <stdlib.h>

//
// Function: main
//
// Description:
//    Load Winsock, create a UDP socket, set the timeout value,
//    and then call recvfrom() will will fail with a timeout since
//    no data is being sent.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKADDR_IN   from;
    SOCKET        s;
    int           ret,
                  iVal=0,
                  fromsz;
    unsigned int  sz = sizeof(iVal);
    char          rcvbuf[1024];

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(1,1), &wsd) != 0)
        return 0;
    //
    // Create a datagram socket
    //
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
        return 0;
    //
    // Set the timeout value
    //
    iVal = 100;
    ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&iVal, sz);
    if (ret == SOCKET_ERROR)
    {
        printf("SO_ERROR failed: %d\n", WSAGetLastError());
        return 0;
    }
    printf("Timeout set\n");
    //
    // Retrieve the value just to be sure
    //
    ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&iVal, &sz);
    if (ret == SOCKET_ERROR)
    {
        printf("SO_ERROR failed: %d\n", WSAGetLastError());
        return 0;
    }
    printf("Timeout val == %d\n", iVal);
    //
    // To test the receive function we need to bind first
    //
    from.sin_family = AF_INET;
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    from.sin_port = htons(5150);
   
    ret = bind(s, (SOCKADDR *)&from, sizeof(from));
    if (ret == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return -1;
    } 
    // Call receive which will fail with WSAETIMEDOUT
    //
    fromsz = sizeof(from);
    ret = recvfrom(s, rcvbuf, 1024, 0, (SOCKADDR *)&from, &fromsz);
    if (ret == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAETIMEDOUT)
            printf("recvfrom failed with: 10060 (WSAETIMEDOUT)\n");
        else
            printf("recvfrom() failed: %d\n", WSAGetLastError());
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
