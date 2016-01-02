// Module Name: select.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the select() API I/O model. This sample is
//    implemented as a console-style application and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts
//    them as they arrive. When this application receives data from a client,
//    it simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
// Compile:
//
//    cl -o select select.cpp ws2_32.lib
//
// Command Line Options:
//
//    select.exe 
//
//    Note: There are no command line options for this sample.
//
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define PORT 5150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   SOCKET Socket;
   OVERLAPPED Overlapped;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Index);

DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[FD_SETSIZE];

void main(void)
{
   SOCKET ListenSocket;
   SOCKET AcceptSocket;
   SOCKADDR_IN InternetAddr;
   WSADATA wsaData;
   INT Ret;
   FD_SET WriteSet;
   FD_SET ReadSet;
   DWORD i;
   DWORD Total;
   ULONG NonBlock;
   DWORD Flags;
   DWORD SendBytes;
   DWORD RecvBytes;


   if ((Ret = WSAStartup(0x0202,&wsaData)) != 0)
   {
      printf("WSAStartup() failed with error %d\n", Ret);
      WSACleanup();
      return;
   }

   // Prepare a socket to listen for connections.

   if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
   {
      printf("WSASocket() failed with error %d\n", WSAGetLastError());
      return;
   }

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);

   if (bind(ListenSocket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr))
      == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return;
   }

   if (listen(ListenSocket, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return;
   }

   // Change the socket mode on the listening socket from blocking to
   // non-block so the application will not block waiting for requests.

   NonBlock = 1;
   if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
   {
      printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
      return;
   }

   while(TRUE)
   {
      // Prepare the Read and Write socket sets for network I/O notification.
      FD_ZERO(&ReadSet);
      FD_ZERO(&WriteSet);

      // Always look for connection attempts.

      FD_SET(ListenSocket, &ReadSet);

      // Set Read and Write notification for each socket based on the
      // current state the buffer.  If there is data remaining in the
      // buffer then set the Write set otherwise the Read set.

      for (i = 0; i < TotalSockets; i++)
         if (SocketArray[i]->BytesRECV > SocketArray[i]->BytesSEND)
            FD_SET(SocketArray[i]->Socket, &WriteSet);
         else
            FD_SET(SocketArray[i]->Socket, &ReadSet);

      if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
      {
         printf("select() returned with error %d\n", WSAGetLastError());
         return;
      }

      // Check for arriving connections on the listening socket.
      if (FD_ISSET(ListenSocket, &ReadSet))
      {
         Total--;
         if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
         {

            // Set the accepted socket to non-blocking mode so the server will
            // not get caught in a blocked condition on WSASends

            NonBlock = 1;
            if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
            {
               printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
               return;
            }

            if (CreateSocketInformation(AcceptSocket) == FALSE)
               return;

         }
         else
         {		
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
               printf("accept() failed with error %d\n", WSAGetLastError());
               return;
            }
         }
      }

      // Check each socket for Read and Write notification until the number
      // of sockets in Total is satisfied.

      for (i = 0; Total > 0 && i < TotalSockets; i++)
      {
         LPSOCKET_INFORMATION SocketInfo = SocketArray[i];

         // If the ReadSet is marked for this socket then this means data
         // is available to be read on the socket.

         if (FD_ISSET(SocketInfo->Socket, &ReadSet))
         {
            Total--;

            SocketInfo->DataBuf.buf = SocketInfo->Buffer;
            SocketInfo->DataBuf.len = DATA_BUFSIZE;

            Flags = 0;
            if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
               &Flags, NULL, NULL) == SOCKET_ERROR)
            {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  printf("WSARecv() failed with error %d\n", WSAGetLastError());

                  FreeSocketInformation(i);
               }

               continue;
            } 
            else
            {
               SocketInfo->BytesRECV = RecvBytes;

               // If zero bytes are received, this indicates the peer closed the
               // connection.
               if (RecvBytes == 0)
               {
                  FreeSocketInformation(i);
                  continue;
               }
            }
         }


         // If the WriteSet is marked on this socket then this means the internal
         // data buffers are available for more data.

         if (FD_ISSET(SocketInfo->Socket, &WriteSet))
         {
            Total--;

            SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
            SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

            if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
               NULL, NULL) == SOCKET_ERROR)
            {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  printf("WSASend() failed with error %d\n", WSAGetLastError());

                  FreeSocketInformation(i);
               }

               continue;
            }
            else
            {
               SocketInfo->BytesSEND += SendBytes;

               if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
               {
                  SocketInfo->BytesSEND = 0;
                  SocketInfo->BytesRECV = 0;
               }
            }
         }
      }
   }
}

BOOL CreateSocketInformation(SOCKET s)
{
   LPSOCKET_INFORMATION SI;
      
   printf("Accepted socket number %d\n", s);

   if ((SI = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
      sizeof(SOCKET_INFORMATION))) == NULL)
   {
      printf("GlobalAlloc() failed with error %d\n", GetLastError());
      return FALSE;
   }

   // Prepare SocketInfo structure for use.

   SI->Socket = s;
   SI->BytesSEND = 0;
   SI->BytesRECV = 0;

   SocketArray[TotalSockets] = SI;

   TotalSockets++;

   return(TRUE);
}

void FreeSocketInformation(DWORD Index)
{
   LPSOCKET_INFORMATION SI = SocketArray[Index];
   DWORD i;

   closesocket(SI->Socket);

   printf("Closing socket number %d\n", SI->Socket);

   GlobalFree(SI);

   // Squash the socket array

   for (i = Index; i < TotalSockets; i++)
   {
      SocketArray[i] = SocketArray[i + 1];
   }

   TotalSockets--;
}
