// Module Name: Ircommon.c
//
// Description:
//    This file contains two simple functions, one for sending
//    data on a connected socket and the other for receiving.
//    These two functions are common to both client and server
//    so they were pulled out into a seperate file.
//
// Compile:
//    cl /c Ircommon.c
//
#ifdef _WIN32_WCE
#include <windows.h>
#include <winsock.h>
#else
#include <winsock2.h>
#endif

#include "ircommon.h"

int senddata(SOCKET s, char *buf, int *len)
{
    int        ret,
               index=0,
               slen;
    DWORD      dwErr;
 
    slen = *len;
    while (len > 0)
    {
        ret = send(s, &buf[index], slen, 0);
        if (ret == SOCKET_ERROR)
        {
            if ((dwErr = WSAGetLastError()) != WSAEWOULDBLOCK)
            {
                return dwErr;
            }
        }
        else if (ret == 0)
        {
            *len = 0;
            return 0;
        }
        slen -= ret;
        index += ret;
    }
    *len = index;
    return 0;
}

int recvdata(SOCKET s, char *buf, int *len)
{
    int        ret,
               index=0;
    DWORD      dwErr;

    ret = recv(s, buf, *len, 0);
    if (ret == SOCKET_ERROR)
    {
        if ((dwErr = WSAGetLastError()) != WSAEWOULDBLOCK)
        {
            return dwErr;
        }
        else if (ret == 0)
        {
            *len = 0;
            return 0;
        }
    }
    *len = ret;
    return 0;
}

