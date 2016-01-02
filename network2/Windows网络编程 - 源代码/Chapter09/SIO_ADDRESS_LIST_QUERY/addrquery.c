// Module: addrquery.c
//
// Description:
//    This sample shows how to use the SIO_ADDRESS_LIST_QUERY
//    ioctl. This is a Windows 2000 command which lists the 
//    local interfaces available.
// 
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

// 
// Function: main
//
// Description:
//    Load Winsock, create a socket, and perform the interface
//    query.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s;
    DWORD         dwBytesRet=0;
    char          buf[1024];
    int           ret,
                  i;
    SOCKET_ADDRESS_LIST *slist;
    SOCKADDR_IN  *addr;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
            WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Get the address list
    //
    ret = WSAIoctl(s, SIO_ADDRESS_LIST_QUERY, NULL, 0, 
            buf, 1024, &dwBytesRet, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_ADDRESS_LIST_QUERY) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    printf("Bytes Returned: %d bytes\n", dwBytesRet);
    //
    // Print the results
    //
    slist = (SOCKET_ADDRESS_LIST *)buf;
   
    printf("      Addr Count: %d\n", slist->iAddressCount);
    for(i=0; i < slist->iAddressCount ;i++)
    {
        addr = (SOCKADDR_IN *) slist->Address[i].lpSockaddr;
        printf("       Addr [%02d]: %s\n", i, inet_ntoa(addr->sin_addr));
        printf("Addr Length [%02d]: %d bytes\n", i, 
            slist->Address[i].iSockaddrLength);
    }
    closesocket(s);
    WSACleanup();

    return 0;
}
