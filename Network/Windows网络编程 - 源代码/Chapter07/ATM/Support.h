// Module Name: support.h
//
// Description:
//    This file contains function prototypes for functions defined
//    in support.c
//
#include <winsock2.h>
#include <ws2atm.h>

int  GetNumATMInterfaces(SOCKET s);
BOOL GetATMAddress(SOCKET s, int device, ATM_ADDRESS *atmaddr);
WSAPROTOCOL_INFO *FindProtocol();
void AtoH( CHAR *szDest, CHAR *szSource, INT iCount );
