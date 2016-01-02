// Module Name: Evnbsvr.c
//
// Description:
//    This sample illustrates a NetBIOS server that uses asynchronous
//    events. The server adds its name to each LANA number and posts
//    an asynch listen on each LANA which will trigger an event when
//    a client connection request is made.  The client will be 
//    serviced.
//  
// Compile:
//    cl -o Evnbsvr.exe Evnbsvr.c ..\Common\Nbcommon.obj Netapi32.lib
//
// Command Line Options:
//    NONE - The server's NetBIOS name is hardcoded as the define
//           SERVER_NAME.  Change this value if you wish the server
//           to register itself under a different name.
//
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\Common\nbcommon.h"

#define MAX_SESSIONS    254
#define MAX_NAMES       254

#define MAX_BUFFER      2048
#define SERVER_NAME     "TEST-SERVER-1"

NCB    *g_Clients=NULL;        // Global NCB structure for clients

//
// Function: ClientThread
//
// Description:
//    This thread takes the NCB structure of a connected session
//    and waits for incoming data which it then sends back to the
//    client until the session is closed.
//
DWORD WINAPI ClientThread(PVOID lpParam)
{
    PNCB        pncb = (PNCB)lpParam;
    NCB         ncb;
    char        szRecvBuff[MAX_BUFFER],
                szClientName[NCBNAMSZ + 1];
    DWORD       dwBufferLen = MAX_BUFFER,
                dwRetVal = NRC_GOODRET;

    // Send and receive messages until the session is closed
    // 
    FormatNetbiosName(pncb->ncb_callname, szClientName);
    while (1)
    {
        dwBufferLen = MAX_BUFFER;
        dwRetVal = Recv(pncb->ncb_lana_num, pncb->ncb_lsn,
			szRecvBuff, &dwBufferLen);
        if (dwRetVal != NRC_GOODRET)
            break;

        szRecvBuff[dwBufferLen] = 0;
        printf("READ [LANA=%d]: '%s'\n", pncb->ncb_lana_num, 
            szRecvBuff);

        dwRetVal = Send(pncb->ncb_lana_num, pncb->ncb_lsn,
			szRecvBuff, dwBufferLen);
        if (dwRetVal != NRC_GOODRET)
            break;
    }
    printf("Client '%s' on LANA %d disconnected\n", szClientName,
        pncb->ncb_lana_num);
    //
    // If the error returned from a read or write is NRC_SCLOSED then
    // all is well; otherwise some other error occured so hangup the
    // connection from this side.
    //
    if (dwRetVal != NRC_SCLOSED)
    {
        ZeroMemory(&ncb, sizeof(NCB));
        ncb.ncb_command = NCBHANGUP;
        ncb.ncb_lsn = pncb->ncb_lsn;
        ncb.ncb_lana_num = pncb->ncb_lana_num;

        if (Netbios(&ncb) != NRC_GOODRET)
        {
            printf("ERROR: Netbios: NCBHANGUP: %d\n",
				ncb.ncb_retcode);
            GlobalFree(pncb);
            dwRetVal = ncb.ncb_retcode;
        }
    }
    // The NCB structure passed in is dynamically allocated it so 
    // delete it before we go.
    //
    GlobalFree(pncb);
    return NRC_GOODRET; 
}

//
// Function: Listen
//
// Description:
//    Post an asynchronous listen on the given LANA number.
//    The NCB structure passed in already has its ncb_event
//    field set to a valid Windows event handle.
//
int Listen(PNCB pncb, int lana, char *name)
{
    pncb->ncb_command = NCBLISTEN | ASYNCH;
    pncb->ncb_lana_num = lana;
    //
    // This is the name clients will connect to
    //
    memset(pncb->ncb_name, ' ', NCBNAMSZ);
    strncpy(pncb->ncb_name, name, strlen(name));
    //
    // An '*' means we'll accept connections from anyone.
    // You could specify a specific name which means only a
    // client with the specified name will be allowed to connect.
    //
    memset(pncb->ncb_callname, ' ', NCBNAMSZ);
    pncb->ncb_callname[0] = '*';
  
    if (Netbios(pncb) != NRC_GOODRET)
    {
        printf("ERROR: Netbios: NCBLISTEN: %d\n", pncb->ncb_retcode);
        return pncb->ncb_retcode;
    }
    return NRC_GOODRET;
}

//
// Function: main
//
// Description:
//    Initialize the NetBIOS interface, allocate some resources, and
//    post asynchronous listens on each LANA using events. Wait for
//    an event to be triggered, and then handle the client 
//    connection.
//
int main(int argc, char **argv)
{
    PNCB        pncb=NULL;
    HANDLE      hArray[64],
                hThread;
    DWORD       dwHandleCount=0,
                dwRet,
                dwThreadId;
    int         i,
                num;
    LANA_ENUM   lenum;

    // Enumerate all LANAs and reset each one
    //
    if (LanaEnum(&lenum) != NRC_GOODRET)
        return 1;
    if (ResetAll(&lenum, (UCHAR)MAX_SESSIONS, (UCHAR)MAX_NAMES,
            FALSE) != NRC_GOODRET)
        return 1;
    //
    // Allocate an array of NCB structures (one for each LANA)
    // 
    g_Clients = (PNCB)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
            sizeof(NCB) * lenum.length);
    //
    // Create the events, add the server name to each LANA, and issue
    // the asynchronous listens on each LANA.
    //
    for(i = 0; i < lenum.length; i++)
    {
        hArray[i] = g_Clients[i].ncb_event = CreateEvent(NULL, TRUE, 
            FALSE, NULL);

        AddName(lenum.lana[i], SERVER_NAME, &num);
        Listen(&g_Clients[i], lenum.lana[i], SERVER_NAME);
    }
    while (1)
    {        
        // Wait until a client connects
        //
        dwRet = WaitForMultipleObjects(lenum.length, hArray, FALSE, 
            INFINITE);
        if (dwRet == WAIT_FAILED)
        {
            printf("ERROR: WaitForMultipleObjects: %d\n", 
                GetLastError());
            break;
        }
        // Go through all the NCB structures to see if more that one
        // succeeded.  If ncb_cmd_plt is not NRC_PENDING then there
		// is a client, create a thread and hand off a new NCB
		// structure to the thread.  We need to re-use the original
		// NCB for other client connections.
        //
        for(i = 0; i < lenum.length; i++)
        {
            if (g_Clients[i].ncb_cmd_cplt != NRC_PENDING) 
            {
                pncb = (PNCB)GlobalAlloc(GMEM_FIXED, sizeof(NCB));
                memcpy(pncb, &g_Clients[i], sizeof(NCB));
                pncb->ncb_event = 0;

                hThread = CreateThread(NULL, 0, ClientThread, 
                    (LPVOID)pncb, 0, &dwThreadId);
                CloseHandle(hThread);
                //
                // Reset the handle and post another listen 
                //
                ResetEvent(hArray[i]);
                Listen(&g_Clients[i], lenum.lana[i], SERVER_NAME);
            }
        }
    }
    // Clean up
    //
    for(i = 0; i < lenum.length; i++)
    {
        DelName(lenum.lana[i], SERVER_NAME);
        CloseHandle(hArray[i]);
    }
    GlobalFree(g_Clients);

    return 0;
}