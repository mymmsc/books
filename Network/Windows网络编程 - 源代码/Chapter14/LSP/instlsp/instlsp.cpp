// Module Name: instlsp.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains an installation program to insert the layered sample
//    into the Winsock catalog of providers.
//    
//
// Compile:
//
//    This project is managed through the INSTLSP.DSW project file.
//
// Execute:
//
//    This project produces a executable file instlsp.exe. To execute simply
//    run this program as: instlsp.exe <install | remove>
//

#include <stdio.h>
#include <ws2spi.h>
#include <sporder.h>

#define PROVIDER_PATH L"lsp.dll"

GUID ProviderGuid = { //c5fabbd0-9736-11d1-937f-00c04fad860d
	0xc5fabbd0,
	0x9736,
	0x11d1,
	{0x93, 0x7f, 0x00, 0xc0, 0x4f, 0xad, 0x86, 0x0d}
};

GUID ProviderChainGuid = {//f9065320-9e90-11d1-9381-00c04fad860d
	0xf9065320,
	0x9e90,
	0x11d1,
	{0x93, 0x81, 0x00, 0xc0, 0x4f, 0xad, 0x86, 0x0d}
};

BOOL GetProviders();
void InstallProvider(void);
void RemoveProvider(void);

LPWSAPROTOCOL_INFOW ProtocolInfo = NULL;
DWORD ProtocolInfoSize = 0;
INT TotalProtocols = 0;

void main(int argc, char *argv[])
{
	if (argc > 1)
	{
		if (!strcmp(argv[1], "install"))
		{
			InstallProvider();
			return;
		}
		if (!strcmp(argv[1], "remove"))
		{
			RemoveProvider();
			return;
		}
	}
	printf("Usage: instlsp [install || remove]\n");
}

BOOL GetProviders()
{
	INT ErrorCode;

	ProtocolInfo = NULL;
	ProtocolInfoSize = 0;
	TotalProtocols = 0;

	// Find out how many entries we need to enumerate
	if (WSCEnumProtocols(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode) == SOCKET_ERROR)
	{
		if (ErrorCode != WSAENOBUFS)
		{
			printf("First WSCEnumProtocols failed %d\n", ErrorCode);
			return(FALSE);
		}
	}

	if ((ProtocolInfo = (LPWSAPROTOCOL_INFOW) GlobalAlloc(GPTR, ProtocolInfoSize)) == NULL)
	{
		printf("Cannot enumerate Protocols %d\n", GetLastError());
		return(FALSE);
	}

	if ((TotalProtocols = WSCEnumProtocols(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode)) == SOCKET_ERROR)
	{
		printf("Second WSCEnumProtocols failed %d\n", ErrorCode);
		return(FALSE);
	}


	printf("Found %d protocols\n",TotalProtocols); 

	return(TRUE);
}

void FreeProviders(void)
{
	GlobalFree(ProtocolInfo);
}


void InstallProvider(void)
{
	INT ErrorCode;
	LPDWORD CatalogEntries;
	INT i;
	INT CatIndex;
	DWORD LayeredCatalogId, RawOrigCatalogId, TcpOrigCatalogId, UdpOrigCatalogId;
	WSAPROTOCOL_INFOW  TCPChainInfo, UDPChainInfo, RAWChainInfo, IPLayeredInfo, ChainArray[3];
	WCHAR ChainName[WSAPROTOCOL_LEN+1];
	BOOL RawIP = FALSE;
	BOOL UdpIP = FALSE;
	BOOL TcpIP = FALSE;
	INT ProvCnt = 0;

	GetProviders();

	// Find ALL AF_INET providers and build a WSAPROTOCOL_INFOW structure to mimic their functionality 
	for (i = 0; i < TotalProtocols; i++)
	{
		// Find first RAW/IP Provider
		if (!RawIP &&
			ProtocolInfo[i].iAddressFamily == AF_INET &&
			ProtocolInfo[i].iProtocol == IPPROTO_IP)
		{
			RawIP = TRUE;

			// Copy RAW/IP properties for our layered provider and protocol chain
			RawOrigCatalogId = ProtocolInfo[i].dwCatalogEntryId;
			memcpy(&IPLayeredInfo, &ProtocolInfo[i], sizeof(WSAPROTOCOL_INFOW));
			memcpy(&RAWChainInfo, &ProtocolInfo[i], sizeof(WSAPROTOCOL_INFOW));

			// Make our provider a NON IFS provider

			IPLayeredInfo.dwServiceFlags1 = RAWChainInfo.dwServiceFlags1 = ProtocolInfo[i].dwServiceFlags1 & (~XP1_IFS_HANDLES); 
		}

		// Find first TCP/IP Provider
		if (!TcpIP &&
			ProtocolInfo[i].iAddressFamily == AF_INET &&
			ProtocolInfo[i].iProtocol == IPPROTO_TCP)
		{
			TcpIP = TRUE;

			// Copy TCP/IP properties for our layered provider and protocol chain
			TcpOrigCatalogId = ProtocolInfo[i].dwCatalogEntryId;
			memcpy(&TCPChainInfo, &ProtocolInfo[i], sizeof(WSAPROTOCOL_INFOW));

			// Make our provider a NON IFS provider

			TCPChainInfo.dwServiceFlags1 = ProtocolInfo[i].dwServiceFlags1 & (~XP1_IFS_HANDLES); 
		}

		// Find first UDP/IP Provider
		if (!UdpIP &&
			ProtocolInfo[i].iAddressFamily == AF_INET &&
			ProtocolInfo[i].iProtocol == IPPROTO_UDP)
		{
			UdpIP = TRUE;

			// Copy UDP/IP properties for our layered provider and protocol chain
			UdpOrigCatalogId = ProtocolInfo[i].dwCatalogEntryId;
			memcpy(&UDPChainInfo, &ProtocolInfo[i], sizeof(WSAPROTOCOL_INFOW));

			// Make our provider a NON IFS provider

			UDPChainInfo.dwServiceFlags1 = ProtocolInfo[i].dwServiceFlags1 & (~XP1_IFS_HANDLES); 
		}

	}

	// Set basic layered IP provider fields
	wcscpy(IPLayeredInfo.szProtocol, L"Layered IP");
	IPLayeredInfo.ProtocolChain.ChainLen = LAYERED_PROTOCOL;

	if (WSCInstallProvider(&ProviderGuid, PROVIDER_PATH, &IPLayeredInfo, 1, &ErrorCode) == SOCKET_ERROR)
	{
		printf("WSCInstallProvider failed %d\n", ErrorCode);
		return;
	}


	FreeProviders();

	GetProviders();

	// Find out what our layered provider catalog entry is
	for (i = 0; i < TotalProtocols; i++)
		if (memcmp (&ProtocolInfo[i].ProviderId, &ProviderGuid, sizeof (GUID))==0)
		{
			LayeredCatalogId = ProtocolInfo[i].dwCatalogEntryId;
			break;
		}


	if (TcpIP)
	{
		// Set basic protocol chain info here
		swprintf(ChainName, L"Layered TCP/IP over [%s]", TCPChainInfo.szProtocol);
		wcscpy(TCPChainInfo.szProtocol, ChainName);

		// Setup the protocol chain to include our layered provider
		if (TCPChainInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		{
			// Setup a new protocol chain
			TCPChainInfo.ProtocolChain.ChainEntries[1] = TcpOrigCatalogId;
		} else
		{
			// Push protocol entries down the protocol chain
			for (i = TCPChainInfo.ProtocolChain.ChainLen; i > 0; i--)
			{
				TCPChainInfo.ProtocolChain.ChainEntries[i + 1] = TCPChainInfo.ProtocolChain.ChainEntries[i];
			}
		}
		TCPChainInfo.ProtocolChain.ChainLen++;
		TCPChainInfo.ProtocolChain.ChainEntries[0] = LayeredCatalogId;

		memcpy(&ChainArray[ProvCnt++], &TCPChainInfo, sizeof(WSAPROTOCOL_INFOW));

	}

	if (UdpIP)
	{
		// Set basic protocol chain info here
		swprintf(ChainName, L"Layered UDP/IP over [%s]", UDPChainInfo.szProtocol);
		wcscpy(UDPChainInfo.szProtocol, ChainName);

		// Setup the protocol chain to include our layered provider
		if (UDPChainInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		{
			// Setup a new protocol chain
			UDPChainInfo.ProtocolChain.ChainEntries[1] = UdpOrigCatalogId;
		} else
		{
			// Push protocol entries down the protocol chain
			for (i = UDPChainInfo.ProtocolChain.ChainLen; i > 0; i--)
			{
				UDPChainInfo.ProtocolChain.ChainEntries[i + 1] = UDPChainInfo.ProtocolChain.ChainEntries[i];
			}
		}
		UDPChainInfo.ProtocolChain.ChainLen++;
		UDPChainInfo.ProtocolChain.ChainEntries[0] = LayeredCatalogId;

		memcpy(&ChainArray[ProvCnt++], &UDPChainInfo, sizeof(WSAPROTOCOL_INFOW));

	}

	if (RawIP)
	{
		// Set basic protocol chain info here
		swprintf(ChainName, L"Layered RAW/IP over [%s]", RAWChainInfo.szProtocol);
		wcscpy(RAWChainInfo.szProtocol, ChainName);

		// Setup the protocol chain to include our layered provider
		if (RAWChainInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		{
			// Setup a new protocol chain
			RAWChainInfo.ProtocolChain.ChainEntries[1] = RawOrigCatalogId;
		} else
		{
			// Push protocol entries down the protocol chain
			for (i = RAWChainInfo.ProtocolChain.ChainLen; i > 0; i--)
			{
				RAWChainInfo.ProtocolChain.ChainEntries[i + 1] = RAWChainInfo.ProtocolChain.ChainEntries[i];
			}
		}
		RAWChainInfo.ProtocolChain.ChainLen++;
		RAWChainInfo.ProtocolChain.ChainEntries[0] = LayeredCatalogId;
		
		memcpy(&ChainArray[ProvCnt++], &RAWChainInfo, sizeof(WSAPROTOCOL_INFOW));
	}


	if (WSCInstallProvider(&ProviderChainGuid, PROVIDER_PATH, ChainArray, ProvCnt, &ErrorCode) == SOCKET_ERROR)
	{
		printf("WSCInstallProvider for protocol chain failed %d\n", ErrorCode);
		return;
	}


	FreeProviders();

	GetProviders();

	// Build provider list order
	if ((CatalogEntries = (LPDWORD) GlobalAlloc(GPTR, TotalProtocols * sizeof(DWORD))) == NULL)
	{
		printf("GlobalAlloc failed %d\n", GetLastError());
		return;
	}

	// Find our provider entries and put them first in the list
	CatIndex = 0;
	for (i = 0; i < TotalProtocols; i++)
		if (memcmp (&ProtocolInfo[i].ProviderId, &ProviderGuid, sizeof (GUID))==0 ||
			memcmp (&ProtocolInfo[i].ProviderId, &ProviderChainGuid, sizeof (GUID))==0)
			CatalogEntries[CatIndex++] = ProtocolInfo[i].dwCatalogEntryId;

	// Put all other entries at the end
	for (i = 0; i < TotalProtocols; i++)
		if (memcmp (&ProtocolInfo[i].ProviderId, &ProviderGuid, sizeof (GUID))!=0 &&
			memcmp (&ProtocolInfo[i].ProviderId, &ProviderChainGuid, sizeof (GUID))!=0)
			CatalogEntries[CatIndex++] = ProtocolInfo[i].dwCatalogEntryId;

	if ((ErrorCode = WSCWriteProviderOrder(CatalogEntries, TotalProtocols)) != ERROR_SUCCESS)
	{
		printf("WSCWriteProviderOrder failed %d\n", ErrorCode);
		return;
	}

	FreeProviders();
}

void RemoveProvider(void)
{
	INT ErrorCode;

	if (WSCDeinstallProvider(&ProviderGuid, &ErrorCode) == SOCKET_ERROR)
	{
		printf("WSCDeistallProvider for Layer failed %d\n", ErrorCode);
	}
	if (WSCDeinstallProvider(&ProviderChainGuid, &ErrorCode) == SOCKET_ERROR)
	{
		printf("WSCDeistallProvider for Chain failed %d\n", ErrorCode);
	}

	// We recommend cleaning up any available protocol chains that reference this
	// provider to avoid having broken chains. We did not implement this functionality
	// in the sample.
}
