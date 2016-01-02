// Module Name: provider.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains support functions that are common to the lsp and
//    the instlsp sample for enumerating the Winsock catalog of service
//    providers.
//    
//
// Compile:
//
//    This project is managed through the LSP.DSW project file.
//
// Execute:
//
//    This project produces a DLL named lsp.dll. This dll should be copied to the
//    %SystemRoot%\System32 directory. Once the file is in place you should execute
//    the application instlsp.exe to insert this provider in the Winsock 2 catalog
//    of service providers.
//

#include <ws2spi.h>
#include <sporder.h>
#include "provider.h"

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

LPWSAPROTOCOL_INFOW GetProviders(LPINT TotalProtocols)
{
	INT ErrorCode;

	LPWSAPROTOCOL_INFOW ProtocolInfo = NULL;
	DWORD ProtocolInfoSize = 0;
	*TotalProtocols = 0;

	// Find out how many entries we need to enumerate
	if (WSCEnumProtocols(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode) == SOCKET_ERROR)
	{
		if (ErrorCode != WSAENOBUFS)
		{
			return(NULL);
		}
	}

	if ((ProtocolInfo = (LPWSAPROTOCOL_INFOW) GlobalAlloc(GPTR, ProtocolInfoSize)) == NULL)
	{
		return(NULL);
	}

	if ((*TotalProtocols = WSCEnumProtocols(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode)) == SOCKET_ERROR)
	{

		return(NULL);
	}

	return(ProtocolInfo);
}

void FreeProviders(LPWSAPROTOCOL_INFOW ProtocolInfo)
{
	GlobalFree(ProtocolInfo);
}
