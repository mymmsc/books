// Module Name: Nbdgram.c
//
// Description:
//    This application demonstrates the various datagram operations
//    possible from NetBIOS.  This includes sending and receiving
//    both directed and group datagrams as well as broadcast datagrams.
//
// Compile:
//    cl -o Nbdgram.exe Nbdgram.c ..\Common\Nbcommon.obj
//      Netapi32.lib User32.lib
//
// Command Line Options:
//    nbdgram.exe options
//    -n:NAME    Register my unique name NAME
//    -g:NAME    Register my group name NAME
//    -s         I will be sending datagrams
//    -c:xxx     Number of datagrams to send
//    -r:NAME    The recipients NetBIOS name
//    -b         Use broadcast datagrams
//    -a         Receive datagrams for any NetBIOS name
//    -l:xxx     Send datagrams on LANA number xxx only
//    -d:xxx     Delay (in milliseconds) between sends
//    
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\Common\nbcommon.h"

#define MAX_SESSIONS            254
#define MAX_NAMES               254
#define MAX_DATAGRAM_SIZE       512

BOOL   bSender = FALSE,               // Send or receive datagrams
       bRecvAny = FALSE,              // Receive for any name
       bUniqueName = TRUE,            // Register my name as unique?
       bBroadcast = FALSE,            // Use broadcast datagrams?
       bOneLana = FALSE;              // Use all LANAs or just one?
char   szLocalName[NCBNAMSZ + 1],     // Local NetBIOS name
       szRecipientName[NCBNAMSZ + 1]; // Recipients NetBIOS name
DWORD  dwNumDatagrams = 25,           // Number of datagrams to sen
       dwOneLana,                     // If using one LANA, which one
       dwDelay = 0;                   // Delay between datagram send

//
// Function: ValidateArgs
//
// Description:
//    This function parses the command line arguments
//    and sets various global flags indicating the selections.
//
void ValidateArgs(int argc, char **argv)
{
    int                i;

    for(i = 1; i < argc; i++)
    {
        if (strlen(argv[i]) < 2)
            continue;
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'n':        // Use a unique name
                    bUniqueName = TRUE;
                    if (strlen(argv[i]) > 2)
                        strcpy(szLocalName, &argv[i][3]);
                    break;
                case 'g':        // Use a group name
                    bUniqueName = FALSE;
                    if (strlen(argv[i]) > 2)
                        strcpy(szLocalName, &argv[i][3]);
                    break;
                case 's':        // Send datagrams
                    bSender = TRUE;
                    break;
                case 'c':        // Num datagrams to send or receive
                    if (strlen(argv[i]) > 2)
                        dwNumDatagrams = atoi(&argv[i][3]);
                    break;
                case 'r':        // Recipient's name for datagrams
                    if (strlen(argv[i]) > 2)
                        strcpy(szRecipientName, &argv[i][3]);
                    break;
                case 'b':        // Use broadcast datagrams
                    bBroadcast = TRUE;
                    break;
                case 'a':        // Receive datagrams on any name
                    bRecvAny = TRUE;
                    break;
                case 'l':        // Operate on this LANA only
                    bOneLana = TRUE;
                    if (strlen(argv[i]) > 2)
                        dwOneLana = atoi(&argv[i][3]);
                    break;
                case 'd':        // Delay (millisecs) between sends
                    if (strlen(argv[i]) > 2)
                        dwDelay = atoi(&argv[i][3]);
                    break;
                default:
                    printf("usage: nbdgram ?\n");
                    break;
             }
        }
    }
    return;
}

//
// Function: DatagramSend
//
// Description:
//    Send a directed datagram to the specified recipient on the 
//    specified LANA number from the given name number to the
//    specified recipient. Also specified is the data buffer and 
//    the number of bytes to send.
//
int DatagramSend(int lana, int num, char *recipient, 
				 char *buffer, int buflen)
{
    NCB                ncb;

    ZeroMemory(&ncb, sizeof(NCB));
    ncb.ncb_command = NCBDGSEND;
    ncb.ncb_lana_num = lana;
    ncb.ncb_num = num;
    ncb.ncb_buffer = (PUCHAR)buffer;
    ncb.ncb_length = buflen;

    memset(ncb.ncb_callname, ' ', NCBNAMSZ);
    strncpy(ncb.ncb_callname, recipient, strlen(recipient));

    if (Netbios(&ncb) != NRC_GOODRET)
    {
        printf("Netbios: NCBDGSEND failed: %d\n", ncb.ncb_retcode);
        return ncb.ncb_retcode;
    }
    return NRC_GOODRET;
}

//
// Function: DatagramSendBC
//
// Description:
//    Send a broadcast datagram on the specified LANA number from the 
//    given name number.  Also specified is the data buffer and number
//    of bytes to send.
//
int DatagramSendBC(int lana, int num, char *buffer, int buflen)
{
    NCB                ncb;

    ZeroMemory(&ncb, sizeof(NCB));
    ncb.ncb_command = NCBDGSENDBC;
    ncb.ncb_lana_num = lana;
    ncb.ncb_num = num;
    ncb.ncb_buffer = (PUCHAR)buffer;
    ncb.ncb_length = buflen;

    if (Netbios(&ncb) != NRC_GOODRET)
    {
        printf("Netbios: NCBDGSENDBC failed: %d\n", ncb.ncb_retcode);
        return ncb.ncb_retcode;
    }
    return NRC_GOODRET;
}

//
// Function: DatagramRecv
//
// Description:
//    Receive a datagram on the given LANA number directed towards the
//    name represented by num.  Data is copied into the supplied buffer.
//    If hEvent is not zero then the receive call is made asynchronously
//    with the supplied event handle. If num is 0xFF then listen for a
//    datagram destined for any NetBIOS name registered by the process.
//
int DatagramRecv(PNCB pncb, int lana, int num, char *buffer, 
                 int buflen, HANDLE hEvent)
{
    ZeroMemory(pncb, sizeof(NCB));
    if (hEvent)
    {
        pncb->ncb_command = NCBDGRECV | ASYNCH;
        pncb->ncb_event = hEvent;
    }
    else
        pncb->ncb_command = NCBDGRECV;
    pncb->ncb_lana_num = lana;
    pncb->ncb_num = num;
    pncb->ncb_buffer = (PUCHAR)buffer;
    pncb->ncb_length = buflen;

    if (Netbios(pncb) != NRC_GOODRET)
    {
        printf("Netbos: NCBDGRECV failed: %d\n", pncb->ncb_retcode);
        return pncb->ncb_retcode;
    }
    return NRC_GOODRET;
}

//
// Function: DatagramRecvBC
//
// Description:
//    Receive a broadcast datagram on the given LANA number.
//    Data is copied into the supplied buffer.  If hEvent is not zero 
//    then the receive call is made asynchronously with the supplied 
//    event handle.
//
int DatagramRecvBC(PNCB pncb, int lana, int num, char *buffer, 
                   int buflen, HANDLE hEvent)
{
    ZeroMemory(pncb, sizeof(NCB));
    if (hEvent)
    {
        pncb->ncb_command = NCBDGRECVBC | ASYNCH;
        pncb->ncb_event = hEvent;
    }
    else
        pncb->ncb_command = NCBDGRECVBC;
    pncb->ncb_lana_num = lana;
    pncb->ncb_num = num;
    pncb->ncb_buffer = (PUCHAR)buffer;
    pncb->ncb_length = buflen;
  
    if (Netbios(pncb) != NRC_GOODRET)   
    {
        printf("Netbios: NCBDGRECVBC failed: %d\n", pncb->ncb_retcode);
        return pncb->ncb_retcode;
    }
    return NRC_GOODRET;
}

//
// Function: main
//
// Description:
//    Initialize the NetBIOS interface, allocate resources, and then
//    send or receive datagrams according to the user's options.
//
int main(int argc, char **argv)
{
    LANA_ENUM   lenum;
    int         i, j;
    char        szMessage[MAX_DATAGRAM_SIZE],
                szSender[NCBNAMSZ + 1];
    DWORD      *dwNum = NULL,
                dwBytesRead,
                dwErr;

    ValidateArgs(argc, argv);
    //
    // Enumerate and reset the LANA numbers
    //
    if ((dwErr = LanaEnum(&lenum)) != NRC_GOODRET)
    {
        printf("LanaEnum failed: %d\n", dwErr);
        return 1;
    }
    if ((dwErr = ResetAll(&lenum, (UCHAR)MAX_SESSIONS,
		(UCHAR)MAX_NAMES, FALSE)) != NRC_GOODRET)
    {
        printf("ResetAll failed: %d\n", dwErr);
        return 1;
    }
    //
    // This buffer holds the name number for the NetBIOS name added
    // to each LANA
    //
    dwNum = (DWORD *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
            sizeof(DWORD) * lenum.length);
    if (dwNum == NULL)
    {
        printf("out of memory\n");
        return 1;
    }
    //
    // If we're going to operate only on one LANA register the name
    // only on that specified LANA; otherwise register it on all.
    // 
    if (bOneLana)
    {
        if (bUniqueName)
            AddName(dwOneLana, szLocalName, &dwNum[0]);
        else
            AddGroupName(dwOneLana, szLocalName, &dwNum[0]);
    }
    else
    {
        for(i = 0; i < lenum.length; i++)
        {
            if (bUniqueName)
                AddName(lenum.lana[i], szLocalName, &dwNum[i]);
            else
                AddGroupName(lenum.lana[i], szLocalName, &dwNum[i]);
        }
    }
    // We are sending datagrams
    //
    if (bSender)
    {
        // Broadcast sender
        //
        if (bBroadcast)
        {
            if (bOneLana)
            {
                // Broadcast the message on the one LANA only
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    wsprintf(szMessage,
						"[%03d] Test broadcast datagram", j);
                    if (DatagramSendBC(dwOneLana, dwNum[0],
						szMessage, strlen(szMessage))
						!= NRC_GOODRET)
                        return 1;
                    Sleep(dwDelay);
                }
            }
            else
            {
                // Broadcast the message on every LANA on the local 
                // machine
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    for(i = 0; i < lenum.length; i++)
                    {
                        wsprintf(szMessage,
							"[%03d] Test broadcast datagram", j);
                        if (DatagramSendBC(lenum.lana[i], dwNum[i],
							szMessage, strlen(szMessage)) 
							!= NRC_GOODRET)
                            return 1;
                    }
                    Sleep(dwDelay);
                }
            }
        }
        else
        {
            if (bOneLana)
            {
                // Send a directed message to the one LANA specified
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    wsprintf(szMessage,
						"[%03d] Test directed datagram", j);
                    if (DatagramSend(dwOneLana, dwNum[0],
						szRecipientName, szMessage,
						strlen(szMessage)) != NRC_GOODRET)
                        return 1;
                    Sleep(dwDelay);
                }
            }
            else
            {
                // Send a directed message to the each LANA on the 
                // local machine
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    for(i = 0; i < lenum.length; i++)
                    {
                        wsprintf(szMessage,
							"[%03d] Test directed datagram", j);
						printf("count: %d.%d\n", j,i);
                        if (DatagramSend(lenum.lana[i], dwNum[i], 
                            szRecipientName, szMessage, 
                            strlen(szMessage)) != NRC_GOODRET)
                            return 1;
                    }
                    Sleep(dwDelay);
                }
            }
        }
    }
    else                // We are receiving datagrams
    {
        NCB     *ncb=NULL;
        char    **szMessageArray = NULL;
        HANDLE  *hEvent=NULL;
        DWORD   dwRet;

        // Allocate an array of NCB structure to submit to each recv 
        // on each LANA
        //
        ncb = (NCB *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                    sizeof(NCB) * lenum.length);
        //
        // Allocate an array of incoming data buffers
        //
        szMessageArray = (char **)GlobalAlloc(GMEM_FIXED, 
                sizeof(char *) * lenum.length);
        for(i = 0; i < lenum.length; i++)
            szMessageArray[i] = (char *)GlobalAlloc(GMEM_FIXED, 
                    MAX_DATAGRAM_SIZE);
        //
        // Allocate an array of event handles for 
		// asynchronous receives
        //
        hEvent = (HANDLE *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, 
                sizeof(HANDLE) * lenum.length);
        for(i = 0; i < lenum.length; i++)
            hEvent[i] = CreateEvent(0, TRUE, FALSE, 0);

        if (bBroadcast)
        {
            if (bOneLana)
            {
                // Post synchronous broadcast receives on 
				// the one LANA specified
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    if (DatagramRecvBC(&ncb[0], dwOneLana, dwNum[0], 
                        szMessageArray[0], MAX_DATAGRAM_SIZE,  
                        NULL) != NRC_GOODRET)
                        return 1;
                    FormatNetbiosName(ncb[0].ncb_callname, szSender);
                    printf("%03d [LANA %d] Message: '%s' "
						"received from: %s\n", j,
						ncb[0].ncb_lana_num, szMessageArray[0],
                        szSender);
                }
            }
            else
            {
                // Post asynchronous broadcast receives on each LANA 
                // number available. For each command that succeeded 
                // print the message otherwise cancel the command.
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    for(i = 0; i < lenum.length; i++)
                    {
                        dwBytesRead = MAX_DATAGRAM_SIZE;
                        if (DatagramRecvBC(&ncb[i], lenum.lana[i],
							dwNum[i], szMessageArray[i],
							MAX_DATAGRAM_SIZE, hEvent[i])
							!= NRC_GOODRET)
                            return 1;
                    }
                    dwRet = WaitForMultipleObjects(lenum.length, 
                        hEvent, FALSE, INFINITE);
                    if (dwRet == WAIT_FAILED)
                    {
                        printf("WaitForMultipleObjects failed: %d\n",
                            GetLastError());
                        return 1;
                    }
                    for(i = 0; i < lenum.length; i++)
                    {
                        if (ncb[i].ncb_cmd_cplt == NRC_PENDING)
                            Cancel(&ncb[i]);
                        else
                        {
                            ncb[i].ncb_buffer[ncb[i].ncb_length] = 0;
                            FormatNetbiosName(ncb[i].ncb_callname, 
                                szSender);
                            printf("%03d [LANA %d] Message: '%s' "
								"received from: %s\n", j,
								ncb[i].ncb_lana_num, 
                                szMessageArray[i], szSender);
                        }
                        ResetEvent(hEvent[i]);
                    }
                }
            }
        }
        else
        {
            if (bOneLana)
            {
                // Make a blocking datagram receive on the specified
                // LANA number.
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    if (bRecvAny)
                    {
                        // Receive data destined for any NetBIOS name
                        // in this process's name table.
                        //
                        if (DatagramRecv(&ncb[0], dwOneLana, 0xFF, 
                            szMessageArray[0], MAX_DATAGRAM_SIZE,
                            NULL) != NRC_GOODRET)
                            return 1;
                    }
                    else
                    {
                        if (DatagramRecv(&ncb[0], dwOneLana,
							dwNum[0], szMessageArray[0],
							MAX_DATAGRAM_SIZE, NULL) 
							!= NRC_GOODRET)
                            return 1;
                    }
                    FormatNetbiosName(ncb[0].ncb_callname, szSender);
                    printf("%03d [LANA %d] Message: '%s' "
						   "received from: %s\n", j,
						   ncb[0].ncb_lana_num, szMessageArray[0],
						   szSender);
                }
            }
            else
            {
                // Post asynchronous datagram receives on each LANA
                // available. For all those commands that succeeded,
                // print the data, otherwise cancel the command.
                //
                for(j = 0; j < dwNumDatagrams; j++)
                {
                    for(i = 0; i < lenum.length; i++)
                    {
                        if (bRecvAny)
                        {
                            // Receive data destined for any NetBIOS 
                            // name in this process's name table.
                            // 
                            if (DatagramRecv(&ncb[i], lenum.lana[i],
								0xFF, szMessageArray[i],
								MAX_DATAGRAM_SIZE, hEvent[i])
								!= NRC_GOODRET)
                                return 1;
                        }
                        else
                        {
                            if (DatagramRecv(&ncb[i], lenum.lana[i],
								dwNum[i], szMessageArray[i],
								MAX_DATAGRAM_SIZE, hEvent[i])
								!= NRC_GOODRET)
                                return 1;
                        }
                    }
                    dwRet = WaitForMultipleObjects(lenum.length, 
                        hEvent, FALSE, INFINITE);
                    if (dwRet == WAIT_FAILED)
                    {
                        printf("WaitForMultipleObjects failed: %d\n",
                            GetLastError());
                        return 1;
                    }
                    for(i = 0; i < lenum.length; i++)
                    {
                        if (ncb[i].ncb_cmd_cplt == NRC_PENDING)
                            Cancel(&ncb[i]);
                        else
                        {
                            ncb[i].ncb_buffer[ncb[i].ncb_length] = 0;
                            FormatNetbiosName(ncb[i].ncb_callname, 
                                szSender);
                            printf("%03d [LANA %d] Message: '%s' "
								"from: %s\n", j, ncb[i].ncb_lana_num,
                                szMessageArray[i], szSender);
                        }
                        ResetEvent(hEvent[i]);
                    }
                }
            }
        }
        // Clean up
        //
        for(i = 0; i < lenum.length; i++)
        {
            CloseHandle(hEvent[i]);
            GlobalFree(szMessageArray[i]);
        }
        GlobalFree(hEvent);
        GlobalFree(szMessageArray);
    }
    // Clean things up
    //
    if (bOneLana)
        DelName(dwOneLana, szLocalName);
    else
    {
        for(i = 0; i < lenum.length; i++)
            DelName(lenum.lana[i], szLocalName);
    }
    GlobalFree(dwNum);

    return 0;
}
