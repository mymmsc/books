// Module Name: overlap.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the Overlapped I/O model with event notification. This 
//    sample is implemented as a console-style application and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts them
//    as they arrive. When this application receives data from a client, it
//    simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
// Compile:
//
//    cl -o overlap overlap.cpp ws2_32.lib
//
// Command Line Options:
//
//    overlap.exe 
//
//    Note: There are no command line options for this sample.

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define PORT 5150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   SOCKET Socket;
   WSAOVERLAPPED Overlapped;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

DWORD WINAPI ProcessIO(LPVOID lpParameter);

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION CriticalSection;   

void main(void)
{
   WSADATA wsaData;
   SOCKET ListenSocket, AcceptSocket;
   SOCKADDR_IN InternetAddr;
   DWORD Flags;
   DWORD ThreadId;
   DWORD RecvBytes;
   INT Ret;

   InitializeCriticalSection(&CriticalSection);

   if ((Ret = WSAStartup(0x0202,&wsaData)) != 0)
   {
      printf("WSAStartup failed with error %d\n", Ret);
      WSACleanup();
      return;
   }

   if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
   {
      printf("Failed to get a socket %d\n", WSAGetLastError());
      return;
   }

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);

   if (bind(ListenSocket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return;
   }

   if (listen(ListenSocket, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return;
   }

   // Setup the listening socket for connections.

   if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
   {
      printf("Failed to get a socket %d\n", WSAGetLastError());
      return;
   }

   if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
   {
      printf("WSACreateEvent failed with error %d\n", WSAGetLastError());
      return;
   }

   // Create a thread to service overlapped requests

   if (CreateThread(NULL, 0, ProcessIO, NULL, 0, &ThreadId) == NULL)
   {
      printf("CreateThread failed with error %d\n", GetLastError());
      return;
   } 

   EventTotal = 1;

   while(TRUE)
   {
       // Accept inbound connections

      if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) == INVALID_SOCKET)
      {
          printf("accept failed with error %d\n", WSAGetLastError());
          return;
      }

      EnterCriticalSection(&CriticalSection);

      // Create a socket information structure to associate with the accepted socket.

      if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
         sizeof(SOCKET_INFORMATION))) == NULL)
      {
         printf("GlobalAlloc() failed with error %d\n", GetLastError());
         return;
      } 

      // Fill in the details of our accepted socket.

      SocketArray[EventTotal]->Socket = AcceptSocket;
      ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
      SocketArray[EventTotal]->BytesSEND = 0;
      SocketArray[EventTotal]->BytesRECV = 0;
      SocketArray[EventTotal]->DataBuf.len = DATA_BUFSIZE;
      SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

      if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] = 
          WSACreateEvent()) == WSA_INVALID_EVENT)
      {
         printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
         return;
      }

      // Post a WSARecv request to to begin receiving data on the socket

      Flags = 0;
      if (WSARecv(SocketArray[EventTotal]->Socket, 
         &(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags,
         &(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != ERROR_IO_PENDING)
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }

      EventTotal++;

      LeaveCriticalSection(&CriticalSection);

      //
      // Signal the first event in the event array to tell the worker thread to
      // service an additional event in the event array
      //
      if (WSASetEvent(EventArray[0]) == FALSE)
      {
         printf("WSASetEvent failed with error %d\n", WSAGetLastError());
         return;
      }
   }

}


DWORD WINAPI ProcessIO(LPVOID lpParameter)
{
   DWORD Index;
   DWORD Flags;
   LPSOCKET_INFORMATION SI;
   DWORD BytesTransferred;
   DWORD i;
   DWORD RecvBytes, SendBytes;

   // Process asynchronous WSASend, WSARecv requests.

   while(TRUE)
   {

      if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
         WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
      {
         printf("WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
         return 0;
      } 

      // If the event triggered was zero then a connection attempt was made
      // on our listening socket.
 
      if ((Index - WSA_WAIT_EVENT_0) == 0)
      {
         WSAResetEvent(EventArray[0]);
         continue;
      }

      SI = SocketArray[Index - WSA_WAIT_EVENT_0];
      WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

      if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred,
         FALSE, &Flags) == FALSE || BytesTransferred == 0)
      {
         printf("Closing socket %d\n", SI->Socket);

         if (closesocket(SI->Socket) == SOCKET_ERROR)
         {
            printf("closesocket() failed with error %d\n", WSAGetLastError());
         }

         GlobalFree(SI);
         WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

         // Cleanup SocketArray and EventArray by removing the socket event handle
         // and socket information structure if they are not at the end of the
         // arrays.

         EnterCriticalSection(&CriticalSection);

         if ((Index - WSA_WAIT_EVENT_0) + 1 != EventTotal)
            for (i = Index - WSA_WAIT_EVENT_0; i < EventTotal; i++)
            {
               EventArray[i] = EventArray[i + 1];
               SocketArray[i] = SocketArray[i + 1];
            }

         EventTotal--;

         LeaveCriticalSection(&CriticalSection);

         continue;
      }

      // Check to see if the BytesRECV field equals zero. If this is so, then
      // this means a WSARecv call just completed so update the BytesRECV field
      // with the BytesTransferred value from the completed WSARecv() call.

      if (SI->BytesRECV == 0)
      {
         SI->BytesRECV = BytesTransferred;
         SI->BytesSEND = 0;
      }
      else
      {
         SI->BytesSEND += BytesTransferred;
      }

      if (SI->BytesRECV > SI->BytesSEND)
      {

         // Post another WSASend() request.
         // Since WSASend() is not gauranteed to send all of the bytes requested,
         // continue posting WSASend() calls until all received bytes are sent.

         ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
         SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

         SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
         SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

         if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
            &(SI->Overlapped), NULL) == SOCKET_ERROR)
         {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
               printf("WSASend() failed with error %d\n", WSAGetLastError());
               return 0;
            }
         }
      }
      else
      {
         SI->BytesRECV = 0;

         // Now that there are no more bytes to send post another WSARecv() request.

         Flags = 0;
         ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
         SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

         SI->DataBuf.len = DATA_BUFSIZE;
         SI->DataBuf.buf = SI->Buffer;

         if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
            &(SI->Overlapped), NULL) == SOCKET_ERROR)
         {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
               printf("WSARecv() failed with error %d\n", WSAGetLastError());
               return 0;
            }
         }
      }
   }
}
