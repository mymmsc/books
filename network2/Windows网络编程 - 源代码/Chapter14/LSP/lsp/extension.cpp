// Module Name: extension.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains all of the Winsock extension functions that can
//    be monitored by a service provider.
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

#include "provider.h"

BOOL PASCAL FAR ExtTransmitFile (
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved)
{
	SOCK_INFO *SocketContext;
	LPWSAOVERLAPPED ProviderOverlapped;
	LPFN_TRANSMITFILE lpProviderTransmitFile;
	GUID TransmitFileGuid = WSAID_TRANSMITFILE;
	DWORD BytesReturned;
	int Errno;

	if (MainUpCallTable.lpWPUQuerySocketHandleContext(hSocket,
		(LPDWORD) &SocketContext,
		&Errno) == SOCKET_ERROR)
		return FALSE;

	if (NextProcTable.lpWSPIoctl(SocketContext->ProviderSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&TransmitFileGuid, sizeof(GUID), (LPVOID) &lpProviderTransmitFile, sizeof(LPFN_TRANSMITFILE), 
		&BytesReturned, NULL, NULL, NULL, &Errno) == SOCKET_ERROR)
	{
		return FALSE;
	}

	// Check for overlapped I/O
	
	if (lpOverlapped)
	{
		ProviderOverlapped = GetOverlappedStructure(hSocket, SocketContext->ProviderSocket, lpOverlapped, NULL,
			NULL, NULL);

		return lpProviderTransmitFile(
			SocketContext->ProviderSocket,
			hFile,
			nNumberOfBytesToWrite,
			nNumberOfBytesPerSend,
			ProviderOverlapped,
			lpTransmitBuffers,
			dwReserved);
	}
	else
	{
		return lpProviderTransmitFile(
			SocketContext->ProviderSocket,
			hFile,
			nNumberOfBytesToWrite,
			nNumberOfBytesPerSend,
			NULL,
			lpTransmitBuffers,
			dwReserved);
	}

	return FALSE;
}


BOOL PASCAL FAR ExtAcceptEx(
	IN SOCKET sListenSocket,
	IN SOCKET sAcceptSocket,
	IN PVOID lpOutputBuffer,
	IN DWORD dwReceiveDataLength,
	IN DWORD dwLocalAddressLength,
	IN DWORD dwRemoteAddressLength,
	OUT LPDWORD lpdwBytesReceived,
	IN LPOVERLAPPED lpOverlapped)
{
	SOCK_INFO *ListenSocketContext;
	SOCK_INFO *AcceptSocketContext;
	LPWSAOVERLAPPED ProviderOverlapped;
	LPFN_ACCEPTEX lpProviderAcceptEx;
	GUID AcceptExGuid = WSAID_ACCEPTEX;
	DWORD BytesReturned;
	int Errno;


	if (MainUpCallTable.lpWPUQuerySocketHandleContext(sListenSocket,
		(LPDWORD) &ListenSocketContext,
		&Errno) == SOCKET_ERROR)
		return FALSE;

	if (MainUpCallTable.lpWPUQuerySocketHandleContext(sAcceptSocket,
		(LPDWORD) &AcceptSocketContext,
		&Errno) == SOCKET_ERROR)
		return FALSE;

	if (NextProcTable.lpWSPIoctl(ListenSocketContext->ProviderSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&AcceptExGuid, sizeof(GUID), (LPVOID) &lpProviderAcceptEx, sizeof(LPFN_ACCEPTEX), 
		&BytesReturned, NULL, NULL, NULL, &Errno) == SOCKET_ERROR)
	{
		return FALSE;
	}

	// Check for overlapped I/O
	
	if (lpOverlapped)
	{
		ProviderOverlapped = GetOverlappedStructure(sListenSocket, ListenSocketContext->ProviderSocket, lpOverlapped, NULL,
			NULL, NULL);

		return lpProviderAcceptEx(
			ListenSocketContext->ProviderSocket,
			AcceptSocketContext->ProviderSocket,
			lpOutputBuffer,
			dwReceiveDataLength,
			dwLocalAddressLength,
			dwRemoteAddressLength,
			lpdwBytesReceived,
			ProviderOverlapped);
	}
	else
	{
		return lpProviderAcceptEx(
			ListenSocketContext->ProviderSocket,
			AcceptSocketContext->ProviderSocket,
			lpOutputBuffer,
			dwReceiveDataLength,
			dwLocalAddressLength,
			dwRemoteAddressLength,
			lpdwBytesReceived,
			NULL);
	}

	return FALSE;
}
