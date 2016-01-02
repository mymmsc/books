// Module: provider.c
//
// Description:
//    This module contains a single routine FindProtocolInfo which
//    enumerates all Winsock providers and searches for an entry
//    of the given address family, socket type, protocol, and flags
//    (such as QOS support).
//
// Compile:
//    cl /c provider.c
//
// Command Line Arguments/Parameters
//    None
//
#include <winsock2.h>
#include <windows.h>

#include <qos.h>
#include <qossp.h>

#include "provider.h"

#include <stdio.h>
#include <stdlib.h>

//
// Function: FindProtocolInfo
//
// Description:
//    This function searches the Winsock catalog for
//    a provider of the given address family, socket type,
//    protocol and flags. The flags field is a bitwise
//    OR of all the attributes that you request such as
//    multipoint or QOS support.
//
WSAPROTOCOL_INFO *FindProtocolInfo(int af, int type, 
    int protocol, DWORD flags)
{
    DWORD             protosz=0,
                      nprotos,
                      i;
    WSAPROTOCOL_INFO *buf=NULL;
    int               ret;
    static WSAPROTOCOL_INFO pinfo;

    // Find out the size of the buffer needed to enumerate
    // all entries.
    //
    ret = WSAEnumProtocols(NULL, NULL, &protosz);
    if (ret != SOCKET_ERROR)
    {
        printf("not supposed to be here!\n");
        return NULL;  
    }
    // Allocate the necessary buffer
    //
    buf = (WSAPROTOCOL_INFO *)LocalAlloc(LPTR, protosz);
    if (!buf)
    {
        printf("LocalAlloc() failed: %d\n", GetLastError());
        return NULL;
    }
    nprotos = protosz / sizeof(WSAPROTOCOL_INFO);
    //
    // Make the real call
    //
    ret = WSAEnumProtocols(NULL, buf, &protosz);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAEnumProtocols() failed: %d\n", WSAGetLastError());
        LocalFree(buf);
        return NULL;
    }
    // Search throught the catalog entries returned for the 
    // requested attributes.
    // 
    for(i=0; i < nprotos ;i++)
    {
        if ((buf[i].iAddressFamily == af) &&
            (buf[i].iSocketType == type) &&
            (buf[i].iProtocol == protocol))
        {
            if ((buf[i].dwServiceFlags1 & flags) == flags)
            {
                memcpy(&pinfo, &buf[i], sizeof(WSAPROTOCOL_INFO));
                LocalFree(buf);
                return &pinfo;
            }
        }
    }
    LocalFree(buf);
    return NULL;
}
