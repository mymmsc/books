// Module Name: asyncselect.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains the I/O manager for all WSAAsyncselect I/O operations
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

#include <windows.h>

#define PROVIDER_CLASS L"Layered WS2 Provider"

extern HINSTANCE HDllInstance;

typedef struct _SOCK_LIST {
	SOCKET AppSocket;
	SOCKET ProvSocket;
	HWND hWnd;
	UINT uMsg;
	struct _SOCK_LIST *Next;
} SOCK_LIST;

static DWORD WINAPI AsyncMsgHandler(LPVOID lpParameter);
static LRESULT CALLBACK AsyncWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HANDLE WorkerThreadHandle = NULL;
SOCKET CallerSocket = INVALID_SOCKET;
UINT CallerMsg;
HWND CallerhWnd;
HWND AsyncWindow = NULL;

SOCK_LIST *SockList = NULL;

BOOL DuplicateAsyncSocket(SOCKET ProvSock, SOCKET NewProvSock, SOCKET NewSock)
{
	SOCK_LIST *tslp, *slp;

	slp = SockList;

	while(slp)
	{
		if (ProvSock == slp->ProvSocket)
		{

			if ((tslp = (SOCK_LIST *) GlobalAlloc(GPTR, sizeof(SOCK_LIST))) != NULL)
			{
				tslp->Next = SockList;
				tslp->AppSocket = NewSock;
				tslp->ProvSocket = NewProvSock;
				tslp->hWnd = slp->hWnd;
				tslp->uMsg = slp->uMsg;
				SockList = tslp;
				return TRUE;
			}
			return FALSE;
		}
		slp = slp->Next;
	}

	return FALSE;
}

BOOL RemoveSockInfo(SOCKET ProvSock)
{
	SOCK_LIST *slp, *prev;

	slp = SockList;

	prev = NULL;

	while(slp)
	{
		if (ProvSock == slp->ProvSocket)
		{
			if (prev)
				prev->Next = slp->Next;
			else
				SockList = slp->Next;

			GlobalFree(slp);
			return TRUE;
		}
		prev = slp;
		slp = slp->Next;
	}
	return FALSE;
}

BOOL InsertSockInfo(SOCKET ProvSock, SOCKET AppSock, HWND hWnd, UINT uMsg)
{
	SOCK_LIST *tslp, *slp;

	slp = SockList;

	while(slp)
	{
		if (ProvSock == slp->ProvSocket)
		{
			slp->AppSocket = AppSock;
			slp->hWnd = hWnd;
			slp->uMsg = uMsg;
			return TRUE;
		}

		slp = slp->Next;
	}

	if ((tslp = (SOCK_LIST *) GlobalAlloc(GPTR, sizeof(SOCK_LIST))) != NULL)
	{
		tslp->Next = SockList;
		tslp->AppSocket = AppSock;
		tslp->ProvSocket = ProvSock;
		tslp->hWnd = hWnd;
		tslp->uMsg = uMsg;
		SockList = tslp;

		return TRUE;
	}

	return FALSE;
}

SOCK_LIST *GetCallerSocket(SOCKET ProvSock)
{
	SOCK_LIST *slp = SockList;

	while(slp)
	{
		if (slp->ProvSocket == ProvSock)
			return slp;
		slp = slp->Next;
	}
	return NULL;
}

HWND SetWorkerWindow(SOCKET ProvSock, SOCKET AppSock, HWND hWnd, UINT uMsg)
{
	DWORD ThreadId;

	if (WorkerThreadHandle == NULL) 
	{
		WNDCLASS wndclass;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = (WNDPROC)AsyncWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = HDllInstance;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = PROVIDER_CLASS;

		if(RegisterClass(&wndclass) == 0)
			return NULL;

		// Create a window.

		AsyncWindow = CreateWindow(
			PROVIDER_CLASS,
			L"Layered Hidden Window",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			HDllInstance,
			NULL
		);

		if (AsyncWindow == NULL)
		{
			return NULL;
		}

		if ((WorkerThreadHandle = CreateThread(NULL, 0, AsyncMsgHandler, NULL, 0, &ThreadId)) == NULL)
			return NULL;
	}

	if (!InsertSockInfo(ProvSock, AppSock, hWnd, uMsg))
	{
		return NULL;
	}

	return AsyncWindow;
}


static DWORD WINAPI AsyncMsgHandler(LPVOID lpParameter)
{
	MSG msg;
	DWORD Ret;

	while((Ret = GetMessage (&msg, AsyncWindow, 0, 0)))
	{
		if (Ret == -1)
		{
			return 0;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}


static LRESULT CALLBACK AsyncWndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	SOCK_LIST *slp;

	if (uMsg == WM_SOCKET)
	{
		if (slp = GetCallerSocket(wParam))
		{
			MainUpCallTable.lpWPUPostMessage(slp->hWnd, slp->uMsg, slp->AppSocket, lParam);
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
