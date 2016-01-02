// Module: qosudp.c
//
// Description:
//    This is a description.
//
// Compile:
//    cl -o qosudp.exe qosudp.c printqos.c provider.c ws2_32.lib 
//
// Command Line Parameters/Options
//    qosudp.exe -q:flag -s -c:Server-IP -w
//       -q:[b,d,a,e]        When to request QOS
//           b               Set QOS before bind or connect
//           d               Set QOS during accept cond func
//           a               Set QOS after session setup
//           e               Set QOS only upon receipt of FD_QOS
//       -r:Addr-IP          Act as a receiver
//       -s:Addr-IP          Send data to recipient's IP
//       -w                  Wait to send until RESV has arrived
//       -c                  Confirm RESV request (receiver only)
//       -f:[ff,se]          Filter style: fixed filter, shared 
//                             explicit
//
#include <winsock2.h>
#include <windows.h>
#include <qos.h>
#include <qossp.h>

#include "provider.h"
#include "printqos.h"

#include <stdio.h>
#include <stdlib.h>

#define QOS_BUFFER_SZ       16000 // Default buffer size for 
                                  // SIO_GET_QOS
#define DATA_BUFFER_SZ       2048 // Send/Recv buffer size

#define MAX_RECEIVERS        24   // Max number of receivers

#define SET_QOS_NONE          0   // No QOS
#define SET_QOS_BEFORE        1   // Set QOS on listening socket
#define SET_QOS_DURING        2   // Set QOS during cond accept
#define SET_QOS_AFTER         3   // Set QOS after accept completed
#define SET_QOS_EVENT         4   // Wait for FD_QOS before setting

int  iSetQos;                // when to set QOS?
     iNumSenders,
     iFlowStyle;
BOOL bSender,                // send or receive?
     bWaitToSend,            // wait to send data until RESV
     bOkayToSend,
     bConfirmResv;
char szReceiverAddr[64],     // receivers's address
     szSenderAddrs[MAX_RECEIVERS][64];
QOS  sendQos,
     recvQos;

RSVP_RESERVE_INFO  qosreserve;             // use to set filter style
FLOWDESCRIPTOR     flow[MAX_RECEIVERS];
RSVP_FILTERSPEC    filters[MAX_RECEIVERS];

//
// Setup some common FLOWSPECS
//
const FLOWSPEC flowspec_notraffic = {QOS_NOT_SPECIFIED,
                                     QOS_NOT_SPECIFIED,
                                     QOS_NOT_SPECIFIED,
                                     QOS_NOT_SPECIFIED,
                                     QOS_NOT_SPECIFIED,
                                     SERVICETYPE_NOTRAFFIC,
                                     QOS_NOT_SPECIFIED,
                                     QOS_NOT_SPECIFIED};

const FLOWSPEC flowspec_g711 = {8500,
                                680,
                                17000,
                                QOS_NOT_SPECIFIED,
                                QOS_NOT_SPECIFIED,
                                SERVICETYPE_CONTROLLEDLOAD,
                                340,
                                340};

const FLOWSPEC flowspec_besteffort = {2250,
                                      440,
                                      6500,
                                      QOS_NOT_SPECIFIED,
                                      QOS_NOT_SPECIFIED,
                                      SERVICETYPE_BESTEFFORT,
                                      340,
                                      340};

//
// Function: InitQos
//
// Description:
//    Setup the client and server QOS structures. This is broken
//    out into a separate function so you can change the requested
//    QOS parameters to see how that affects the application.
//
void InitQos()
{
    sendQos.SendingFlowspec = flowspec_g711;
    sendQos.ReceivingFlowspec =  flowspec_notraffic;
    sendQos.ProviderSpecific.buf = NULL;
    sendQos.ProviderSpecific.len = 0;

    recvQos.SendingFlowspec = flowspec_notraffic;
    recvQos.ReceivingFlowspec = flowspec_g711;
    recvQos.ProviderSpecific.buf = NULL;
    recvQos.ProviderSpecific.len = 0;
}

//
// Function: usage
//
// Description:
//    Print out usage information.
//
void usage(char *progname)
{
    printf("usage: %s -q:x -r -s:IP -c -f:filter\n", progname);
    printf("      -q:[b,d,a,e] When to request QOS\n");
    printf("          b        Set QOS before bind or connect\n");
    printf("          d        Set QOS during accept cond func\n");
    printf("          a        Set QOS after session setup\n");
    printf("          e        Set QOS only upon receipt of FD_QOS\n");
    printf("      -s:Addr      Send data to Addr\n");
    printf("      -r           Act as receiver (default)\n");
    printf("      -w           Wait to send until RESV has arrived\n");
    printf("      -c           Confirm RESV request\n");
    printf("      -f:filter    Filter style for receiver\n");
    printf("         ff          (Multiple) Fixed Filter\n");
    printf("         se          Shared explicit\n");
    ExitProcess(-1);
}

// 
// Function: ValidateArgs
//
// Description:
//    Parse command line arguments and set global variables to
//    indicate how the application should act.
//
void ValidateArgs(int argc, char **argv)
{
    char    *ptr=NULL;
    int      i;

    // Initialize globals to a default value
    //
    iSetQos = SET_QOS_NONE;
    bSender = FALSE;
    bWaitToSend = FALSE;
    bOkayToSend = FALSE;
    iNumSenders = 0;
    iFlowStyle = RSVP_SHARED_EXPLICIT_STYLE;
    bConfirmResv = FALSE;

    for(i=1; i < argc ;i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'q':        // When to set QOS
                    if (tolower(argv[i][3]) == 'b')
                        iSetQos = SET_QOS_BEFORE;
                    else if (tolower(argv[i][3]) == 'd')
                        iSetQos = SET_QOS_DURING;
                    else if (tolower(argv[i][3]) == 'a')
                        iSetQos = SET_QOS_AFTER;
                    else if (tolower(argv[i][3]) == 'e')
                        iSetQos = SET_QOS_EVENT;
                    else
                        usage(argv[0]);
                    break;
                case 's':        // sender
                    bSender = TRUE;
                    strcpy(szReceiverAddr, &argv[i][3]);
                    break;
                case 'r':        // receiver
                    bSender = FALSE;
                    if (strlen(argv[i]) > 3)
                        strcpy(szSenderAddrs[iNumSenders++], &argv[i][3]);
                    break;
                case 'w':       // wait to send data until
                                // RESV has arrived
                    bWaitToSend = TRUE;
                    break;
                case 'c':       // confirm RESV request
                    bConfirmResv = TRUE;
                    break;
                case 'f':       // Filter style
                    ptr = &argv[i][3];
                    while (*ptr)  
                        *ptr++ = tolower(*ptr);
                    if (!strncmp("ff", &argv[i][3], 2))
                        iFlowStyle = RSVP_FIXED_FILTER_STYLE;
                    else if (!strncmp("se", &argv[i][3], 2))
                        iFlowStyle = RSVP_SHARED_EXPLICIT_STYLE;
                    else
                        usage(argv[0]);
                    break;
                default:
                    usage(argv[0]);
                    break;
            }
        }
    }
    return;
}

//
// Function: AbleToSend
//
// Description:
//    Checks to send whether data can be sent on the socket before
//    any RESV message have arrived. This checks to see if the 
//    best effort level currently available on the network is 
//    sufficient for the QOS levels that were set on the socket.
//
BOOL AbleToSend(SOCKET s)
{
    int     ret;
    DWORD   dwCode = ALLOWED_TO_SEND_DATA,
            dwValue,
            dwBytes;

    ret = WSAIoctl(s, SIO_CHK_QOS, &dwCode, sizeof(dwCode),
                &dwValue, sizeof(dwValue), &dwBytes, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl() failed: %d\n", WSAGetLastError());
        return FALSE;
    }
    return (BOOL)dwValue;
}

//
// Function: ChkForQosStatus
//
// Description:
//    Check for the presence of a RSVP_STATUS_INFO object and
//    determine if the supplied flags are present in that
//    object.
//
DWORD ChkForQosStatus(QOS *lpqos, DWORD dwFlags)
{
    QOS_OBJECT_HDR   *objhdr= NULL;
    RSVP_STATUS_INFO *status=NULL;
    char             *bufptr= NULL;
    BOOL              bDone = FALSE;
    DWORD             objcount = 0;

    if (lpqos->ProviderSpecific.len == 0)
        return 0;

    bufptr = lpqos->ProviderSpecific.buf;
    objhdr = (QOS_OBJECT_HDR *)bufptr;


    while (!bDone)
    {
        // Only interested in status info objects
        //
        if (objhdr->ObjectType == RSVP_OBJECT_STATUS_INFO)
        {
            status = (RSVP_STATUS_INFO *)objhdr;
            if (status->StatusCode & dwFlags)
                return 1;
        }
        else if (objhdr->ObjectType == QOS_OBJECT_END_OF_LIST)
            bDone = TRUE;
        //
        // Increment past current object to the next object
        //
        bufptr += objhdr->ObjectLength;
        objcount += objhdr->ObjectLength;
        objhdr = (QOS_OBJECT_HDR *)bufptr;
        //
        // Make sure we haven't exceeded the provider specific
        //  buffer
        //
        if (objcount >= lpqos->ProviderSpecific.len)
            bDone = TRUE;
    }
    return 0;
}

BOOL SetQosDestAddr(SOCKET s, QOS *lpqos)
{
    QOS_DESTADDR    qosdest;
    SOCKADDR_IN     receiver;
    DWORD           dwBytes;
    int             ret;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(5150);
    receiver.sin_addr.s_addr = inet_addr(szReceiverAddr);
    printf("Receivers address is: %s\n", inet_ntoa(receiver.sin_addr));

    qosdest.ObjectHdr.ObjectType = QOS_OBJECT_DESTADDR;
    qosdest.ObjectHdr.ObjectLength = sizeof(QOS_DESTADDR);
    qosdest.SocketAddress = (SOCKADDR *)&receiver;
    qosdest.SocketAddressLength = sizeof(receiver);

    lpqos->ProviderSpecific.buf = (char *)&qosdest;
    lpqos->ProviderSpecific.len = sizeof(qosdest);

    PrintQos(lpqos);

    ret = WSAIoctl(s, SIO_SET_QOS, lpqos, 
        sizeof(QOS) + sizeof(QOS_DESTADDR), NULL, 0, &dwBytes,
        NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_SET_QOS) failed: %d\n",
            WSAGetLastError());
        return FALSE;
    }
    return TRUE;
}

//
// Function: SetQosReserveInfo
//
// Description:
//    This function simply fills out the RSVP_RESERV_INFO object
//    with the requested style and sets the RSVPFLOWDESCRIPTOR
//    and RSVP_FILTERSPEC with the sender's addresses according
//    to the selected filter style.
//
void SetQosReserveInfo(QOS *lpqos)
{
    DWORD  dwSize=0;
    int    i, j;

    memset(&qosreserve, 0, sizeof(qosreserve));
    //
    // Initialize the RSVP_RESERVE_INFO structure
    //
    qosreserve.ObjectHdr.ObjectType = RSVP_OBJECT_RESERVE_INFO;
    qosreserve.ObjectHdr.ObjectLength = sizeof(RSVP_RESERVE_INFO);
    qosreserve.Style = iFlowStyle;
    qosreserve.ConfirmRequest = bConfirmResv;
    qosreserve.NumPolicyElement  = 0;
    qosreserve.PolicyElementList = NULL;
    qosreserve.FlowDescList = flow;
    
    if (iFlowStyle == RSVP_SHARED_EXPLICIT_STYLE)
    {
        // For shared explicit there is one FLOWSPEC for all
        // senders.
        //
        qosreserve.NumFlowDesc = 1;

        flow[0].FlowSpec = lpqos->ReceivingFlowspec; 
        flow[0].NumFilters = iNumSenders;
        flow[0].FilterList = filters;

        for(i=0; i < iNumSenders ;i++) 
        {
            filters[i].Type = FILTERSPECV4;
            filters[i].FilterSpecV4.Address.Addr = inet_addr(szSenderAddrs[i]);
            filters[i].FilterSpecV4.Port = htons(5150);
        }
    }
    else
    {
        // For multiple fixed filter there is one FLOWSPEC for
        // each sender. This means that each RSVPFLOWDESCRIPTOR
        // references a single FLOWSPEC and a single RSVP_FILTERSPEC.
        //
        qosreserve.NumFlowDesc = iNumSenders;
        
        j=0;
        for(i=0; i < iNumSenders ;i++)
        {
            flow[i].FlowSpec = lpqos->ReceivingFlowspec;
            flow[i].NumFilters = 1;
            flow[i].FilterList = &filters[j];
        
            filters[j].Type = FILTERSPECV4;
            filters[j].FilterSpecV4.Address.Addr = inet_addr(szSenderAddrs[j]);
            filters[j].FilterSpecV4.Port = htons(5150);
            j++;
        }
    }
    lpqos->ProviderSpecific.buf = (char *)&qosreserve;
    lpqos->ProviderSpecific.len = sizeof(RSVP_RESERVE_INFO);

    return;
}

//
// Function: SetQosReceivers
//
// Description:
//    This function sets the RSVP_RESERVE_INFO provider specific
//    object if a different filter has been specified along with
//    the sender's IP address. It then calls SIO_SET_QOS to set
//    the supplied QOS parameters on the socket.
//
BOOL SetQosReceivers(SOCKET s, QOS *lpqos)
{
    int       ret;
    DWORD     dwBytes,
              dwSize;

    if (iNumSenders > 0)
    {
        SetQosReserveInfo(lpqos);
        dwSize = sizeof(QOS) + sizeof(RSVP_RESERVE_INFO);
    }
    else
    {
        lpqos->ProviderSpecific.buf = NULL;
        lpqos->ProviderSpecific.len = 0;

        dwSize = sizeof(QOS);
    }
    PrintQos(lpqos);

    ret = WSAIoctl(s, SIO_SET_QOS, lpqos, dwSize, NULL, 0, 
        &dwBytes, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_SET_QOS) failed: %d\n",
            WSAGetLastError());
        return FALSE;
    }
    return TRUE;
}

//
// Function: Receiver
//
// Description:
//
void DoStuff(SOCKET s)
{
    WSANETWORKEVENTS  ne;
    WSAEVENT          hEvent;
    SOCKADDR_IN       from,
                      local;
    WSABUF            wbuf;
    DWORD             dwBytes,
                      dwFromLen,
                      dwFlags;
    QOS              *lpqos=NULL;
    char              rcvbuf[DATA_BUFFER_SZ],
                      sndbuf[DATA_BUFFER_SZ],
                      qosbuf[QOS_BUFFER_SZ];
    int               ret;

    // No matter when we decide to set QOS, the socket should be
    // bound locally (even if its to INADDR_ANY). This is required
    // if you're not using WSAConnect which does an implicit bind,
    // but we'll do it anyway for simplicity.
    //
    if (bSender)
    {
 	local.sin_family = AF_INET;
	local.sin_port = htons(5150); //0);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        local.sin_family = AF_INET;
        local.sin_port = htons(5150);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return;
    }
    // For the receiver there's really only:
    //  1. Set QOS beforehand (SET_QOS_BEFORE)
    //  2. Set QOS upon an FD_QOS event (SET_QOS_EVENT);
    //
    if ((!bSender) && (iSetQos != SET_QOS_EVENT))
    {
        lpqos = &recvQos;
        if (SetQosReceivers(s, lpqos) == FALSE)
            return;
    }
    // For the sender you can:
    //  1. Set QOS before hand with SIO_SET_QOS (SET_QOS_BEFORE)
    //  2. Set QOS during WSAConnect (SET_QOS_DURING)
    //  3. Set QOS after WSAConnect (SET_QOSAFTER)
    //  4. Set QOS upon FD_QOS (SET_QOS_EVENT)
    //     (not really meaningful though - no RESV are generated
    //      by the receiver until it receives a PATH)
    //
    if ((bSender) && (iSetQos == SET_QOS_BEFORE))
    {
        // Set QOS before along with QOS_DESTADDR
        //
        lpqos = &sendQos;
        if (SetQosDestAddr(s, lpqos) == FALSE)
            return;
    }
    else if ((bSender) && (iSetQos == SET_QOS_DURING))
    {
        // Set QOS in WSAConnect call
        //
        SOCKADDR_IN   remote;

        lpqos = &sendQos;

        remote.sin_family = AF_INET;
        remote.sin_port = htons(5150);
        remote.sin_addr.s_addr = inet_addr(szReceiverAddr);
        printf("WSAConnect() to %s\n", inet_ntoa(remote.sin_addr));

        ret = WSAConnect(s, (SOCKADDR *)&remote, sizeof(remote), 
            NULL, NULL, lpqos, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAConnect() failed: %d\n", WSAGetLastError());
            return;
        }
    }
    else if ((bSender) && (iSetQos == SET_QOS_AFTER))
    {
        // Call WSAConnect() without any QOS parametrs and then do
        // a SIO_SET_QOS. We don't use SetQosDestAddr() as it always
        // sets a QOS_DESTADDR object which isn't necessary since
        // we've already associated an endpoint address with the
        // WSAConnect() call.
        //
        SOCKADDR_IN   remote;

        remote.sin_family = AF_INET;
        remote.sin_port = htons(5150);
        remote.sin_addr.s_addr = inet_addr(szReceiverAddr);
        printf("WSAConnect() to %s (NO QOS)\n", inet_ntoa(remote.sin_addr));

        ret = WSAConnect(s, (SOCKADDR *)&remote, sizeof(remote), 
            NULL, NULL, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAConnect() failed: %d\n", WSAGetLastError());
            return;
        }

        lpqos = &sendQos;
        ret = WSAIoctl(s, SIO_SET_QOS, lpqos, sizeof(QOS), NULL, 0,
            &dwBytes, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAIoctl(SIO_SET_QOS) failed: %d\n",
                WSAGetLastError());
            return;
        }
    }
    if (iSetQos == SET_QOS_EVENT)
    {
        // Just register for FD_QOS events
        //
        if (bSender)
        {
            lpqos = &sendQos;
            lpqos->SendingFlowspec.ServiceType |= SERVICE_NO_QOS_SIGNALING;
        }
        else
        {
            lpqos = &recvQos;
            lpqos->ReceivingFlowspec.ServiceType |= SERVICE_NO_QOS_SIGNALING;
        }
        if (SetQosDestAddr(s, lpqos) == FALSE)
            return;
    }

    hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed: %d\n", WSAGetLastError());
        return;
    }
 
    ret = WSAEventSelect(s, hEvent, FD_READ | FD_WRITE | FD_QOS);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAEventSelect() failed: %d\n", WSAGetLastError());
        return;
    }

    while (1)
    {
        ret = WSAWaitForMultipleEvents(1, &hEvent, FALSE, 
            WSA_INFINITE, FALSE);
        if (ret == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed: %d\n",
                WSAGetLastError());
            return;
        }

        ret = WSAEnumNetworkEvents(s, hEvent, &ne);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAEnumNetworkEvents() failed: %d\n",
                WSAGetLastError());
            return;
        }
        if (ne.lNetworkEvents & FD_READ)
        {
            if (ne.iErrorCode[FD_READ_BIT])
            {
                printf("FD_READ error: %d\n", 
                    ne.iErrorCode[FD_READ_BIT]);
            }
            else
                printf("FD_READ\n");
            dwFromLen = sizeof(from);
            wbuf.buf = rcvbuf;
            wbuf.len = DATA_BUFFER_SZ;
            dwFlags = 0;

            ret = WSARecvFrom(s, &wbuf, 1, &dwBytes, &dwFlags,
                (SOCKADDR *)&from, &dwFromLen, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSARecvFrom() failed: %d\n", 
                    WSAGetLastError());
                return;
            }
        }
        if (ne.lNetworkEvents & FD_WRITE)
        {
            if (ne.iErrorCode[FD_WRITE_BIT])
            {
                printf("FD_WRITE error: %d\n",
                    ne.iErrorCode[FD_WRITE_BIT]);
            }
            else
                printf("FD_WRITE\n");
            if ((bSender) && (!bWaitToSend))
            {
                SOCKADDR_IN        receiver;

                memset(sndbuf, '$', DATA_BUFFER_SZ);
                wbuf.buf = sndbuf;
                wbuf.len = DATA_BUFFER_SZ - 1;

                receiver.sin_family = AF_INET;
                receiver.sin_port = htons(5150);
                receiver.sin_addr.s_addr = inet_addr(szReceiverAddr);

                ret = WSASendTo(s, &wbuf, 1, &dwBytes, 0, 
                    (SOCKADDR *)&receiver, sizeof(receiver), NULL, NULL);
                if (ret == SOCKET_ERROR)
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                        continue;
                    printf("WSASendTo() failed: %d\n",
                        WSAGetLastError());
                    return;
                }
                printf("WSASendTo() wrote: %d bytes to %s\n",
                    dwBytes, inet_ntoa(receiver.sin_addr));
            }
        }
        if (ne.lNetworkEvents & FD_QOS)
        {
            if (ne.iErrorCode[FD_QOS_BIT])
            {
                if (ne.iErrorCode[FD_QOS_BIT] == WSA_QOS_RECEIVERS)
                {
                    printf("WSA_QOS_RECEIVERS\n");
                    bOkayToSend = TRUE;
                }
                else
                    printf("FD_QOS error: %d\n",
                        ne.iErrorCode[FD_QOS_BIT]);
            }
            printf("FD_QOS\n");

            lpqos = (QOS *)qosbuf;
            lpqos->ProviderSpecific.buf = &qosbuf[sizeof(QOS)];
            lpqos->ProviderSpecific.len = sizeof(qosbuf) - sizeof(QOS);

            ret = WSAIoctl(s, SIO_GET_QOS, NULL, 0, lpqos, 
                QOS_BUFFER_SZ, &dwBytes, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSAIoctl(SIO_GET_QOS) failed: %d\n",
                    WSAGetLastError());
                return;
            }
            PrintQos(lpqos);
            //
            // See if an RSVP_STATUS_INFO object has been returned
            //
            if (ChkForQosStatus(lpqos, WSA_QOS_RECEIVERS))
                bOkayToSend = TRUE;

            if ((bWaitToSend) && (bOkayToSend))
            {
                SOCKADDR_IN        receiver;

                memset(sndbuf, '$', DATA_BUFFER_SZ);
                wbuf.buf = sndbuf;
                wbuf.len = DATA_BUFFER_SZ - 1;

                receiver.sin_family = AF_INET;
                receiver.sin_port = htons(5150);
                receiver.sin_addr.s_addr = inet_addr(szReceiverAddr);
        
                ret = WSASendTo(s, &wbuf, 1, &dwBytes, 0, 
                    (SOCKADDR *)&receiver, sizeof(receiver), NULL, NULL);
                if (ret == SOCKET_ERROR)
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                        continue;
                    printf("WSASendTo() failed: %d\n",
                        WSAGetLastError());
                    return;
                }
                printf("WSASendTo() wrote: %d bytes to %s\n",
                    dwBytes, inet_ntoa(receiver.sin_addr));
            }
            if (iSetQos == SET_QOS_EVENT)
            {
                if (bSender)
                    lpqos->SendingFlowspec.ServiceType = 
                        sendQos.SendingFlowspec.ServiceType;
                else
                    lpqos->ReceivingFlowspec.ServiceType = 
                        recvQos.ReceivingFlowspec.ServiceType;
                ret = WSAIoctl(s, SIO_SET_QOS, lpqos, sizeof(QOS), 
                    NULL, 0, &dwBytes, NULL, NULL);
                if (ret == SOCKET_ERROR)
                {
                    printf("WSAIoctl(SIO_SET_QOS) failed: %d\n",
                        WSAGetLastError());
                    return;
                }
                iSetQos = SET_QOS_BEFORE;
            }
        }
    }
}

//
// Function: main
//
// Description:
//
int main(int argc, char **argv)
{
    WSADATA           wsd;
    WSAPROTOCOL_INFO *pinfo=NULL;
    SOCKET            s;

    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("Unable to load Winsock: %d\n", GetLastError());
        return -1;
    }
    pinfo = FindProtocolInfo(AF_INET, SOCK_DGRAM, IPPROTO_UDP,
                XP1_QOS_SUPPORTED);
    if (!pinfo)
    {
        printf("unable to find suitable provider!\n");
        return -1;
    }
    printf("Provider returned: %s\n", pinfo->szProtocol); 
   
    s = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
                FROM_PROTOCOL_INFO, pinfo, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }

    InitQos();

    DoStuff(s);

    closesocket(s);
    WSACleanup();
    return 0;
}
