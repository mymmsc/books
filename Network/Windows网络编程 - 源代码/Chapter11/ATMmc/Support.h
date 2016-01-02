// Module: support.h
//
// Description:
//    This file contains function prototypes for the support
//    routines found in support.c.
//
#include <winsock2.h>
#include <ws2atm.h>

int  GetNumATMInterfaces(SOCKET s);
BOOL GetATMAddress(SOCKET s, int device, ATM_ADDRESS *atmaddr);
BOOL FindProtocol(WSAPROTOCOL_INFO *lpProto);
void AtoH( CHAR *szDest, CHAR *szSource, INT iCount );
