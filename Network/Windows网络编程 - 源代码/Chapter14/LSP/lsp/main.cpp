// Module Name: main.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through an IP socket. The application
//    reports when sockets are created and reports how many bytes were sent and
//    received when a socket closes. The results are reported using the OutputDebugString
//    API which will allow you to intercept the I/O by using a debugger such as cdb.exe
//    or you can monitor the I/O using dbmon.exe.
//
//    This file contains the 30 SPI functions you are required to implement in a
//    service provider. It also contains the two functions that must be exported
//    from the DLL module DllMain and WSPStartup.
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

WSPUPCALLTABLE MainUpCallTable;
DWORD gLayerCatId = 0;
DWORD gChainId = 0;
DWORD gEntryCount = 0;
CRITICAL_SECTION gCriticalSection;
LPWSPDATA gWSPData = NULL;
WSPPROC_TABLE NextProcTable;
LPWSPPROC_TABLE gProcTable = NULL;
LPWSAPROTOCOL_INFOW gBaseInfo = NULL;
HINSTANCE HDllInstance = NULL;
HINSTANCE hProvider = NULL;
INT gLayerCount=0;                    // Number of base providers we're layered over

static TCHAR Msg[512];

BOOL WINAPI DllMain(IN HINSTANCE hinstDll, IN DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason)
    {

        case DLL_PROCESS_ATTACH:
            HDllInstance = hinstDll;
            InitializeCriticalSection(&gCriticalSection);
//            InitAsyncSelectCS();
            InitOverlappedCS();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

SOCKET WSPAPI WSPAccept (
    SOCKET s,                      
    struct sockaddr FAR * addr,  
    LPINT addrlen,                 
    LPCONDITIONPROC lpfnCondition,  
    DWORD dwCallbackData,          
    LPINT lpErrno)
{
    SOCKET NewProviderSocket;
    SOCKET NewSocket;
    SOCK_INFO *NewSocketContext;

    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return INVALID_SOCKET;

    NewProviderSocket = NextProcTable.lpWSPAccept(SocketContext->ProviderSocket, addr, addrlen,
                                                    lpfnCondition, dwCallbackData, lpErrno);
    
    
    if (NewProviderSocket != INVALID_SOCKET)
    {
        if ((NewSocketContext = (SOCK_INFO *) GlobalAlloc(GPTR, sizeof SOCK_INFO)) == NULL)
        {
            *lpErrno = WSAENOBUFS;
            return INVALID_SOCKET;
        }
        NewSocketContext->ProviderSocket = NewProviderSocket;
        NewSocketContext->bClosing  = FALSE;
        NewSocketContext->dwOutstandingAsync = 0;
        NewSocketContext->BytesRecv = 0;
        NewSocketContext->BytesSent = 0;

        if ((NewSocket = MainUpCallTable.lpWPUCreateSocketHandle(gChainId, (DWORD) NewSocketContext, lpErrno)) != INVALID_SOCKET)
            DuplicateAsyncSocket(SocketContext->ProviderSocket, NewProviderSocket, NewSocket);

        {
            TCHAR buffer[128];
            wsprintf(buffer, L"Creating socket %d\n", NewSocket);
            OutputDebugString(buffer);
        }

        return NewSocket;
    }

    return INVALID_SOCKET;
}

int WSPAPI WSPAddressToString(
    LPSOCKADDR lpsaAddress,            
    DWORD dwAddressLength,               
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPWSTR lpszAddressString,            
    LPDWORD lpdwAddressStringLength,   
    LPINT lpErrno)
{
    return NextProcTable.lpWSPAddressToString(lpsaAddress, dwAddressLength,               
        &gBaseInfo[0], lpszAddressString, lpdwAddressStringLength, lpErrno);
}

int WSPAPI WSPAsyncSelect (
    SOCKET s,              
    HWND hWnd,           
    unsigned int wMsg,     
    long lEvent,           
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    HWND hWorkerWindow;


    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    if ((hWorkerWindow = SetWorkerWindow(SocketContext->ProviderSocket, s, hWnd, wMsg)) == NULL)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPAsyncSelect(SocketContext->ProviderSocket, hWorkerWindow, WM_SOCKET, lEvent, lpErrno);
}

int WSPAPI WSPBind(
    SOCKET s,                           
    const struct sockaddr FAR * name,
    int namelen,                        
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPBind(SocketContext->ProviderSocket, name, namelen, lpErrno);
}

int WSPAPI WSPCancelBlockingCall(
    LPINT lpErrno)
{
    return NextProcTable.lpWSPCancelBlockingCall(lpErrno);
}

int WSPAPI WSPCleanup (
    LPINT lpErrno  
    )
{
    int Ret;

    if (!gEntryCount)
    {
        *lpErrno = WSANOTINITIALISED;
        return SOCKET_ERROR;
    }

    Ret = NextProcTable.lpWSPCleanup(lpErrno);

    EnterCriticalSection(&gCriticalSection);

    gEntryCount--;

    if (gEntryCount == 0)
    {
        FreeLibrary(hProvider);
        hProvider = NULL;
    }

    LeaveCriticalSection(&gCriticalSection);

    return Ret;
}

int WSPAPI WSPCloseSocket (  
    SOCKET s,        
    LPINT lpErrno
)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    if (SocketContext->dwOutstandingAsync != 0)
    {
        SocketContext->bClosing = TRUE;

        if (NextProcTable.lpWSPCloseSocket(SocketContext->ProviderSocket, lpErrno) == SOCKET_ERROR) 
            return SOCKET_ERROR;

        return 0;
    }

    if (NextProcTable.lpWSPCloseSocket(SocketContext->ProviderSocket, lpErrno) == SOCKET_ERROR) 
        return SOCKET_ERROR;

    RemoveSockInfo(SocketContext->ProviderSocket);

    if (MainUpCallTable.lpWPUCloseSocketHandle(s, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    {
        TCHAR buffer[128];
        wsprintf(buffer, L"Closing socket %d Bytes Sent [%lu] Bytes Recv [%lu]\n", s,
            SocketContext->BytesSent, SocketContext->BytesRecv);
        OutputDebugString(buffer);
    }

    GlobalFree(SocketContext);

    return 0;
}

int WSPAPI WSPConnect (
    SOCKET s,                           
    const struct sockaddr FAR * name,
    int namelen,                        
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,              
    LPQOS lpSQOS,
    LPQOS lpGQOS,                       
    LPINT lpErrno
)
{
    SOCK_INFO *SocketContext;
    INT ret;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }

    ret =  NextProcTable.lpWSPConnect(SocketContext->ProviderSocket, name, namelen, lpCallerData, lpCalleeData,
        lpSQOS, lpGQOS, lpErrno);

    return ret;
}

int WSPAPI WSPDuplicateSocket(
    SOCKET s,                             
    DWORD dwProcessId,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPDuplicateSocket(SocketContext->ProviderSocket,                             
        dwProcessId, lpProtocolInfo, lpErrno);
}

int WSPAPI WSPEnumNetworkEvents(  
    SOCKET s,                             
    WSAEVENT hEventObject,                  
    LPWSANETWORKEVENTS lpNetworkEvents,   
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPEnumNetworkEvents(SocketContext->ProviderSocket,                             
        hEventObject, lpNetworkEvents, lpErrno);
}

int WSPAPI WSPEventSelect(
    SOCKET s,                
    WSAEVENT hEventObject,   
    long lNetworkEvents,     
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;
    
    return NextProcTable.lpWSPEventSelect(SocketContext->ProviderSocket, hEventObject,
        lNetworkEvents, lpErrno);
}

BOOL WSPAPI WSPGetOverlappedResult (
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags,
    LPINT lpErrno)
{
    DWORD Ret;

    if (lpOverlapped->Internal!=WSS_OPERATION_IN_PROGRESS) 
    {
        *lpcbTransfer = lpOverlapped->InternalHigh;
        *lpdwFlags = lpOverlapped->Offset;
        *lpErrno = lpOverlapped->OffsetHigh;

        return(lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
    }
    else
        if (fWait) 
        {
            Ret = WaitForSingleObject(lpOverlapped->hEvent, INFINITE);
            if ((Ret == WAIT_OBJECT_0)
                && (lpOverlapped->Internal != WSS_OPERATION_IN_PROGRESS))
            {
                *lpcbTransfer = lpOverlapped->InternalHigh;
                *lpdwFlags = lpOverlapped->Offset;
                *lpErrno = lpOverlapped->OffsetHigh;
                    
                return(lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
            }
            else 
                *lpErrno = WSASYSCALLFAILURE;
        }
        else 
            *lpErrno = WSA_IO_INCOMPLETE;

    return FALSE;
}

int WSPAPI WSPGetPeerName(  
    SOCKET s,                     
    struct sockaddr FAR * name,     
    LPINT namelen,                
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetPeerName(SocketContext->ProviderSocket, name,
        namelen, lpErrno);
}

int WSPAPI WSPGetSockName(
    SOCKET s,                     
    struct sockaddr FAR * name,
    LPINT namelen,                
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetSockName(SocketContext->ProviderSocket, name,
        namelen, lpErrno);
}

int WSPAPI WSPGetSockOpt(
    SOCKET s,              
    int level,           
    int optname,           
    char FAR * optval,     
    LPINT optlen,        
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetSockOpt(SocketContext->ProviderSocket, level,           
        optname, optval, optlen, lpErrno);
}

BOOL WSPAPI WSPGetQOSByName(
    SOCKET s,               
    LPWSABUF lpQOSName,   
    LPQOS lpQOS,            
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetQOSByName(SocketContext->ProviderSocket, lpQOSName,
        lpQOS, lpErrno);
}


int WSPAPI WSPIoctl(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    GUID AcceptExGuid = WSAID_ACCEPTEX;
    GUID TransmitFileGuid = WSAID_TRANSMITFILE;
    GUID GetAcceptExSockAddrsGuid = WSAID_GETACCEPTEXSOCKADDRS;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext,
        lpErrno) == SOCKET_ERROR)
    {
//        MessageBox(NULL, L"wham", L"wham", MB_OK);
        return SOCKET_ERROR;
    }

    if (dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER)
    {
        if (memcmp (lpvInBuffer, &TransmitFileGuid, sizeof (GUID)) == 0)
        {
            *((LPFN_TRANSMITFILE *)lpvOutBuffer) = ExtTransmitFile;
            *lpErrno = 0;
            return 0;
        }
        else
            if (memcmp(lpvInBuffer, &AcceptExGuid, sizeof(GUID)) == 0)
            {
                *((LPFN_ACCEPTEX *)lpvOutBuffer) = ExtAcceptEx;
                *lpErrno = 0;
                return 0;
            }
            else 
                if (memcmp (lpvInBuffer, &GetAcceptExSockAddrsGuid, sizeof (GUID)) == 0)
                {
                    // No socket handle translation needed, let the call pass through below
                }
                else 
                {
                    *lpErrno = WSAEOPNOTSUPP;
                    return SOCKET_ERROR;
                }

    }


    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, NULL);

        Ret = NextProcTable.lpWSPIoctl(SocketContext->ProviderSocket, dwIoControlCode, lpvInBuffer,
            cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, ProviderOverlapped, NULL, NULL, lpErrno);
    }
    else
    {
        Ret = NextProcTable.lpWSPIoctl(SocketContext->ProviderSocket, dwIoControlCode, lpvInBuffer,
            cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }

    return Ret;
}

SOCKET WSPAPI WSPJoinLeaf(
    SOCKET s,                           
    const struct sockaddr FAR * name,     
    int namelen,                        
    LPWSABUF lpCallerData,                
    LPWSABUF lpCalleeData,              
    LPQOS lpSQOS,                         
    LPQOS lpGQOS,                       
    DWORD dwFlags,                        
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    SOCKET NextProviderSocket;
    SOCKET NewSocket;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    NextProviderSocket = NextProcTable.lpWSPJoinLeaf(SocketContext->ProviderSocket,                           
        name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, dwFlags,                        
        lpErrno);
        
    if (NextProviderSocket != INVALID_SOCKET)
    {
        if ((SocketContext = (SOCK_INFO *) GlobalAlloc(GPTR, sizeof SOCK_INFO)) == NULL)
        {
            *lpErrno = WSAENOBUFS;
            return INVALID_SOCKET;
        }
        SocketContext->ProviderSocket = NextProviderSocket;
        SocketContext->bClosing  = FALSE;
        SocketContext->dwOutstandingAsync = 0;
        SocketContext->BytesRecv = 0;
        SocketContext->BytesSent = 0;

        NewSocket = MainUpCallTable.lpWPUCreateSocketHandle(gChainId, (DWORD) SocketContext, lpErrno);

        {
            TCHAR buffer[128];
            wsprintf(buffer, L"Creating socket %d\n", NewSocket);
            OutputDebugString(buffer);
        }

        return NewSocket;
    }

    return INVALID_SOCKET;
}

int WSPAPI WSPListen(
    SOCKET s,        
    int backlog,     
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPListen (SocketContext->ProviderSocket, backlog, lpErrno);
}

int WSPAPI WSPRecv(
    SOCKET s,                                                 
    LPWSABUF lpBuffers,                                       
    DWORD dwBufferCount,                                      
    LPDWORD lpNumberOfBytesRecvd,                             
    LPDWORD lpFlags,                                          
    LPWSAOVERLAPPED lpOverlapped,                             
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
    LPWSATHREADID lpThreadId,                                 
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;


    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesRecv);

        Ret = NextProcTable.lpWSPRecv(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, ProviderOverlapped, NULL, NULL, lpErrno);

    }
    else
    {
        // Process ordinary blocking call

        Ret = NextProcTable.lpWSPRecv(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId,
            lpErrno);

        if (Ret != SOCKET_ERROR)
        {
            SocketContext->BytesRecv += *lpNumberOfBytesRecvd;
        }
    }

    return Ret;
}

int WSPAPI WSPRecvDisconnect(
    SOCKET s,                           
    LPWSABUF lpInboundDisconnectData,     
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPRecvDisconnect(SocketContext->ProviderSocket,                           
        lpInboundDisconnectData, lpErrno);
}

int WSPAPI WSPRecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesRecv);

        if (ProviderOverlapped == NULL)
        {
            OutputDebugString(L"WSPRecvFrom got a NULL overlapp structure\n");
        }

        Ret = NextProcTable.lpWSPRecvFrom(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, ProviderOverlapped, NULL, NULL, lpErrno);

    }
    else
    {
        Ret = NextProcTable.lpWSPRecvFrom(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

        if (Ret != SOCKET_ERROR)
        {
            SocketContext->BytesRecv += *lpNumberOfBytesRecvd;
        }
    }

    return Ret;
}

int WSPAPI WSPSelect(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    u_int i;
    u_int count;
    int Ret;
    int HandleCount;

    // Convert handles
    struct
    {
        SOCKET ClientSocket;
        SOCKET ProvSocket;

    } Read[FD_SETSIZE], Write[FD_SETSIZE], Except[FD_SETSIZE];

    fd_set ReadFds, WriteFds, ExceptFds;

    if (readfds)
    {
        FD_ZERO(&ReadFds);

        for (i = 0; i < readfds->fd_count; i++)
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(
                (Read[i].ClientSocket = readfds->fd_array[i]), 
                (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
                return SOCKET_ERROR;
            FD_SET((Read[i].ProvSocket = SocketContext->ProviderSocket), &ReadFds);
        }
    }

    if (writefds)
    {
        FD_ZERO(&WriteFds);

        for (i = 0; i < writefds->fd_count; i++)
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(
                (Write[i].ClientSocket = writefds->fd_array[i]), 
                (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
                return SOCKET_ERROR;
            FD_SET((Write[i].ProvSocket = SocketContext->ProviderSocket), &WriteFds);
        }
    }

    if (exceptfds)
    {
        FD_ZERO(&ExceptFds);

        for (i = 0; i < exceptfds->fd_count; i++)
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(
                (Except[i].ClientSocket = exceptfds->fd_array[i]), 
                (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
                return SOCKET_ERROR;
            FD_SET((Except[i].ProvSocket = SocketContext->ProviderSocket), &ExceptFds);
        }
    }

    Ret = NextProcTable.lpWSPSelect(nfds, 
        (readfds ? &ReadFds : NULL), (writefds ? &WriteFds : NULL), 
        (exceptfds ? &ExceptFds : NULL), timeout, lpErrno);

    if (Ret != SOCKET_ERROR)
    {
        HandleCount = Ret;

        if (readfds)
        {
            count = readfds->fd_count;
            FD_ZERO(readfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if (MainUpCallTable.lpWPUFDIsSet(Read[i].ProvSocket, &ReadFds))
                {
                    FD_SET(Read[i].ClientSocket, readfds);
                    HandleCount--;
                }
            }
        }

        if (writefds)
        {
            count = writefds->fd_count;
            FD_ZERO(writefds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if (MainUpCallTable.lpWPUFDIsSet(Write[i].ProvSocket, &WriteFds))
                {
                    FD_SET(Write[i].ClientSocket, writefds);
                    HandleCount--;
                }
            }
        }

        if (exceptfds)
        {
            count = exceptfds->fd_count;
            FD_ZERO(exceptfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if (MainUpCallTable.lpWPUFDIsSet(Except[i].ProvSocket, &ExceptFds))
                {
                    FD_SET(Except[i].ClientSocket, exceptfds);
                    HandleCount--;
                }
            }
        }
    }

    return Ret;
}

int WSPAPI WSPSend (
    SOCKET s,                                                 
    LPWSABUF lpBuffers,                                       
    DWORD dwBufferCount,                                      
    LPDWORD lpNumberOfBytesSent,                              
    DWORD dwFlags,                                            
    LPWSAOVERLAPPED lpOverlapped,                             
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
    LPWSATHREADID lpThreadId,                                 
    LPINT lpErrno                                             
    )
{
    INT Ret;
    SOCK_INFO *SocketContext;
    LPWSAOVERLAPPED ProviderOverlapped;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesSent);

        Ret = NextProcTable.lpWSPSend(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, ProviderOverlapped, NULL, NULL, lpErrno);

    }
    else
    {
        Ret = NextProcTable.lpWSPSend(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

        if (Ret != SOCKET_ERROR)
        {
            SocketContext->BytesSent += *lpNumberOfBytesSent;
        }
    }


    return Ret;
}

int WSPAPI WSPSendDisconnect(
    SOCKET s,                            
    LPWSABUF lpOutboundDisconnectData,
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPSendDisconnect(SocketContext->ProviderSocket,
        lpOutboundDisconnectData, lpErrno);
}

int WSPAPI WSPSendTo(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    // Check for overlapped I/O
    
    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesSent);

        Ret = NextProcTable.lpWSPSendTo(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpTo, iTolen, ProviderOverlapped, NULL, NULL, lpErrno);

    }
    else
    {
        Ret = NextProcTable.lpWSPSendTo(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

        if (Ret != SOCKET_ERROR)
        {
            SocketContext->BytesSent += *lpNumberOfBytesSent;
        }

    }

    return Ret;
}

int WSPAPI WSPSetSockOpt(
    SOCKET s,
    int level,                 
    int optname,                 
    const char FAR * optval,   
    int optlen,                  
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPSetSockOpt(SocketContext->ProviderSocket, level,                 
        optname, optval, optlen, lpErrno);
}

int WSPAPI WSPShutdown (
    SOCKET s,        
    int how,         
    LPINT lpErrno)
{
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPShutdown(SocketContext->ProviderSocket, how, lpErrno);
}


int WSPAPI WSPStringToAddress(
    LPWSTR AddressString,                 
    INT AddressFamily,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPSOCKADDR lpAddress,                   
    LPINT lpAddressLength,                
    LPINT lpErrno)
{
    return NextProcTable.lpWSPStringToAddress (AddressString, AddressFamily,
        &gBaseInfo[0], lpAddress, lpAddressLength, lpErrno);
}


SOCKET WSPAPI WSPSocket(
    int af,                               
    int type,
    int protocol,                         
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,                              
    DWORD dwFlags,
    LPINT lpErrno                         
    )
{
    SOCKET NextProviderSocket;
    SOCKET NewSocket;
    SOCK_INFO *SocketContext;
    LPWSAPROTOCOL_INFOW pInfo=NULL;
    INT        iProtocol, iSockType, i;

    iProtocol = (!lpProtocolInfo ? lpProtocolInfo->iProtocol   : protocol);
    iSockType = (!lpProtocolInfo ? lpProtocolInfo->iSocketType : type);
    for(i=0; i < gLayerCount ;i++)
    {
        if ((gBaseInfo[i].iSocketType == iSockType) && 
            (gBaseInfo[i].iProtocol   == iProtocol))
        {
            pInfo = &gBaseInfo[i];
            break;
        }
    }

    NextProviderSocket = NextProcTable.lpWSPSocket(af, type, protocol, (pInfo ? pInfo : lpProtocolInfo),
                                g, dwFlags, lpErrno);

    if (NextProviderSocket != INVALID_SOCKET)
    {
        if ((SocketContext = (SOCK_INFO *) GlobalAlloc(GPTR, sizeof SOCK_INFO)) == NULL)
        {
            *lpErrno = WSAENOBUFS;
            return INVALID_SOCKET;
        }
        SocketContext->ProviderSocket = NextProviderSocket;
        SocketContext->bClosing  = FALSE;
        SocketContext->dwOutstandingAsync = 0;
        SocketContext->BytesRecv = 0;
        SocketContext->BytesSent = 0;

        NewSocket = MainUpCallTable.lpWPUCreateSocketHandle(gChainId, (DWORD) SocketContext, lpErrno);

        {
            TCHAR buffer[128];
            wsprintf(buffer, L"Creating socket %d\n", NewSocket);
            OutputDebugString(buffer);
        }

        return NewSocket;
    }

    return INVALID_SOCKET;
}


int WSPAPI WSPStartup(
    WORD wVersion,
    LPWSPDATA lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    WSPUPCALLTABLE UpCallTable,
    LPWSPPROC_TABLE lpProcTable)
{

    INT      ReturnCode = 0;
    WCHAR    ProviderPath[MAX_PATH];
    INT      ProviderPathLen = MAX_PATH;
    WCHAR    LibraryPath[MAX_PATH];
    INT      i, j, x, y, z;
    INT      TotalProtocols, idx;
    INT      Error;
    DWORD    NextProviderCatId;
    UINT     iBaseId;

    LPWSAPROTOCOL_INFOW ProtocolInfo;
    LPWSAPROTOCOL_INFOW ProtoInfo = lpProtocolInfo;
    LPWSPSTARTUP    WSPStartupFunc = NULL;

    EnterCriticalSection(&gCriticalSection);

    MainUpCallTable = UpCallTable;

    // Load Next Provider in chain if this is the first time called
    if (!gEntryCount)
    {
        OutputDebugString(L"Layered Service Provider\n");

        //  Get all protocol information in database
        if ((ProtocolInfo = GetProviders(&TotalProtocols)) == NULL)
        {
            return  WSAEPROVIDERFAILEDINIT;
        }

        // Find out what our layered protocol catalog ID entry is
        for (i = 0; i < TotalProtocols; i++)
            if (memcmp (&ProtocolInfo[i].ProviderId, &ProviderGuid, sizeof (GUID))==0)
            {
                gLayerCatId = ProtocolInfo[i].dwCatalogEntryId;
                break;
            }

        // Save our protocol chains catalog ID entry
        gChainId = lpProtocolInfo->dwCatalogEntryId;

        gLayerCount=0;
        for(x=0; x < TotalProtocols ;x++)
        {
            for(y=0; y < ProtocolInfo[x].ProtocolChain.ChainLen ;y++)
            {
                if (gLayerCatId == ProtocolInfo[x].ProtocolChain.ChainEntries[y])
                {
                    gLayerCount++;
                    break;
                }
            }
        }
        gBaseInfo = (LPWSAPROTOCOL_INFOW)GlobalAlloc(GPTR, sizeof(WSAPROTOCOL_INFOW)*gLayerCount);
        if (!gBaseInfo)
        {
            return WSAENOBUFS;
        }
        idx=0;
        for(x=0; x < TotalProtocols ;x++)
        {
            for(y=0; y < ProtocolInfo[x].ProtocolChain.ChainLen ;y++)
            {
                if (gLayerCatId == ProtocolInfo[x].ProtocolChain.ChainEntries[y])
                {
                    // Our LSP exists in this entries chain
                    //
                    iBaseId = ProtocolInfo[x].ProtocolChain.ChainEntries[ProtocolInfo[x].ProtocolChain.ChainLen-1];
                    for(z=0; z < TotalProtocols ;z++)
                    {
                        if (ProtocolInfo[z].dwCatalogEntryId == iBaseId)
                        {
                            memcpy(&gBaseInfo[idx++], &ProtocolInfo[z], sizeof(WSAPROTOCOL_INFOW));
                            OutputDebugString(gBaseInfo[idx-1].szProtocol);
                            OutputDebugString(L"\n");
                        }
                    }
                }
            }
        }

        // Find our layered catalog ID entry in the protocol chain
        for(j = 0; j < lpProtocolInfo->ProtocolChain.ChainLen; j++)
        {
            if (lpProtocolInfo->ProtocolChain.ChainEntries[j] == gLayerCatId)
            {

                NextProviderCatId = lpProtocolInfo->ProtocolChain.ChainEntries[j + 1];
                break;
            }
        }


        // Find next provider path to load
        for (i = 0; i < TotalProtocols; i++)
            if (NextProviderCatId == ProtocolInfo[i].dwCatalogEntryId)
            {
                if (WSCGetProviderPath(&ProtocolInfo[i].ProviderId, ProviderPath, &ProviderPathLen, &Error) == SOCKET_ERROR)
                {
                    return  WSAEPROVIDERFAILEDINIT;
                }
                break;
            }

        if (!ExpandEnvironmentStrings(ProviderPath, LibraryPath, MAX_PATH))
        {
            return  WSAEPROVIDERFAILEDINIT;
        }

        if ((hProvider = LoadLibrary(LibraryPath)) == NULL)
        {
            return  WSAEPROVIDERFAILEDINIT;
        }


        if((WSPStartupFunc = (LPWSPSTARTUP) GetProcAddress(hProvider, "WSPStartup")) == NULL)
        {
            return  WSAEPROVIDERFAILEDINIT;
        }

        ReturnCode = (*WSPStartupFunc)(wVersion, lpWSPData, ProtoInfo, UpCallTable, lpProcTable);

        // Save the next providers procedure table
        memcpy(&NextProcTable, lpProcTable, sizeof WSPPROC_TABLE);

        // Remap service provider functions here

        lpProcTable->lpWSPAccept = WSPAccept;
        lpProcTable->lpWSPAddressToString = WSPAddressToString;
        lpProcTable->lpWSPAsyncSelect = WSPAsyncSelect;
        lpProcTable->lpWSPBind = WSPBind;
        lpProcTable->lpWSPCancelBlockingCall = WSPCancelBlockingCall;
        lpProcTable->lpWSPCleanup = WSPCleanup;
        lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
        lpProcTable->lpWSPConnect = WSPConnect;
        lpProcTable->lpWSPDuplicateSocket = WSPDuplicateSocket;
        lpProcTable->lpWSPEnumNetworkEvents = WSPEnumNetworkEvents;
        lpProcTable->lpWSPEventSelect = WSPEventSelect;
        lpProcTable->lpWSPGetOverlappedResult = WSPGetOverlappedResult;
        lpProcTable->lpWSPGetPeerName = WSPGetPeerName;
        lpProcTable->lpWSPGetSockOpt = WSPGetSockOpt;
        lpProcTable->lpWSPGetSockName = WSPGetSockName;
        lpProcTable->lpWSPGetQOSByName = WSPGetQOSByName;
        lpProcTable->lpWSPIoctl = WSPIoctl;
        lpProcTable->lpWSPJoinLeaf = WSPJoinLeaf;
        lpProcTable->lpWSPListen = WSPListen;
        lpProcTable->lpWSPRecv = WSPRecv;
        lpProcTable->lpWSPRecvDisconnect = WSPRecvDisconnect;
        lpProcTable->lpWSPRecvFrom = WSPRecvFrom;
        lpProcTable->lpWSPSelect = WSPSelect;
        lpProcTable->lpWSPSend = WSPSend;
        lpProcTable->lpWSPSendDisconnect = WSPSendDisconnect;
        lpProcTable->lpWSPSendTo = WSPSendTo;
        lpProcTable->lpWSPSetSockOpt = WSPSetSockOpt;
        lpProcTable->lpWSPShutdown = WSPShutdown;
        lpProcTable->lpWSPSocket = WSPSocket;
        lpProcTable->lpWSPStringToAddress = WSPStringToAddress;

        gWSPData = lpWSPData;
        gProcTable = lpProcTable;
    } else
    {
        lpWSPData = gWSPData;
        lpProcTable = gProcTable;
        ReturnCode = 0;
    }

    gEntryCount++;

    LeaveCriticalSection(&gCriticalSection);

    return(ReturnCode);
}

