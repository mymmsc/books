// Module Name: qosmcast.c
//
// Description:
//    This sample illustrates multicasting with QOS. QOS may be
//    set before calling WSAJoinLeaf, during WSAJoinLeaf, after
//    WSAJoinLeaf, or QOS may be set when an FD_QOS event has been
//    received. This sample acts as wither a sender or receiver. 
//    In both cases, the multicast group is joined. Multiple 
//    groups may be joined by specifying one or more -m:IP
//    options.
//  
// Compile:
//    cl qosmcast.c provider.c printqos.c ws2_32.lib
//
// Command Line Parameters/Options
//    qosmcast.exe -q:x -s -r -m:IP-addr
//        -q:[b,d,a,e]    When to request QOS
//            b           Set QOS before bind or connect
//            d           Set QOS during accept cond func
//            a           Set QOS after session setup
//            e           Set QOS only upon receipt of FD_QOS
//        -s              Act as sender
//        -r:IP           Act as receiver
//        -w              Wait to send until RESV has arrived
//        -m:addr         Multicast address to join
//        -c              Confirm reservation request
//
#include <winsock2.h>
#include <windows.h>

#include <qos.h>
#include <qossp.h>

#include "provider.h"
#include "printqos.h"

#include <stdio.h>
#include <stdlib.h>

#define QOS_BUFFER_SZ       32000      // Default buffer size for SIO_GET_QOS
#define DATA_BUFFER_SZ       2048      // Send/Recv buffer size

#define MAX_MULTICAST_GROUPS   32
#define MAX_RECEIVERS          32

#define SET_QOS_NONE          0      // No QOS
#define SET_QOS_BEFORE        1      // Set QOS on listening socket
#define SET_QOS_DURING        2      // Set QOS during conditional accept
#define SET_QOS_AFTER         3      // Set QOS after accept completed
#define SET_QOS_EVENT         4      // Wait for FD_QOS before setting

int   iSetQos,                       // When to set qos
      iNumGroups,                    // Number of groups to join
      iNumSenders;
      iFlowStyle=RSVP_DEFAULT_STYLE; // Style
char  szMulticastAddrs[MAX_MULTICAST_GROUPS][64],
      szSenderAddrs[MAX_RECEIVERS][64];
BOOL  bSender,                       // Sending or receiving?
      bWaitToSend,                   // Wait for confirmation before sending?
      bConfirmResv;
QOS   sendQos,                       // Sender's QOS values
      recvQos;                       // Receiver's QOS values

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

//
// Function: usage
//
// Description:
//    Print out usage information.
//
void usage(char *progname)
{
    printf("usage: %s -q:x -s -r[:IP] -m:IP-addr\n", progname);
    printf("      -q:[b,d,a,e]    When to request QOS\n");
    printf("          b           Set QOS before bind or connect\n");
    printf("          d           Set QOS during accept cond func\n");
    printf("          a           Set QOS after session setup\n");
    printf("          e           Set QOS only upon receipt of FD_QOS\n");
    printf("      -s              Act as sender\n");
    printf("      -r[:IP]         Act as receiver\n");
    printf("      -w              Wait to send until RESV has arrived\n");
    printf("      -m:addr         Multicast address to join\n");
    printf("      -f:[se:ff]      Filter style\n");
    printf("          se          Shared explicit style\n");
    printf("          ff          (Multiple) Fixed filter style\n");
    printf("      -c              Confirm reservation request\n");
    ExitProcess(-1);
}

// 
// Function: InitQos
//
// Description:
//    Initalize the sending and receiving QOS structures for use in
//    setting QOS on a socket. Also use the RSVP_RESERVE_INFO object
//    to request confirmation of the reservation request (which is
//    only valid for the receiver since it's the receiver who
//    generates the RESV statement).
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
    iNumGroups = 0;
    iNumSenders = 0;
    bSender = TRUE;
    bWaitToSend = FALSE;
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
                    break;
                case 'r':        // receiver
                    bSender = FALSE;
                    if (strlen(argv[i]) > 3)
                        strcpy(szSenderAddrs[iNumSenders++], &argv[i][3]);
                    break;
                case 'm':        // the multicast address to join
                    if (strlen(argv[i]) > 3)
                        strcpy(szMulticastAddrs[iNumGroups++], &argv[i][3]);
                    else
                        usage(argv[0]);
                    break;
                case 'w':       // wait to send data until
                                // RESV has arrived
                    bWaitToSend = TRUE;
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
                case 'c':       // Confirm reservation
                    bConfirmResv = TRUE;
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
        if (objhdr->ObjectType == RSVP_OBJECT_STATUS_INFO)
        {
            status = (RSVP_STATUS_INFO *)objhdr;
            if (status->StatusCode & dwFlags)
                return 1;
        }
        else if (objhdr->ObjectType == QOS_OBJECT_END_OF_LIST)
            bDone = TRUE;

        bufptr += objhdr->ObjectLength;
        objcount += objhdr->ObjectLength;
        objhdr = (QOS_OBJECT_HDR *)bufptr;

        if (objcount >= lpqos->ProviderSpecific.len)
            bDone = TRUE;
    }
    return 0;
}

//
// Function: MulticastDriver
//
// Description:
//    Because the only difference between the sender and the
//    receiver is calling WSARecvFrom or WSASendTo, we implement
//    both sides in this one function.
//
void MulticastDriver(SOCKET s)
{
    WSANETWORKEVENTS ne;
    SOCKADDR_IN      mcast,
                     from,
                     local;
    WSAEVENT         hEvent;
    WSABUF           wbuf;
    DWORD            dwBytes=0,
                     dwFlags=0,
                     dwFromLen,
                     dwBytesRecv,
                     dwBytesSent;
    QOS             *lpqos=NULL;
    char             rcvbuf[DATA_BUFFER_SZ],
                     sndbuf[DATA_BUFFER_SZ],
                     qosbuf[QOS_BUFFER_SZ];
    int              ret,
                     i;
    static BOOL      bOkToSend=FALSE;

    //
    // Create the event used to signal socket events
    //
    hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed: %d\n", WSAGetLastError());
        return;
    }

    local.sin_family = AF_INET;
    local.sin_port = htons(5150);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
   
    ret = bind(s, (SOCKADDR *)&local, sizeof(local));
    if (ret == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return;
    }

    // Check to see if we're supposed to set QOS before joining the 
    // multicast group or if we just want to enabled FD_QOS on the
    // socket.
    //
    lpqos = NULL;
    if (iSetQos == SET_QOS_BEFORE)
    {
        if (bSender)
            lpqos = &sendQos;
        else
            lpqos = &recvQos;
        //
        // Call this routine to set QOS for both senders and receivers.
        // In the case of senders, no provider specific objects are used.
        //
        if (SetQosReceivers(s, lpqos) == FALSE)
            return;
        lpqos = NULL;
    }
    else if (iSetQos == SET_QOS_EVENT)
    {
        // Only want to receive the FD_QOS event and set QOS
        // at a later time.
        //
        if (bSender)
        {
            lpqos = &sendQos;
            sendQos.SendingFlowspec.ServiceType = 
                sendQos.SendingFlowspec.ServiceType;
        }
        else
        {
            lpqos = &recvQos;
            recvQos.ReceivingFlowspec.ServiceType = 
                recvQos.ReceivingFlowspec.ServiceType;
        }
        if (SetQosReceivers(s, lpqos) == FALSE)
            return;
        lpqos = NULL;
    }
    else if (iSetQos == SET_QOS_DURING)
    {
        if (bSender)
            lpqos = &sendQos;
        else
            lpqos = &recvQos;
        //
        // If setting QOS in the WSAJoinLeaf call we need to set
        // the RSVP_RESERVE_INFO structure beforehand.
        //
        if (iNumSenders > 0)
            SetQosReserveInfo(lpqos);
    }
    else
        lpqos  = NULL;
    // Setup the flags to indicate whether we are sending or receiving
    //
    if (bSender)
        dwFlags = JL_SENDER_ONLY;
    else
        dwFlags = JL_RECEIVER_ONLY;

    for(i=0; i < iNumGroups ;i++)
    {
        // Setup the mutlicast address structure which will be used 
        // throughout this function.
        //
        mcast.sin_family = AF_INET;
        mcast.sin_port   = htons(5150);
        mcast.sin_addr.s_addr = inet_addr(szMulticastAddrs[i]);
        //
        // Join the multicast group!
        //
        ret = WSAJoinLeaf(s, (SOCKADDR *)&mcast, sizeof(mcast), NULL, NULL,
                    lpqos, NULL, dwFlags);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAJoinLeaf() failed: %d\n", WSAGetLastError());
            return;
        }
    }
    if (lpqos)
        PrintQos(lpqos);

    if (iSetQos == SET_QOS_AFTER)
    {
        // Setting QOS after joining the group. Since WSAJoinLeaf does
        // an implicit bind, we only have to call SIO_SET_QOS.
        //
        if (bSender)
            lpqos = &sendQos;
        else
            lpqos = &recvQos;

        if (SetQosReceivers(s, lpqos) == FALSE)
            return;
    }
    // Register for the events we want to receive
    //
    ret = WSAEventSelect(s, hEvent, FD_QOS | FD_READ | FD_WRITE);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAEventSelect() failed: %d\n", WSAGetLastError());
        return;
    }
    // Loop until done!
    //
    while (1)
    {
        // Wait until an event shows up
        //
        ret = WSAWaitForMultipleEvents(1, &hEvent, FALSE, WSA_INFINITE, FALSE);
        if (ret == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed: %d\n",
                WSAGetLastError());
            return;
        }
        // Enumerate any events that have occured.
        //
        ret = WSAEnumNetworkEvents(s, hEvent, &ne);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAEnumNetworkEvents() failed: %d\n", WSAGetLastError());
            return;
        }

        if (ne.lNetworkEvents & FD_READ)
        {
            // Got a FD_READ event. Check for errors and then read pending data.
            //
            if (ne.iErrorCode[FD_READ_BIT])
            {
                printf("FD_READ error: %d\n", ne.iErrorCode[FD_READ_BIT]);
            }
            dwFromLen = sizeof(from);
            wbuf.buf = rcvbuf;
            wbuf.len = DATA_BUFFER_SZ;
            dwFlags = 0;
        
            ret = WSARecvFrom(s, &wbuf, 1, &dwBytesRecv, &dwFlags, (SOCKADDR *)&from,
                        &dwFromLen, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSARecvFrom() failed: %d\n", WSAGetLastError());
                return;
            }
            rcvbuf[dwBytesRecv] = 0;
            printf("READ [%s] from [%s]\n", rcvbuf, inet_ntoa(from.sin_addr));
        }
        if (ne.lNetworkEvents & FD_WRITE)
        {
            // Got an FD_WRITE event. If we're the sender check to see if
            // we're waiting for confirmation before sending..if so don't send
            // until such a time. 
            //
            if (ne.iErrorCode[FD_WRITE_BIT])
            {
                printf("FD_WRITE error: %d\n", ne.iErrorCode[FD_WRITE_BIT]);
            }
            if ((bSender) && (!bWaitToSend))
            {
                memset(sndbuf, '#', DATA_BUFFER_SZ);
                wbuf.buf = sndbuf;
                wbuf.len = DATA_BUFFER_SZ - 1;

                for(i=0; i < iNumGroups ;i++)
                {
                    mcast.sin_family = AF_INET;
                    mcast.sin_port = htons(5150);
                    mcast.sin_addr.s_addr = inet_addr(szMulticastAddrs[i]);
                    ret = WSASendTo(s, &wbuf, 1, &dwBytesSent, 0, (SOCKADDR *)&mcast,
                            sizeof(mcast), NULL, NULL);
                    if (ret == SOCKET_ERROR)
                    {
                        printf("WSASendTo() failed: %d\n", WSAGetLastError());
                        return;
                    }
                    printf("WSASendTo() wrote: %d bytes to %s\n", dwBytesSent, 
                        inet_ntoa(mcast.sin_addr));
                }
            }
        }
        if (ne.lNetworkEvents & FD_QOS)
        {
            // Got an FD_QOS event. This can be triggered by a great
            // variety of things. Typically if an error code is set it is
            // actually more of a status such as notification that there
            // are senders or receivers out there so we need to check
            // for quite a few "error" codes to make sure we're doing
            // the right thing.
            //
            bOkToSend = FALSE;
            printf("FD_QOS event\n");
            if (ne.iErrorCode[FD_QOS_BIT])
            {
                switch (ne.iErrorCode[FD_QOS_BIT])
                {
                    case WSA_QOS_RECEIVERS:
                        printf("\tQOS recievers\n");
                        bOkToSend = TRUE;
                        break;
                    case WSA_QOS_SENDERS:
                        printf("\tQOS senders\n");
                        break;
                    case WSA_QOS_NO_SENDERS:
                        printf("\tQOS no senders\n");
                        break;
                    case WSA_QOS_NO_RECEIVERS:
                        printf("\tQOS no receivers\n");
                        break;
                    case WSA_QOS_REQUEST_CONFIRMED:
                        printf("\tQOS request confirmed\n");
                        break;
                    case WSA_QOS_ADMISSION_FAILURE:
                        printf("\tQOS admission failure\n");
                        return;
                    case WSA_QOS_POLICY_FAILURE:
                        printf("\tQOS policy failure\n");
                        return;
                    case WSA_QOS_BAD_STYLE:
                        printf("\tQOS bad style\n");
                        return;
                    case WSA_QOS_BAD_OBJECT:
                        printf("\tQOS bad object\n");
                        return;
                    case WSA_QOS_TRAFFIC_CTRL_ERROR:
                        printf("\tQOS traffic control error\n");
                        return;
                    case WSA_QOS_GENERIC_ERROR:
                        printf("\tQOS generic error\n");
                        return;
                    default:
                        printf("FD_QOS error: %d\n", ne.iErrorCode[FD_QOS_BIT]);
                        return;
                }
            }
            lpqos = (QOS *)qosbuf;
            lpqos->ProviderSpecific.buf = &qosbuf[sizeof(QOS)];
            lpqos->ProviderSpecific.len = sizeof(qosbuf) - sizeof(QOS);

            ret = WSAIoctl(s, SIO_GET_QOS, NULL, 0, lpqos, QOS_BUFFER_SZ,
                        &dwBytes, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSAIoctl() failed: %d\n", WSAGetLastError());
                return;
            }
            PrintQos(lpqos);
            //
            // If we're the sender and we've specified to wait for confirmation
            // before sending not only do we need to check the error code checked
            // earlier, but it is possible to return status information in the
            // provider specific buffer so we check that too.
            //
            if (bSender)
            {
                if (ChkForQosStatus(lpqos, WSA_QOS_RECEIVERS))
                    bOkToSend = TRUE;
                //
                // If we got confirmation, send data
                //
                if (bWaitToSend && bOkToSend)
                {
                    memset(sndbuf, '@', DATA_BUFFER_SZ);
                    wbuf.buf = sndbuf;
                    wbuf.len = DATA_BUFFER_SZ - 1;

                    for(i=0; i < iNumGroups ;i++)
                    {
                        mcast.sin_family = AF_INET;
                        mcast.sin_port = htons(5150);
                        mcast.sin_addr.s_addr = inet_addr(szMulticastAddrs[i]);

                        ret = WSASendTo(s, &wbuf, 1, &dwBytesSent, 0, (SOCKADDR *)&mcast,
                                sizeof(mcast), NULL, NULL);
                        if (ret == SOCKET_ERROR)
                        {
                            printf("WSASendTo() failed: %d\n", WSAGetLastError());
                            return;
                        }
                        printf("WSASendTo() wrote: %d bytes to %s\n", dwBytesSent, 
                            inet_ntoa(mcast.sin_addr));
                    }
                }
            }
        }
    }
    return;
}

//
// Function: main
//
// Description:
//    Parse command line arguments, find a QOS enabled provider that 
//    supports multicasting, create a socket, and call the driver
//    function.
//
int main(int argc, char **argv)
{
    WSAPROTOCOL_INFO *pi=NULL;
    WSADATA           wsd;
    SOCKET            s;

    // Parse command line args and initialize Winsock.
    //
    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Find a provider that supports QOS and multicasting.
    //
    pi = FindProtocolInfo(AF_INET, SOCK_DGRAM, IPPROTO_UDP,
             XP1_QOS_SUPPORTED | XP1_SUPPORT_MULTIPOINT);
    if (!pi)
    {
        printf("unable to find suitable provider!\n");
        WSACleanup();
        return -1;
    }
    printf("Provider returned: %s\n", pi->szProtocol);
    //
    // Create a socket.
    //
    s = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
            FROM_PROTOCOL_INFO, pi, 0, WSA_FLAG_OVERLAPPED |
            WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
    if (s == SOCKET_ERROR)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    // Initialize the sending and receiving QOS structures
    //
    InitQos();

    MulticastDriver(s);

    closesocket(s);
    WSACleanup();
    return 0;
}
