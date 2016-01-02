// Module Name: overlapped.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains an overlapped I/O manager that supports the event
//    object I/O model, completion routine I/O model, and the I/O completion
//    port model that is used in winsock.
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

#define POOLSIZE 64

typedef struct _WSAOVERLAPPEDPLUS
{
    WSAOVERLAPPED ProviderOverlapped;
    SOCKET CallerSocket;
    SOCKET ProviderSocket;
    LPWSAOVERLAPPED lpCallerOverlapped;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCallerCompletionRoutine;
    LPWSATHREADID lpCallerThreadId;
    DWORD BytesTransferred;
    DWORD Flags;
    DWORD Error;
    DWORD *ByteCount;
    _WSAOVERLAPPEDPLUS *Next;
} WSAOVERLAPPEDPLUS, * LPWSAOVERLAPPEDPLUS;

LPWSAOVERLAPPEDPLUS PoolHead, PoolTail;
WSAOVERLAPPEDPLUS OverlappedPool[POOLSIZE];
HANDLE WorkerThread = NULL,
       WorkerMutex  = NULL;
WSAEVENT EventArray[POOLSIZE];
CRITICAL_SECTION gOverlappedCS;

DWORD WINAPI OverlappedManager(LPVOID lpParameter);
void CALLBACK IntermediateCompletionRoutine(DWORD dwContext);

static TCHAR Msg[512];

void InitOverlappedCS(void)
{
    InitializeCriticalSection(&gOverlappedCS);
}


LPWSAOVERLAPPED GetOverlappedStructure(SOCKET CallerSocket,
                                       SOCKET ProviderSocket,
                                       LPWSAOVERLAPPED lpCallerOverlapped,
                                       LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCallerCompletionRoutine, 
                                       LPWSATHREADID lpCallerThreadId,
                                       DWORD *ByteCount)
{
    LPWSAOVERLAPPEDPLUS lpWorkerOverlappedPlus;
    SOCK_INFO *SocketContext;
    INT Error;

    if (!WorkerThread)
    {
        int i;
        DWORD ThreadId;
        BOOL EventFailed = FALSE;

        EnterCriticalSection(&gOverlappedCS);

        if (!WorkerThread)
        {
            WorkerMutex = CreateMutex(NULL, FALSE, NULL);

            // Initialize the overlapped plus structure pool

            PoolHead = &OverlappedPool[0];

            for(i = 1; i < POOLSIZE; i++)
            {
                OverlappedPool[i - 1].Next = &OverlappedPool[i];
            }

            PoolTail = &OverlappedPool[POOLSIZE - 1];
            PoolTail->Next = NULL;

            // Setup event object handles for the pool
        
            for (i = 0; i < POOLSIZE; i++)
            {
                if ((EventArray[i] = OverlappedPool[i].ProviderOverlapped.hEvent = 
                    MainUpCallTable.lpWPUCreateEvent(&Error)) == NULL)
                    EventFailed = TRUE;
            }

            // Start a worker thread

            WorkerThread = CreateThread(NULL, 0, OverlappedManager, NULL, 0,
                &ThreadId);
        }

        LeaveCriticalSection(&gOverlappedCS);

        if (!WorkerThread || EventFailed)
            return NULL;
    }

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(CallerSocket, (LPDWORD) &SocketContext, &Error) == SOCKET_ERROR)
        return NULL;

    // We have to keep track of the number of outstanding overlapped requests an application
    // has. Otherwise, if the app were to close a socket that had oustanding overlapped ops
    // remaining, we'd start leaking structures in OverlappedPool. The idea here is to force
    // the CallerSocket to remain open until the lower provider has processed all the 
    // overlapped requests. If we closed both the lower socket and the caller socket, we
    // would no longer be able to correlate completed requests to any apps sockets.

    (SocketContext->dwOutstandingAsync)++;

    // Get an Overlapped plus structure from the queue
    WaitForSingleObject(WorkerMutex, INFINITE);

    if (PoolHead)
    {
        lpWorkerOverlappedPlus = PoolHead;
        PoolHead = PoolHead->Next;
    }
    else // Empty pool
    {
        ReleaseMutex(WorkerMutex);
        return NULL;
    }
    ReleaseMutex(WorkerMutex);

    // Set callers overlapped status

    lpCallerOverlapped->Internal = WSS_OPERATION_IN_PROGRESS;
    lpCallerOverlapped->InternalHigh = 0;

    lpWorkerOverlappedPlus->CallerSocket = CallerSocket;
    lpWorkerOverlappedPlus->ProviderSocket = ProviderSocket;
    lpWorkerOverlappedPlus->lpCallerOverlapped = lpCallerOverlapped;
    lpWorkerOverlappedPlus->lpCallerCompletionRoutine = lpCallerCompletionRoutine;
    lpWorkerOverlappedPlus->lpCallerThreadId = lpCallerThreadId;
    lpWorkerOverlappedPlus->ByteCount = ByteCount;

    return(&(lpWorkerOverlappedPlus->ProviderOverlapped));
}

DWORD WINAPI OverlappedManager(LPVOID lpParameter)
{
    SOCK_INFO *SocketContext;
    DWORD Ret, Ret2;
    INT OverlappedResultError;
    INT Error;

    while((Ret = WaitForMultipleObjects(POOLSIZE, EventArray, FALSE,
            INFINITE)) != WAIT_FAILED)
    {
        if (Ret >= WAIT_OBJECT_0 && Ret <= (WAIT_OBJECT_0 + POOLSIZE - 1))
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(OverlappedPool[Ret - WAIT_OBJECT_0].CallerSocket,
                                                              (LPDWORD) &SocketContext, 
                                                              &Error) == SOCKET_ERROR)
                continue;

            // Reset event handle
            MainUpCallTable.lpWPUResetEvent(EventArray[Ret - WAIT_OBJECT_0], &Error);

            // Get Overlapped I/O results

            Ret2 = NextProcTable.lpWSPGetOverlappedResult(
                OverlappedPool[Ret - WAIT_OBJECT_0].ProviderSocket,
                &OverlappedPool[Ret - WAIT_OBJECT_0].ProviderOverlapped,
                &OverlappedPool[Ret - WAIT_OBJECT_0].BytesTransferred,
                TRUE,
                &OverlappedPool[Ret - WAIT_OBJECT_0].Flags,
                &OverlappedResultError);

            if (Ret2 != 0)
            {
                if (OverlappedPool[Ret - WAIT_OBJECT_0].ByteCount)
                {
                    *(OverlappedPool[Ret - WAIT_OBJECT_0].ByteCount) += 
                        OverlappedPool[Ret - WAIT_OBJECT_0].BytesTransferred;
                }
            }

            // When an I/O operation is complete, the provider sets OffsetHigh
            // to the Windows Sockets 2 error code resulting from the operation,
            // sets Offset to the flags resulting from the I/O operation, and
            // calls WPUCompleteOverlappedRequest, passing the transfer byte count
            // as one of the parameters. WPUCompleteOverlappedRequest eventually
            // sets InternalHigh to the transfer byte count, then sets Internal
            // to a value other than WSS_OPERATION_IN_PROGRESS.
            OverlappedPool[Ret - WAIT_OBJECT_0].lpCallerOverlapped->OffsetHigh =
                ((Ret2 == 0) ? WSAENOTSOCK : OverlappedResultError);

            OverlappedPool[Ret - WAIT_OBJECT_0].lpCallerOverlapped->Offset = 
                OverlappedPool[Ret - WAIT_OBJECT_0].Flags;    

            if (OverlappedPool[Ret - WAIT_OBJECT_0].lpCallerCompletionRoutine)
            {
                // Handle callers completion routine
                MainUpCallTable.lpWPUQueueApc(
                    OverlappedPool[Ret - WAIT_OBJECT_0].lpCallerThreadId,
                    IntermediateCompletionRoutine,
                    (DWORD) &OverlappedPool[Ret - WAIT_OBJECT_0],   
               	    &Error);
            }
            else
            {
                // Signal callers overlapped event handle
                WPUCompleteOverlappedRequest(
                    OverlappedPool[Ret - WAIT_OBJECT_0].CallerSocket,
                    OverlappedPool[Ret - WAIT_OBJECT_0].lpCallerOverlapped,
                    OverlappedResultError,                 
                    OverlappedPool[Ret - WAIT_OBJECT_0].BytesTransferred,             
                    &Error);
            }

            (SocketContext->dwOutstandingAsync)--;

            if ((SocketContext->bClosing) && (SocketContext->dwOutstandingAsync == 0))
            {
                RemoveSockInfo(SocketContext->ProviderSocket);

                if (MainUpCallTable.lpWPUCloseSocketHandle(OverlappedPool[Ret-WAIT_OBJECT_0].CallerSocket, &Error) == SOCKET_ERROR)
                    return SOCKET_ERROR;

                {
                    TCHAR buffer[128];
                    wsprintf(buffer, L"Closing socket %d Bytes Sent [%lu] Bytes Recv [%lu]\n", 
                        OverlappedPool[Ret-WAIT_OBJECT_0].CallerSocket,
                        SocketContext->BytesSent, SocketContext->BytesRecv);
                    OutputDebugString(buffer);
                }

                GlobalFree(SocketContext);
            }
            // Put Overlapped plus structure back in the pool

            WaitForSingleObject(WorkerMutex, INFINITE);
            PoolTail->Next = &OverlappedPool[Ret - WAIT_OBJECT_0];
            PoolTail = &OverlappedPool[Ret - WAIT_OBJECT_0];
            PoolTail->Next = NULL;
            ReleaseMutex(WorkerMutex);
        }
    }
    return 0;
}


void CALLBACK IntermediateCompletionRoutine(DWORD dwContext)
{
    LPWSAOVERLAPPEDPLUS olp = (LPWSAOVERLAPPEDPLUS) dwContext;

    olp->lpCallerCompletionRoutine(olp->Error, olp->BytesTransferred,
        olp->lpCallerOverlapped, olp->Flags);
}
