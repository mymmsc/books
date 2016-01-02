// Module Name: wsnbdef.h
//
// Description:
//    This file contains common defines and function prototype
//    used by the NetBIOS Winsock samples.
//
#define MAX_BUFFER        16000
#define DEFAULT_COUNT        20
#define INVALID_LANA         0x80000000 

BOOL FindProtocol(WSAPROTOCOL_INFO **wsapi, DWORD *dwCount);
