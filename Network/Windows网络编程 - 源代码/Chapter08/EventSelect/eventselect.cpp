// Module Name: eventselect.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the WSAEventSelect() I/O model. This sample is
//    implemented as a console-style application and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts them
//    as they arrive. When this application receives data from a client, it
//    simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
// Compile:
//
//    cl -o eventselect eventselect.cpp ws2_32.lib
//
// Command Line Options:
//
//    eventselect.exe 
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
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Event);

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];

void main(void)
{
   SOCKET Listen;
   SOCKET Accept;
   SOCKADDR_IN InternetAddr;
   DWORD Event;
   WSANETWORKEVENTS NetworkEvents;
   WSADATA wsaData;
   DWORD Ret;
   DWORD Flags;
   DWORD RecvBytes;
   DWORD SendBytes;

   if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
   {
      printf("WSAStartup() failed with error %d\n", Ret);
      return;
   }

   if ((Listen = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
   {
      printf("socket() failed with error %d\n", WSAGetLastError());
      return;
   } 

   CreateSocketInformation(Listen);

   if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_ACCEPT|FD_CLOSE) == SOCKET_ERROR)
   {
      printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
      return;
   }

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);

   if (bind(Listen, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return;
   }

   if (listen(Listen, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return;
   }
     		

   while(TRUE)
   {
      // Wait for one of the sockets to receive I/O notification and 
      if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
         WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
      {
         printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
         return;
      }


      if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event - 
         WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
      {
         printf("WSAEnumNetworkEvents failed with error %d\n", WSAGetLastError());
         return;
      }

      
      if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
      {
         if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
         {
            printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
            break;
         }

         if ((Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
         {		
            printf("accept() failed with error %d\n", WSAGetLastError());
            break;
         }

         if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
         {
            printf("Too many connections - closing socket.\n");
            closesocket(Accept);
            break;
         }

         CreateSocketInformation(Accept);

         if (WSAEventSelect(Accept, EventArray[EventTotal - 1], FD_READ|FD_WRITE|FD_CLOSE) == SOCKET_ERROR)
         {
            printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
            return;
         }

         printf("Socket %d connected\n", Accept);
      }


      // Try to read and write data to and from the data buffer if read and write events occur.

      if (NetworkEvents.lNetworkEvents & FD_READ ||
         NetworkEvents.lNetworkEvents & FD_WRITE)
      {
         if (NetworkEvents.lNetworkEvents & FD_READ &&
            NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
         {
            printf("FD_READ failed with error %d\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
            break;
         }

         if (NetworkEvents.lNetworkEvents & FD_WRITE && 
            NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
         {
            printf("FD_WRITE failed with error %d\n", NetworkEvents.iErrorCode[FD_WRITE_BIT]);
            break;
         }

         LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

         // Read data only if the receive buffer is empty.

         if (SocketInfo->BytesRECV == 0)
         {
            SocketInfo->DataBuf.buf = SocketInfo->Buffer;
            SocketInfo->DataBuf.len = DATA_BUFSIZE;

            Flags = 0;
            if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
               &Flags, NULL, NULL) == SOCKET_ERROR)
            {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  printf("WSARecv() failed with error %d\n", WSAGetLastError());
                  FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                  return;
               }
            } 
            else
            {
               SocketInfo->BytesRECV = RecvBytes;
            }
         }

         // Write buffer data if it is available.

         if (SocketInfo->BytesRECV > SocketInfo->BytesSEND)
         {
            SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
            SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

            if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
               NULL, NULL) == SOCKET_ERROR)
            {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  printf("WSASend() failed with error %d\n", WSAGetLastError());
                  FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                  return;
               }

               // A WSAEWOULDBLOCK error has occured. An FD_WRITE event will be posted
               // when more buffer space becomes available
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

      if (NetworkEvents.lNetworkEvents & FD_CLOSE)
      {
         if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
         {
            printf("FD_CLOSE failed with error %d\n", NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
            break;
         }

         printf("Closing socket information %d\n", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);

         FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
      }
   }
   return;
}


BOOL CreateSocketInformation(SOCKET s)
{
   LPSOCKET_INFORMATION SI;
      
   if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
   {
      printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
      return FALSE;
   }

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

   SocketArray[EventTotal] = SI;

   EventTotal++;

   return(TRUE);
}


void FreeSocketInformation(DWORD Event)
{
   LPSOCKET_INFORMATION SI = SocketArray[Event];
   DWORD i;

   closesocket(SI->Socket);

   GlobalFree(SI);

   WSACloseEvent(EventArray[Event]);

   // Squash the socket and event arrays

   for (i = Event; i < EventTotal; i++)
   {
      EventArray[i] = EventArray[i + 1];
      SocketArray[i] = SocketArray[i + 1];
   }

   EventTotal--;
}
