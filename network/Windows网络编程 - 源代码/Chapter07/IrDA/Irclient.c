// Module Name: Irclient.c
//
// Description:
//    This sample illustrates how to create an IrSock client. This
//    sample is targeted for Windows 98 and NT 5 but can be used on
//    Windows CE. There are conditional defines to mark the major
//    differences in the platforms. Mainly, CE requires Winsock 1.1
//    while the others require 2.2.  Also, CE must be a Windows app
//    so you can't use main(). The only thing you need to do to for
//    CE is paste this file into a project file in VC to compile it
//    for the given target processor type.
//
// Compile:
//    CE: Paste into VC project and target for yur device. Link with 
//        winsock.lib
//    NT:
//        cl /D"_WIN32_WINNT" -o Irclient Irclient.c 
//		   Ircommon.obj ws2_32.lib
//    Windows 98:
//        cl /D"_WIN32_WINDOWS" -o Irclient Irclient.c 
//         Ircommon.obj ws2_32.lib
//
// Command line parameters/options:
//    None. The client is hardcode to attach to "MyServer". Change
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

#include "ircommon.h"

#define IR_SERVICE_NAME	"MyServer"
#define TEST_STRING		"This is a test of the client"
#define MAX_RETRIES     10
#define MAX_BUFFER      4096

// 
// Function: FindDevices
//
// Description:
//    This function attempts to locate any IR capable devices within
//    range.  This  is  done by calling the IRLMP_ENUMDEVICES socket 
//    option.  We  call  this several times in a loop to make a good
//    effort to find any.  Upon  success a  DEVICELIST  structure is
//    filled in with the device IDs of the discovered device.
//
int FindDevices(SOCKET s, DEVICELIST *devlist)
{
    DWORD        dwNumRetries=0,
                 dwDevListSz;
    int          ret;

    dwDevListSz = sizeof(*devlist);

    devlist->numDevice = 0; 
    while ((devlist->numDevice == 0) 
		&& (dwNumRetries <= MAX_RETRIES))
    {
        ret = getsockopt(s, SOL_IRLMP, IRLMP_ENUMDEVICES,
                           (char *)devlist, &dwDevListSz);
        if (ret == SOCKET_ERROR)
        {
			fprintf(stderr, "getsockopt(IRLMP_ENUMDEVICES) "
				"failed: %d\n",	WSAGetLastError());
            return 0;
        }
        dwNumRetries++;
        Sleep(1000);
    }
    if (dwNumRetries > MAX_RETRIES)
    {
	return 0;
    }
    return devlist->numDevice;
}

// 
// Function: main (WinMain)
//
// Description:
//    This is the main function for the client. The appropriate
//    Winsock library is loaded, an IR socket is created, and then
//    we enumerate any IR devices within range. We then attempt to
//    connect to each of these devices to a particular service. 
//    The first connection attemp that succeeds, we take.  Once
//    connected we send and receive data.
//
#ifdef _WIN32_WCE
int WINAPI WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                   LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
    WSADATA         wsd;
    SOCKET          sock;
    SOCKADDR_IRDA   irAddr = {AF_IRDA, 0, 0, 0, 0, "\0"};
    DWORD           dwIrSize = sizeof(SOCKADDR_IRDA),
                    dwErr;
    int             i, j, 
                    ret,
                    optval,
                    len;
    BOOL            bDone=FALSE;
    DEVICELIST      devlist;
    char            szRecvBuff[MAX_BUFFER];
    WORD            wVersion;

#ifdef _WIN32_WCE
    wVersion = MAEKWORD(1, 1);
#else
    wVersion = MAKEWORD(2, 2);
#endif

    if (WSAStartup(wVersion, &wsd) != 0)
    {
        printf("Unable to load the Winsock library!\n"); 
        return 1;
    }
    sock = socket(AF_IRDA, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("socket() failed: %d", WSAGetLastError());
        return 1;
    }
    if (FindDevices(sock, &devlist) == 0)
    {
        printf("No IrDA devices in range!\n");
        return 1;
    }
    // Setup the SOCKADDR_IRDA structure with the service 
	// name we want to connect to
    //
    strcpy(irAddr.irdaServiceName, IR_SERVICE_NAME);

    for(i = 0; i < devlist.numDevice; i++)
    {
        for(j = 0; j < 4; i++)
        {
            irAddr.irdaDeviceID[i] = 
				devlist.Device[i].irdaDeviceID[j];
        }
        if (connect(sock, (struct sockaddr *)&irAddr, 
                sizeof(SOCKADDR_IRDA)) == SOCKET_ERROR)
        {
            if (i == (devlist.numDevice - 1))
            {
                printf("Unable to locate service: '%s'\n", 
                    irAddr.irdaServiceName);
                return 1;
            }
            continue;
        }
        else
            break;
    }
    // Make the socket non-blocking
    // 
    optval = 1;
    if (ioctlsocket(sock, FIONBIO, &optval) == SOCKET_ERROR)
    {
        printf("ioctlsocket(FIONBIO) failed: %d\n", 
			WSAGetLastError());
        return 1;
    }
    // Read data from the server and echo it back until the server
    // closes the connection. I should probably use a select() call
	// to test for readability/writeability before calling 
	// send()/recv().
    //
    while (!bDone)
    {
        len = strlen(TEST_STRING);
        if ((ret = senddata(sock, TEST_STRING, &len)) != 0)
        {
            printf("send() failed: %d\n", ret);
            break;
        }
        if (len == 0)        // Graceful close
            break;

        len = MAX_BUFFER;
        if ((ret = recvdata(sock, szRecvBuff, &len)) != 0)
        {
            printf("recv() failed: %d\n", ret);
            break;
        }
        if (len == 0)        // Graceful close
            break;
    }

    closesocket(sock);
    WSACleanup();

    return 1;
}
