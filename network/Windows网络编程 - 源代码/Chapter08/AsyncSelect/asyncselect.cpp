// Module Name: asyncselect.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the WSAAsyncSelect() I/O model. This sample is
//    implemented as a console-style application (to reduce the programming
//    complexity of writing a real Windows application) and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts them
//    as they arrive. When this application receives data from a client, it
//    simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
//    Since the WSAAsyncSelect I/O model requires an application to manage
//    window messages when network event occur, this application creates
//    a window for the I/O model only. The window stays hidden during the
//    entire execution of this application.
//
// Compile:
//
//    cl -o asyncselect asyncselect.cpp ws2_32.lib user32.lib gdi32.lib
//
// Command Line Options:
//
//    asyncselect.exe 
//
//    Note: There are no command line options for this sample.

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#define PORT 5150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
   BOOL RecvPosted;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   SOCKET Socket;
   DWORD BytesSEND;
   DWORD BytesRECV;
   _SOCKET_INFORMATION *Next;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

#define WM_SOCKET (WM_USER + 1)

void CreateSocketInformation(SOCKET s);
LPSOCKET_INFORMATION GetSocketInformation(SOCKET s);
void FreeSocketInformation(SOCKET s);

HWND MakeWorkerWindow(void);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LPSOCKET_INFORMATION SocketInfoList;

void main(void)
{
   MSG msg;
   DWORD Ret;
   SOCKET Listen;
   SOCKADDR_IN InternetAddr;
   HWND Window;
   WSADATA wsaData;

   if ((Window = MakeWorkerWindow()) == NULL)
      return;

   // Prepare echo server

   if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
   {
      printf("WSAStartup failed with error %d\n", Ret);
      return;
   }

   if ((Listen = socket (PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
   {
      printf("socket() failed with error %d\n", WSAGetLastError());
      return;
   } 

   WSAAsyncSelect(Listen, Window, WM_SOCKET, FD_ACCEPT|FD_CLOSE);

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
     		
   // Translate and dispatch window messages for the application thread

   while(Ret = GetMessage(&msg, NULL, 0, 0))
   {
      if (Ret == -1)
      {
         printf("GetMessage() failed with error %d\n", GetLastError());
         return;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   SOCKET Accept;
   LPSOCKET_INFORMATION SocketInfo;
   DWORD RecvBytes, SendBytes;
   DWORD Flags;

   if (uMsg == WM_SOCKET)
   {
      if (WSAGETSELECTERROR(lParam))
      {
         printf("Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
         FreeSocketInformation(wParam);
      } 
      else
      {
         switch(WSAGETSELECTEVENT(lParam))
         {
            case FD_ACCEPT:

               if ((Accept = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
               {		
                  printf("accept() failed with error %d\n", WSAGetLastError());
                  break;
               }

               // Create a socket information structure to associate with the
               // socket for processing I/O.

               CreateSocketInformation(Accept);

               printf("Socket number %d connected\n", Accept);

               WSAAsyncSelect(Accept, hwnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE);

               break;

            case FD_READ:

               SocketInfo = GetSocketInformation(wParam);

               // Read data only if the receive buffer is empty.

               if (SocketInfo->BytesRECV != 0)
               {
                  SocketInfo->RecvPosted = TRUE;
                  return 0;
               }
               else
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
                        FreeSocketInformation(wParam);
                        return 0;
                     }
                  } 
                  else // No error so update the byte count
                  {
                     SocketInfo->BytesRECV = RecvBytes;
                  }
               }

               // DO NOT BREAK HERE SINCE WE GOT A SUCCESSFUL RECV. Go ahead
               // and begin writing data to the client.

            case FD_WRITE:   			

               SocketInfo = GetSocketInformation(wParam);

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
                        FreeSocketInformation(wParam);
                        return 0;
                     }
                  } 
                  else // No error so update the byte count
                  {
                     SocketInfo->BytesSEND += SendBytes;
                  }
               }

               if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
               {
                  SocketInfo->BytesSEND = 0;
                  SocketInfo->BytesRECV = 0;

                  // If a RECV occurred during our SENDs then we need to post an FD_READ
                  // notification on the socket.

                  if (SocketInfo->RecvPosted == TRUE)
                  {
                     SocketInfo->RecvPosted = FALSE;
                     PostMessage(hwnd, WM_SOCKET, wParam, FD_READ);
                  }
               }

               break;

            case FD_CLOSE:

               printf("Closing socket %d\n", wParam);
               FreeSocketInformation(wParam);

               break;
         }
      }
      return 0;
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void CreateSocketInformation(SOCKET s)
{
   LPSOCKET_INFORMATION SI;

   if ((SI = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
      sizeof(SOCKET_INFORMATION))) == NULL)
   {
      printf("GlobalAlloc() failed with error %d\n", GetLastError());
      return;
   }

   // Prepare SocketInfo structure for use.

   SI->Socket = s;
   SI->RecvPosted = FALSE;
   SI->BytesSEND = 0;
   SI->BytesRECV = 0;

   SI->Next = SocketInfoList;

   SocketInfoList = SI;
}

LPSOCKET_INFORMATION GetSocketInformation(SOCKET s)
{
   SOCKET_INFORMATION *SI = SocketInfoList;

   while(SI)
   {
      if (SI->Socket == s)
         return SI;

      SI = SI->Next;
   }

   return NULL;
}

void FreeSocketInformation(SOCKET s)
{
   SOCKET_INFORMATION *SI = SocketInfoList;
   SOCKET_INFORMATION *PrevSI = NULL;

   while(SI)
   {
      if (SI->Socket == s)
      {
         if (PrevSI)
            PrevSI->Next = SI->Next;
         else
            SocketInfoList = SI->Next;

         closesocket(SI->Socket);
         GlobalFree(SI);
         return;
      }

      PrevSI = SI;
      SI = SI->Next;
   }
}

HWND MakeWorkerWindow(void)
{
   WNDCLASS wndclass;
   CHAR *ProviderClass = "AsyncSelect";
   HWND Window;

   wndclass.style = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc = (WNDPROC)WindowProc;
   wndclass.cbClsExtra = 0;
   wndclass.cbWndExtra = 0;
   wndclass.hInstance = NULL;
   wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
   wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
   wndclass.lpszMenuName = NULL;
   wndclass.lpszClassName = ProviderClass;

   if (RegisterClass(&wndclass) == 0)
   {
      printf("RegisterClass() failed with error %d\n", GetLastError());
      return NULL;
   }

   // Create a window.

   if ((Window = CreateWindow(
      ProviderClass,
      "",
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      NULL,
      NULL,
      NULL,
      NULL)) == NULL)
   {
      printf("CreateWindow() failed with error %d\n", GetLastError());
      return NULL;
   }

   return Window;
}
