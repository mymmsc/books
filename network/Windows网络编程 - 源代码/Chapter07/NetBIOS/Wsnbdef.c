// Module Name: wsnbdef.c
//
// Description:
//    This file contains common definitions and functions used
//    by all the NetBIOS Winsock samples. 
//
// Compile:
//    See wsnbclnt.c, wsnbsvr.c, and wsnbdgs.c for compilation
//     instructions.
//
#include <winsock2.h>

#include <stdio.h>
#include <stdlib.h>

#include "wsnbdef.h"

extern int iSocketType;

//
// Function: FindProtocol
//
// Description:
//    Search through the available network service providers for
//    AF_NETBIOS compatible protocols. The number of providers 
//    returned will be equal to 2 times the number of LANAs we
//    would have in NetBIOS. This is because there is two providers
//    for each LANA: one datagram and one session oriented provider.
//
BOOL FindProtocol(WSAPROTOCOL_INFO **wsapi, DWORD *dwCount)
{
    WSAPROTOCOL_INFO *lpProtocolBuf=NULL;
    DWORD             dwErr,
                      dwRet,
                      dwBufLen=0;
    int               i;

    *dwCount = 0;
    if (SOCKET_ERROR != WSAEnumProtocols(NULL, lpProtocolBuf, 
        &dwBufLen))
    {
        // This should never happen as there is a NULL buffer
        //
	printf("WSAEnumProtocols failed!\n");
	return FALSE;
    }
    else if (WSAENOBUFS != (dwErr = WSAGetLastError()))
    {
	// We failed for some reason not relating to buffer size - 
        // also odd
        //
	printf("WSAEnumProtocols failed: %d\n", dwErr);
	return FALSE;
    }
    // Allocate the correct buffer size for WSAEnumProtocols as
    // well as the buffer to return
    //
    lpProtocolBuf = (WSAPROTOCOL_INFO *)GlobalAlloc(GMEM_FIXED, 
        dwBufLen);
    *wsapi = (WSAPROTOCOL_INFO *)GlobalAlloc(GMEM_FIXED, dwBufLen);

    if ((lpProtocolBuf == NULL) || (*wsapi == NULL))
    {
	printf("GlobalAlloc failed: %d\n", GetLastError());
	return FALSE;
    }
    dwRet = WSAEnumProtocols(NULL, lpProtocolBuf, &dwBufLen);
    if (dwRet == SOCKET_ERROR)
    {
	printf("WSAEnumProtocols failed: %d\n", WSAGetLastError());
	GlobalFree(lpProtocolBuf);
	return FALSE;
    }
    // Loop through the returned protocol information looking for those
    // that are in the AF_NETBIOS address family.
    //
    for (i=0; i < dwRet ;i++)
    {
	if (lpProtocolBuf[i].iAddressFamily == AF_NETBIOS)
	{
            if (iSocketType == SOCK_SEQPACKET)
            {
                if ( (lpProtocolBuf[i].dwServiceFlags1 & XP1_GUARANTEED_DELIVERY) &&
                     (lpProtocolBuf[i].dwServiceFlags1 & XP1_GUARANTEED_ORDER) &&
                    ((lpProtocolBuf[i].dwServiceFlags1 & XP1_CONNECTIONLESS) == 0) &&
                     (lpProtocolBuf[i].iSocketType == iSocketType))
                {
                    (*wsapi)[(*dwCount)++] = lpProtocolBuf[i];
                }
            }
            else if (iSocketType == SOCK_DGRAM)
            {
                if ( !(lpProtocolBuf[i].dwServiceFlags1 & XP1_GUARANTEED_DELIVERY) &&
                     !(lpProtocolBuf[i].dwServiceFlags1 & XP1_GUARANTEED_ORDER) &&
                    ((lpProtocolBuf[i].dwServiceFlags1 & XP1_CONNECTIONLESS) != 0) &&
                     (lpProtocolBuf[i].iSocketType == iSocketType))
                {
                    (*wsapi)[(*dwCount)++] = lpProtocolBuf[i];
                }
            }
	}
    }
    GlobalFree(lpProtocolBuf);
    return TRUE;
}
