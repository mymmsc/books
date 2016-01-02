// Module Name: Irserver.c
//
// Description:
//    This sample illustrates a IrSock server. This sampe is targeted
//    towards Windows 98 and Windows 2000 but can also be used on 
//    Windows CE. There are conditional defines to make the major 
//    differences in the platforms. Mainly, Windows CE requires 
//    Winsock 1.1 while the others require 2.2.  Also, Windows CE 
//    must be a Windows application so you can't use main(). The only
//    thing you need to do to for Windows CE is paste this file into 
//    a project file in VC to compile it for the given target 
//    processor type.
//
// Compile:
//    Windows CE: Paste into VC project and target for yur device.
//                Link with winsock.lib
//    Windows NT:
//        cl /D"_WIN32_WINNT" -o Irserver Irserver.c 
//         Ircommon.obj ws2_32.lib
//    Windows 98:
//        cl /D"_WIN32_WINDOWS" -o Irserver Irserver.c 
//         Ircommon.obj ws2_32.lib
//
// Command line parameters/options:
//    None. The server is hardcode to listen on "MyServer". Change
//    the define IR_SERVICE_NAME if you desire something else (don't
//    forget to change the server too).
//

#ifdef _WIN32_WCE
#include <windows.h>
#include <winsock.h>
#else
#include <winsock2.h>
#endif

#include "af_irda.h"

#include <stdio.h>
#include <stdlib.h>

#define IR_SERVICE_NAME	  "MyServer"
#define MAX_BUFFER        4096

//
// Function: ClientThread
//
// Description:
//    This is a client thread that is spawned with each client
//    connection.  The thread simply reads data and writes it
//    back to the client until the socket is closed.
//
DWORD WINAPI ClientThread(LPVOID lpParam)
{
    SOCKET        s = (SOCKET)lpParam;
    int           ret,
                  len;
    char          szRecvBuff[MAX_BUFFER];

    while (1)
    {
        // Read data from client
        //
        len = MAX_BUFFER;
        if ((ret = recvdata(s, szRecvBuff, &len)) != 0)
        {
            printf("recv() failed: %d\n", ret);
            break;
        }
        if (len == 0)        // Graceful close
            break;
        szRecvBuff[len] = 0;
        printf("Read: %d bytes\n", len);
        // 
        // Write data back until socket closure, len will
        //  equal 0 when the socket has been gracefully closed
        // 
        if ((ret = senddata(s, szRecvBuff, &len)) != 0)
        {
            printf("send() failed: %d\n", ret);
            break;
        }
        if (len == 0)        // Graceful close
            break;
        printf("Wrote: %d bytes\n", len);
    }
    closesocket(s);
    return 0;
}

//
// Function: main (Winmain)
//
// Description:
//    This is the main function of the server. The appropriate 
//    Winsock library is loaded first, an IR listening socket is 
//    created, and we bind the socket to our server name.
//    Afterwards we block on an accept() in a loop. Once a client
//    connection is made, we spawn a thread to handle the connection.
//
#ifdef _WIN32_WCE
int WINAPI WinMain(HANDLE hInstance, HANDLE hPrevIntance,
                   LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
    WSADATA			wsd;
    SOCKET			sock,
					sockClient;
    SOCKADDR_IRDA	irAddr = { AF_IRDA, 0, 0, 0, 0, "\0" },
					remoteIrAddr;
    DWORD			dwIrSize = sizeof(SOCKADDR_IRDA),
					dwRet, 
					dwErr,
					dwId;
    BOOL			bDone = FALSE;
    char			szRecvBuff[MAX_BUFFER];
    int				optval;
    HANDLE			hThread;
    WORD			wVersion;

#ifdef _WIN32_WCE
    wVersion = MAKEWORD(1, 1);
#else
    wVersion = MAKEWORD(2, 2);
#endif

    if (WSAStartup(wVersion, &wsd) != 0)
    {
        fprintf(stderr, "Unable to load Winsock library!\n");
		return 0;
    }
    sock = socket(AF_IRDA, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
		fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
		return 0;
    }

    strcpy(irAddr.irdaServiceName, IR_SERVICE_NAME);
    //
    // Bind our socket to the local service name
    //
    printf("Binding to service class name: %s\n", 
		irAddr.irdaServiceName);
    if (bind(sock, (struct sockaddr *)&irAddr, sizeof(SOCKADDR_IRDA))
		== SOCKET_ERROR)
    {
		fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
		return 0;
    }
 
    listen(sock, 10);

    while (1)
    {
        sockClient = accept(sock, (struct sockaddr *)&remoteIrAddr,
			&dwIrSize);
        if (sockClient == SOCKET_ERROR)
        {
			fprintf(stderr, "accept() failed: %d\n",
				WSAGetLastError());
			return 0;
        } 
        // Make the client socket non-blocking
        //
        optval = 1;
        if (ioctlsocket(sockClient, FIONBIO, &optval) 
			== SOCKET_ERROR)
        {
			fprintf(stderr, "ioctlsocket(FIONBIO) failed: %d\n",
				WSAGetLastError());
            return 0;
        }
        hThread = CreateThread(NULL, 0, ClientThread, 
			(LPVOID)sockClient, 0, &dwId);
        if (hThread == NULL)
        {
			// Unable to create thread
        }
        CloseHandle(hThread);
    }
    // Close and cleanup
    //
    closesocket(sockClient);
    closesocket(sock);

    WSACleanup();

    return 1;
}
