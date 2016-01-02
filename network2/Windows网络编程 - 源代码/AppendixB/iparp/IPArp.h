#ifndef _IPARP_H
#define _IPARP_H

#include <windows.h>
#include <winsock.h>
#include <iphlpapi.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <iptypes.h>


void  PrintIpNetTable(PMIB_IPNETTABLE pIpNetTable);
DWORD MyGetIpNetTable(PMIB_IPNETTABLE & pIpArpTab, bool fOrder = FALSE);
DWORD MyGetIpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, bool fOrder = FALSE);
void  PrintIpAddrTable(PMIB_IPADDRTABLE pIpAddrTable);

void DoGetIpNetTable();
void DoSetIpNetEntry(char* pszDottedInetAddr, char* pszPhysAddr, char* pszInterface = NULL);
void DoDeleteIpNetEntry(char* pszInetAddr, char* pszInterface = NULL);

bool PhysAddrToString(BYTE PhysAddr[], DWORD PhysAddrLen, char str[]);
int  StringToPhysAddr(char* szInEther, char* szOutEther);
bool InterfaceIdxToInterfaceIp(PMIB_IPADDRTABLE pIpAddrTable, DWORD dwIndex, char str[]);


#endif //_IPARP_H
