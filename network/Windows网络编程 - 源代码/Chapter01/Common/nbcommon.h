// Module Name: nbcommon.h
//
// Purpose:
//    This header file contains the function prototypes for a set
//    of functions that implement some of the most common NetBIOS
//    functions such as enumerating LANAs, adding names, removing
//    names, etc.  The functions are implemented in Nbcommon.c
//    
#include <windows.h>
#include <nb30.h>

int Recv(int lana, int lsn, char *buffer, DWORD *len);
int Send(int lana, int lsn, char *data, DWORD len);
int AddName(int lana, char *name, int *num);
int DelName(int lana, char *name);
int AddGroupName(int lana, char *name, int *num);
int ResetAll(LANA_ENUM *lenum, UCHAR ucMaxSession, 
	     UCHAR ucMaxName, BOOL bFirstName);
int LanaEnum(LANA_ENUM *lenum);
int Hangup(int lana, int lsn);
int Cancel(PNCB pncb);
int FormatNetbiosName(char *nbname, char *outname);
