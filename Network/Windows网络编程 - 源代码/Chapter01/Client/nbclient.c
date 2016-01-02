// Module Name: Nbclient.c
//
// Purpose:
//    This is a NetBIOS client application that can interact with
//    either of the two server samples.  The client attempts a
//    connection to the server on all LANA numbers. Once the first
//    connect succeeds, all others are cancelled or disconnected.
//
// Compile:
//    cl -o Nbclient.exe Nbclient.c ..\Common\Nbcommon.obj
//       netapi32.lib user32.lib
//
// Command Line Options:
//    Nbclient.exe CLIENT-NAME SERVER-NAME
//
//    CLIENT-NAME        The NetBIOS name this client registers as
//    SERVER-NAME        The NetBIOS name of the server to connect to
//
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\Common\nbcommon.h"

#define MAX_SESSIONS     254
#define MAX_NAMES        254

#define MAX_BUFFER       1024

char    szServerName[NCBNAMSZ];

//
// Function: Connect
//
// Description:
//    Post an asynchronous connect on the given LANA number to
//    the server. The NCB structure passed in already has the 
//    ncb_event field set to a valid Windows event handle. Just
//    fill in the blanks and make the call.
//
int Connect(PNCB pncb, int lana, char *server, char *client)
{
    pncb->ncb_command = NCBCALL | ASYNCH;
    pncb->ncb_lana_num = lana;

    memset(pncb->ncb_name, ' ', NCBNAMSZ);
    strncpy(pncb->ncb_name, client, strlen(client));

    memset(pncb->ncb_callname, ' ', NCBNAMSZ);
    strncpy(pncb->ncb_callname, server, strlen(server));

    if (Netbios(pncb) != NRC_GOODRET)
    {
        printf("ERROR: Netbios: NCBCONNECT: %d\n",
			pncb->ncb_retcode);
        return pncb->ncb_retcode;
    }
    return NRC_GOODRET;
}

//
// Function: main
//
// Description:
//    Initialize the NetBIOS interface, allocate some resources
//    (event handles, a send buffer, and so on), and issue an 
//    NCBCALL for each LANA to the given server. Once a connection
//    has been made, cancel or hang up any other outstanding
//    connections. Then send/receive the data. Finally, clean 
//    things up.
//
int main(int argc, char **argv)
{
    HANDLE      *hArray;
    NCB         *pncb;
    char         szSendBuff[MAX_BUFFER];
    DWORD        dwBufferLen,
                 dwRet,
                 dwIndex,
                 dwNum;
    LANA_ENUM    lenum;
    int          i;

    if (argc != 3)
    {
        printf("usage: nbclient CLIENT-NAME SERVER-NAME\n");
        return 1;
    }
    // Enumerate all LANAs and reset each one
    //
    if (LanaEnum(&lenum) != NRC_GOODRET)
        return 1;
    if (ResetAll(&lenum, (UCHAR)MAX_SESSIONS, (UCHAR)MAX_NAMES, 
            FALSE) != NRC_GOODRET)
        return 1;
    strcpy(szServerName, argv[2]);
    //
    // Allocate an array of HANDLEs to use for asynchronous events.
    // Also allocate an array of NCB structures.  We need 1 handle
	// and 1 NCB for each LANA number.
    //
    hArray = (HANDLE *)GlobalAlloc(GMEM_FIXED,
		sizeof(HANDLE) * lenum.length);
    pncb   = (NCB *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
		sizeof(NCB) * lenum.length);        
    //
    // Create an event and assign it into the corresponding NCB 
    // structure and issue an asynchronous connect (NCBCALL). 
    // Additionally don't forget to add the clients name to each
	// LANA it wants to connect over.
    //
    for(i = 0; i < lenum.length; i++)
    {
        hArray[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        pncb[i].ncb_event = hArray[i];

        AddName(lenum.lana[i], argv[1], &dwNum);
        Connect(&pncb[i], lenum.lana[i], szServerName, argv[1]);
    }
    // Wait for at least one connection to succeed
    //
    dwIndex = WaitForMultipleObjects(lenum.length, hArray, FALSE, 
        INFINITE);
    if (dwIndex == WAIT_FAILED)
    {
        printf("ERROR: WaitForMultipleObjects: %d\n",
			GetLastError());
    }
    else
    {
        // If more than one connection succeeds, hang up the extra 
        // connection. We'll use the connection that was returned
        // by WaitForMultipleObjects. Otherwise, if it's still pending,
        // cancel it.
        //
        for(i = 0; i < lenum.length; i++)
        {
            if (i != dwIndex)
            {
                if (pncb[i].ncb_cmd_cplt == NRC_PENDING)
                    Cancel(&pncb[i]);
                else
                    Hangup(pncb[i].ncb_lana_num, pncb[i].ncb_lsn);
            }
        }
        printf("Connected on LANA: %d\n", pncb[dwIndex].ncb_lana_num);
        //
        // Send and receive the messages
        //
        for(i = 0; i < 20; i++)
        {
            wsprintf(szSendBuff, "Test message %03d", i);
            dwRet = Send(pncb[dwIndex].ncb_lana_num, 
                pncb[dwIndex].ncb_lsn, szSendBuff,
				strlen(szSendBuff));
            if (dwRet != NRC_GOODRET)
                break;
            dwBufferLen = MAX_BUFFER;
            dwRet = Recv(pncb[dwIndex].ncb_lana_num, 
                pncb[dwIndex].ncb_lsn, szSendBuff, &dwBufferLen);
            if (dwRet != NRC_GOODRET)
                break;
            szSendBuff[dwBufferLen] = 0;
            printf("Read: '%s'\n", szSendBuff);
        }
        Hangup(pncb[dwIndex].ncb_lana_num, pncb[dwIndex].ncb_lsn);
    }
    // Clean things up
    //
    for(i = 0; i < lenum.length; i++)
    {
        DelName(lenum.lana[i], argv[1]);
        CloseHandle(hArray[i]);
    }
    GlobalFree(hArray);
    GlobalFree(pncb);

    return 0;
}
