// Module: qostemplate.c
//
// Description:
//    This sample illustrates how to call the WSAGetQOSByName
//    first to enumerate the installed templates and then how
//    to retrieve the QOS template for a given template.
//    This module relies on the PrintQos() function to print
//    the template information.
//
// Compile:
//    cl qostemplate.c printqos.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    qostemplate.exe
//
#include <winsock2.h>
#include <windows.h>

#include <qos.h>
#include <qossp.h>

#include <stdio.h>
#include <stdlib.h>

#include "provider.h"
#include "printqos.h"

//
// Function: main
//
// Description:
//    Load Winsock, find the QOS enabled provider, and first
//    enumerate all the installed QOS templates. Next it
//    retrieves the QOS template.
//
int main(int argc, char **argv)
{
    WSAPROTOCOL_INFO *wsa=NULL;
    char              buf1[16000], 
                     *ptr;
    WSABUF            wbuf1,
                      wbuf2;
    SOCKET            s;
    WSADATA           wsdata;
    QOS               qos;

    // Load Winsock
    // 
    if (WSAStartup(MAKEWORD(2,2), &wsdata) != 0)
    {
        printf("Unable to load Winsock: %d\n", GetLastError());
        return -1;
    }
    // Find a provider
    //
    wsa = FindProtocolInfo(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                XP1_QOS_SUPPORTED);
    if (!wsa)
    {
        printf("unable to find a suitable provider!\n");
        return -1;
    }
    // Create a QOS socket
    //
    s = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
                wsa, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Enumerate all templates first
    //
    wbuf1.buf = buf1;
    wbuf1.len = 16000;
    if (WSAGetQOSByName(s, &wbuf1, NULL) == FALSE)
    {
        printf("WSAGetQOSByName() failed: %d\n", WSAGetLastError());
        return -1;
    }
    else
    {
        ptr = buf1;
        while (*ptr)
        {
            // Get a single template name and call the function to
            // retrieve QOS for that template
            //
	    printf("Template: %s\n", ptr);
            wbuf2.buf = ptr;
            wbuf2.len = strlen(ptr)+1; 
            if (WSAGetQOSByName(s, &wbuf2, &qos) == FALSE)
            {
                printf("WSAGetQOSByName() failed: %d\n", WSAGetLastError());
                return -1;
            }
            PrintQos(&qos);

	    while (*ptr++);
        }
    }

    closesocket(s);
    WSACleanup();
 
    return 0;
}
