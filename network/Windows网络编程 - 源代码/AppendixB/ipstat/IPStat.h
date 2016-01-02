#ifndef _IPSTAT_H
#define _IPSTAT_H

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



#define MAX_STRLEN        128    /* general max string length */

void DoGetConnTable(char* pszProto);
void DoGetStat(char* pszProto = NULL);

void DumpTcpTable(PMIB_TCPTABLE pTcpTable);
void DumpUdpTable(PMIB_UDPTABLE pUdpTable);

DWORD MyGetTcpTable(PMIB_TCPTABLE& pTcpTable, BOOL fOrder = FALSE);
DWORD MyGetUdpTable(PMIB_UDPTABLE& pUdpTable, BOOL fOrder = FALSE);

DWORD MyGetIpStatistics(PMIB_IPSTATS& pIpStats);
DWORD MyGetIcmpStatistics(PMIB_ICMP& pIcmpStats);
DWORD MyGetTcpStatistics(PMIB_TCPSTATS& pTcpStats);
DWORD MyGetUdpStatistics(PMIB_UDPSTATS& pUdpStats);

void PrintIpStats(PMIB_IPSTATS pStats);
void PrintIcmpStats(MIBICMPINFO *pStats);
void PrintTcpStats(PMIB_TCPSTATS pStats);
void PrintUdpStats(PMIB_UDPSTATS pStats);

#endif // _IPSTAT_H
