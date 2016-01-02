// Module: Qostcp.c
//
// Description:
//    This sample illustrates unicast TCP connections. This sample
//    may act as either a client or server. For both, QOS may be 
//    set before connection or acceptance, during connection or
//    acceptance, after connection or acceptance, or once an 
//    FD_QOS event has been received. The server may handle up to
//    MAX_CONN number of client connections.
//
// Compile:
//    cl -o qostcp.exe qostcp.c printqos.c provider.c ws2_32.lib 
//
// Command Line Parameters/Options
//    qostcp.exe -q:flag -s -c:Server-IP -w
//       -q:[b,d,a,e]        When to request QOS
//           b               Set QOS before bind or connect
//           d               Set QOS during accept cond func
//           a               Set QOS after session setup
//           e               Set QOS only upon receipt of FD_QOS
//       -s                  Act as server
//       -c:Server-IP        Act as client
//       -w                  Wait to send until RESV has arrived
//       -r                  Confirm reservation request
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

#define SET_QOS_NONE          0   // No QOS
#define SET_QOS_BEFORE        1   // Set QOS on listening socket
#define SET_QOS_DURING        2   // Set QOS in conditional accept
#define SET_QOS_AFTER         3   // Set QOS after accept
#define SET_QOS_EVENT         4   // Wait for FD_QOS and then set

#define MAX_CONN              10

int  iSetQos,          // When to set QOS?
     nConns;
BOOL bServer,          // Client or server?
     bWaitToSend,      // Wait to send data until RESV
     bConfirmResv;
char szServerAddr[64]; // Server's address
QOS  clientQos,        // QOS client structure
     serverQos;        // QOS server structure
RSVP_RESERVE_INFO  qosreserve; 

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

const FLOWSPEC flowspec_guaranteed = {17000,
                                      1260,
                                      34000,
                                      QOS_NOT_SPECIFIED,
                                      QOS_NOT_SPECIFIED,
                                      SERVICETYPE_GUARANTEED,
                                      340,
                                      340};

//
// Function: SetReserveInfo
//
// Description:
//    For receivers, if a confirmation is requested this must be
//    done with an RSVP_RESERVE_INFO structure
//
void SetQosReserveInfo(QOS *lpqos)
{
    qosreserve.ObjectHdr.ObjectType = RSVP_OBJECT_RESERVE_INFO;
    qosreserve.ObjectHdr.ObjectLength = sizeof(RSVP_RESERVE_INFO);
    qosreserve.Style = RSVP_DEFAULT_STYLE;
    qosreserve.ConfirmRequest = bConfirmResv;
    qosreserve.NumPolicyElement  = 0;
    qosreserve.PolicyElementList = NULL;
    qosreserve.FlowDescList = NULL;

    lpqos->ProviderSpecific.buf = (char *)&qosreserve;
    lpqos->ProviderSpecific.len = sizeof(qosreserve);

    return;
}

//
// Function: InitQos
//
// Description:
//    Setup the client and server QOS structures. This is 
//    broken out into a separate function so you can change 
//    the requested QOS parameters to see how that affects 
//    the application.
//
void InitQos()
{
    clientQos.SendingFlowspec = flowspec_g711;
    clientQos.ReceivingFlowspec =  flowspec_notraffic;
    clientQos.ProviderSpecific.buf = NULL;
    clientQos.ProviderSpecific.len = 0;

    serverQos.SendingFlowspec = flowspec_notraffic;
    serverQos.ReceivingFlowspec = flowspec_g711;
    serverQos.ProviderSpecific.buf = NULL;
    serverQos.ProviderSpecific.len = 0;
    if (bConfirmResv)
        SetQosReserveInfo(&serverQos);
}

//
// Function: usage
//
// Description:
//    Print out usage information.
//
void usage(char *progname)
{
    printf("usage: %s -q:x -s -c:IP\n", progname);
    printf("      -q:[b,d,a,e] When to request QOS\n");
    printf("          b        Set QOS before bind or connect\n");
    printf("          d        Set QOS during accept cond func\n");
    printf("          a        Set QOS after session setup\n");
    printf("          e        Set QOS only upon receipt of FD_QOS\n");
    printf("      -s           Act as server\n");
    printf("      -c:Server-IP Act as client\n");
    printf("      -w           Wait to send until RESV has arrived\n");
    printf("      -r           Confirm reservatin request\n");
    ExitProcess(-1);
}

// 
// Function: ValidateArgs
//
// Description:
//    Parse command line arguments and set global variables to
//    indicate how the application should act
//
void ValidateArgs(int argc, char **argv)
{
    int      i;

    // Initialize globals to a default value
    //
    iSetQos = SET_QOS_NONE;
    bServer = TRUE;
    bWaitToSend = FALSE;
    bConfirmResv = FALSE;

    for(i = 1; i < argc; i++)
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
                case 's':        // Server
                    printf("Server flag set!\n");
                    bServer = TRUE;
                    break;
                case 'c':        // Client
                    printf("Client flag set!\n");
                    bServer = FALSE;
                    if (strlen(argv[i]) > 3)
                        strcpy(szServerAddr, &argv[i][3]);
                    else
                        usage(argv[0]);
                    break;
                case 'w':       // Wait to send data until
                                // RESV has arrived
                    bWaitToSend = TRUE;
                    break;
                case 'r':
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
//    object
//
DWORD ChkForQosStatus(QOS *lpqos, DWORD dwFlags)
{
    QOS_OBJECT_HDR   *objhdr = NULL;
    RSVP_STATUS_INFO *status = NULL;
    char             *bufptr = NULL;
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
// Function: HandleClientEvents
//
// Description:
//    This function is called by the Server function to handle
//    events which occured on client SOCKET handles. The socket
//    array is passed in along with the event array and the index
//    of the client who received the signal. Within the function
//    the event is decoded and the appropriate action occurs.
//
void HandleClientEvents(SOCKET socks[], HANDLE events[], int index)
{
    WSANETWORKEVENTS  ne;
    char              databuf[4096];
    WSABUF            wbuf;
    DWORD             dwBytesRecv,
                      dwFlags;
    int               ret,
                      i;

    // Enumerate the network events that occured
    //
    ret = WSAEnumNetworkEvents(socks[index], events[index], &ne);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAEnumNetworkEvents() failed: %d\n", 
            WSAGetLastError());
        return;
    }
    // Data to be read
    //
    if ((ne.lNetworkEvents & FD_READ) == FD_READ)
    {
        wbuf.buf = databuf;
        wbuf.len = 4096;

        if (ne.iErrorCode[FD_READ_BIT])
            printf("FD_READ error: %d\n", 
                ne.iErrorCode[FD_READ_BIT]);
        else
            printf("FD_READ\n");

        dwFlags = 0;
        ret = WSARecv(socks[index], &wbuf, 1, &dwBytesRecv, 
            &dwFlags, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSARecv() failed: %d\n", WSAGetLastError());
            return;
        }
        wbuf.len = dwBytesRecv;

        printf("Read: %d bytes\n", dwBytesRecv);
    }
    // Able to write data, nothing to do here
    //
    if ((ne.lNetworkEvents & FD_WRITE) == FD_WRITE)
    {
        if (ne.iErrorCode[FD_WRITE_BIT])
            printf("FD_WRITE error: %d\n", 
                ne.iErrorCode[FD_WRITE_BIT]);
        else
            printf("FD_WRITE\n");
    }
    // The client closed the connection. Close the socket on our 
    // end and clean up the data structures.
    //
    if ((ne.lNetworkEvents & FD_CLOSE) == FD_CLOSE)
    {
        if (ne.iErrorCode[FD_CLOSE_BIT])
            printf("FD_CLOSE error: %d\n", 
                ne.iErrorCode[FD_CLOSE_BIT]);
        else
            printf("FD_CLOSE ...\n");
        closesocket(socks[index]);
        WSACloseEvent(events[index]);

        socks[index] = INVALID_SOCKET;
        //
        // Remote the client socket entry from the array and 
        // compact the remaining clients to the beginning of the 
        // array
        //
        for(i = index; i < MAX_CONN - 1; i++)
            socks[i] = socks[i + 1];
        nConns--;
    }
    // Received an FD_QOS event. This could mean several things.
    //
    if ((ne.lNetworkEvents & FD_QOS) == FD_QOS)
    {
        char        buf[QOS_BUFFER_SZ];
        QOS        *lpqos = NULL;
        DWORD       dwBytes;

        if (ne.iErrorCode[FD_QOS_BIT])
            printf("FD_QOS error: %d\n", 
                ne.iErrorCode[FD_QOS_BIT]);
        else
            printf("FD_QOS\n");

        lpqos = (QOS *)buf;
        lpqos->ProviderSpecific.buf = &buf[sizeof(QOS)];
        lpqos->ProviderSpecific.len = sizeof(buf) - sizeof(QOS);

        ret = WSAIoctl(socks[index], SIO_GET_QOS, NULL, 0,
                buf, QOS_BUFFER_SZ, &dwBytes, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAIoctl(SIO_GET_QOS) failed: %d\n", 
                WSAGetLastError());
            return;
        }
        PrintQos(lpqos);
        //
        // See if we're set for receiving FD_QOS events only. 
        // If so we need to actually invoke QOS on the connection 
        // now otherwise client will never receive a RESV message.
        //
        if (iSetQos == SET_QOS_EVENT)
        {
            lpqos->ReceivingFlowspec.ServiceType = 
                serverQos.ReceivingFlowspec.ServiceType;

            ret = WSAIoctl(socks[index], SIO_SET_QOS, lpqos, 
                dwBytes, NULL, 0, &dwBytes, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSAIoctl(SIO_SET_QOS) failed: %d\n", 
                    WSAGetLastError());
                return;
            }
            //
            // Change iSetQos so we don't set QOS again if we 
            // receive another FD_QOS event
            //
            iSetQos = SET_QOS_BEFORE;
        }
    }
    return;
}

//
// Function: SrvCondAccept
//
// Description:
//    This is the conditional function for WSAAccept. There is a 
//    limitation with the QOS service provider that the QOS values
//    passed into here are unreliable so the option SET_QOS_DURING 
//    is rather useless unless we call SIO_SET_QOS with are own 
//    values (as opposed to what the client is requesting since 
//    that is what is supposed to be returned in lpSQOS). Note that 
//    on Windows 98 if lpSQOS is not NULL you have to use set some
//    kind of QOS values (with SIO_SET_QOS) in the conditional 
//    function otherwise WSAAccept will fail.
//
int CALLBACK SrvCondAccept(LPWSABUF lpCallerId, 
    LPWSABUF lpCallerdata, LPQOS lpSQOS, LPQOS lpGQOS, 
    LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP *g, 
    DWORD dwCallbackData)
{
    DWORD       dwBytes = 0;
    SOCKET      s = (SOCKET)dwCallbackData;
    SOCKADDR_IN client;
    int         ret;

    if (nConns == MAX_CONN)
        return CF_REJECT;

    memcpy(&client, lpCallerId->buf, lpCallerId->len);
    printf("Client request: %s\n", inet_ntoa(client.sin_addr));

    if (iSetQos == SET_QOS_EVENT)
    {
        printf("Setting for event!\n");
        serverQos.SendingFlowspec.ServiceType |= 
            SERVICE_NO_QOS_SIGNALING;
        serverQos.ReceivingFlowspec.ServiceType |= 
            SERVICE_NO_QOS_SIGNALING;

        ret = WSAIoctl(s, SIO_SET_QOS, &serverQos, 
            sizeof(serverQos), NULL, 0, &dwBytes, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAIoctl() failed: %d\n", 
                WSAGetLastError());
            return CF_REJECT;
        }
    }
    return CF_ACCEPT;
}

//
// Function: Server
//
// Description:
//    This server routine handles incoming client connections.
//    First it sets up the listening socket, sets QOS when
//    appropriate, and wait for incoming clients and events.
//
void Server(SOCKET s)
{
    SOCKET        sc[MAX_CONN + 1];
    WSAEVENT      hAllEvents[MAX_CONN+1];
    SOCKADDR_IN   local,
                  client;
    int           clientsz,
                  ret,
                  i;
    DWORD         dwBytesRet;
    WSANETWORKEVENTS ne;

    // Initialize the arrays to invalid values
    //
    for(i = 0; i < MAX_CONN+1; i++)
    {
        hAllEvents[i] = WSA_INVALID_EVENT;
        sc[i] = INVALID_SOCKET;
    }
    // Array index 0 will be our listening socket
    //
    hAllEvents[0] = WSACreateEvent();
    sc[0]         = s;
    nConns        = 0;

    local.sin_family = AF_INET;
    local.sin_port = htons(5150);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return;
    }
    listen(s, 7);

    if (iSetQos == SET_QOS_BEFORE)
    {
        ret = WSAIoctl(sc[0], SIO_SET_QOS, &serverQos, 
            sizeof(serverQos), NULL, 0, &dwBytesRet, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAIoctl(SIO_SET_QOS) failed: %d\n", 
                WSAGetLastError());
            return;
        }
        printf("Set QOS on listening socket:\n");
        PrintQos(&serverQos);
    }

    if (WSAEventSelect(sc[0], hAllEvents[0], FD_ACCEPT) == 
        SOCKET_ERROR)
    {
        printf("WSAEventSelect() failed: %d\n", WSAGetLastError());
        return;
    }

    while (1)
    {
        ret = WSAWaitForMultipleEvents(nConns+1, hAllEvents, FALSE,
            WSA_INFINITE, FALSE); 
        if (ret == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleObject() failed: %d\n",
                WSAGetLastError());
            return;
        }
        if ((i = ret - WSA_WAIT_EVENT_0) > 0)  // Client network event
            HandleClientEvents(sc, hAllEvents, i);
        else
        {
            ret = WSAEnumNetworkEvents(sc[0], hAllEvents[0], 
                &ne);
            if (ret == SOCKET_ERROR)
            {
                printf("WSAEnumNetworkevents() failed: %d\n", 
                    WSAGetLastError());
                return;
            }
            if ((ne.lNetworkEvents & FD_ACCEPT) == FD_ACCEPT)
            {
                if (ne.iErrorCode[FD_ACCEPT_BIT])
                    printf("FD_ACCEPT error: %d\n", 
                        ne.iErrorCode[FD_ACCEPT_BIT]);
                else
                    printf("FD_ACCEPT\n");

                clientsz = sizeof(client);
                sc[++nConns] = WSAAccept(s, (SOCKADDR *)&client, 
                    &clientsz, SrvCondAccept, sc[nConns]);
                if (sc[nConns] == SOCKET_ERROR)
                {
                    printf("WSAAccept() failed: %d\n", 
                        WSAGetLastError());
                    nConns--;
                    return;
                }
                hAllEvents[nConns] = WSACreateEvent();

                Sleep(10000);
                if (iSetQos == SET_QOS_AFTER)
                {
                    ret = WSAIoctl(sc[nConns], SIO_SET_QOS, 
                        &serverQos, sizeof(serverQos), NULL, 0,
                        &dwBytesRet, NULL, NULL);
                    if (ret == SOCKET_ERROR)
                    {
                        printf("WSAIoctl() failed: %d\n", 
                            WSAGetLastError());
                        return;
                    }
                }
                ret = WSAEventSelect(sc[nConns],
                    hAllEvents[nConns], FD_READ | FD_WRITE | 
                    FD_CLOSE | FD_QOS);
                if (ret == SOCKET_ERROR)
                {
                    printf("WSAEventSelect() failed: %d\n", 
                        WSAGetLastError());
                    return;
                }
            }
            if (ne.lNetworkEvents & FD_CLOSE)
                printf("FD_CLOSEn");
            if (ne.lNetworkEvents & FD_READ)
                printf("FD_READn");
            if (ne.lNetworkEvents & FD_WRITE)
                printf("FD_WRITEn");
            if (ne.lNetworkEvents & FD_QOS)
                printf("FD_QOS\n");
        }
    }
    return;
}

//
// Function: Client
//
// Description:
//    The client routine initiates the connection, sets QOS when
//    appropriate, and handle incoming events.
//
void Client(SOCKET s)
{
    SOCKADDR_IN  server,
                 local;
    WSABUF       wbuf;
    DWORD        dwBytes,
                 dwBytesSent,
                 dwBytesRecv,
                 dwFlags;
    HANDLE       hEvent;
    int          ret, i;
    char         databuf[DATA_BUFFER_SZ];
    QOS         *lpqos;
    WSANETWORKEVENTS ne;

    hEvent = WSACreateEvent();
    if (hEvent == NULL)
    {
        printf("WSACreateEvent() failed: %d\n", WSAGetLastError());
        return;
    }

    lpqos = NULL;
    if (iSetQos == SET_QOS_BEFORE)
    {
        local.sin_family = AF_INET;
        local.sin_port = htons(0);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(s, (SOCKADDR *)&local, sizeof(local)) == 
            SOCKET_ERROR) 
        {
            printf("bind() failed: %d\n", WSAGetLastError());
            return;
        }
        ret = WSAIoctl(s, SIO_SET_QOS, &clientQos, 
            sizeof(clientQos), NULL, 0, &dwBytes, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAIoclt(SIO_SET_QOS) failed: %d\n",
                WSAGetLastError());
            return;
        }
    }
    else if (iSetQos == SET_QOS_DURING)
        lpqos = &clientQos;
    else if (iSetQos == SET_QOS_EVENT)
    {
        clientQos.SendingFlowspec.ServiceType |= 
            SERVICE_NO_QOS_SIGNALING;
        clientQos.ReceivingFlowspec.ServiceType |= 
            SERVICE_NO_QOS_SIGNALING;

        ret = WSAIoctl(s, SIO_SET_QOS, &clientQos, 
            sizeof(clientQos), NULL, 0, &dwBytes, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAIoctl() failed: %d\n", WSAGetLastError());
            return;
        }
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(5150);
    server.sin_addr.s_addr = inet_addr(szServerAddr);

    printf("Connecting to: %s\n", inet_ntoa(server.sin_addr));

    ret = WSAConnect(s, (SOCKADDR *)&server, sizeof(server),
        NULL, NULL, lpqos, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAConnect() failed: %d\n", WSAGetLastError());
        return;
    }


    ret = WSAEventSelect(s, hEvent, FD_READ | FD_WRITE | 
        FD_CLOSE | FD_QOS);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAEventSelect() failed: %d\n", WSAGetLastError());
        return;
    }

    wbuf.buf = databuf;
    wbuf.len = DATA_BUFFER_SZ;

    memset(databuf, '#', DATA_BUFFER_SZ);
    databuf[DATA_BUFFER_SZ-1] = 0;
 
    while (1)
    {
        ret = WSAWaitForMultipleEvents(1, &hEvent, FALSE, 
            WSA_INFINITE, FALSE);
        if (ret == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMulipleEvents() failed: %d\n", 
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
                printf("FD_READ error: %d\n", 
                    ne.iErrorCode[FD_READ_BIT]);
            else
                printf("FD_READ\n");

            wbuf.len = 4096;
            dwFlags = 0;
            ret = WSARecv(s, &wbuf, 1, &dwBytesRecv, &dwFlags, 
                NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSARecv() failed: %d\n", 
                    WSAGetLastError());
                return;
            }
            printf("Read: %d bytes\n", dwBytesRecv);

            wbuf.len = dwBytesRecv;
            ret = WSASend(s, &wbuf, 1, &dwBytesSent, 0, NULL, 
                NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSASend() failed: %d\n", 
                    WSAGetLastError());
                return;
            }
            printf("Sent: %d bytes\n", dwBytesSent);
        }
        if (ne.lNetworkEvents & FD_WRITE)
        {
            if (ne.iErrorCode[FD_WRITE_BIT])
                printf("FD_WRITE error: %d\n", 
                    ne.iErrorCode[FD_WRITE_BIT]);
            else
                printf("FD_WRITE\n");

            if (!bWaitToSend)
            {
                wbuf.buf = databuf;
                wbuf.len = DATA_BUFFER_SZ;
                //
                // If the network can't support the bandwidth
                // don't send
                //
                if (!AbleToSend(s))
                {
                    printf("Network is unable to provide "
						"sufficient best effort bandwidth\n");
                    printf("before the reservation "
						"request is approved\n");
                }
                
                for(i = 0; i < 1; i++)
                {
                    ret = WSASend(s, &wbuf, 1, &dwBytesSent, 0, 
                        NULL, NULL);
                    if (ret == SOCKET_ERROR)
                    {
                        printf("WSASend() failed: %d\n", 
                            WSAGetLastError());
                        return;
                    }
                    printf("Sent: %d bytes\n", dwBytesSent);
                }
            }
        }
        if (ne.lNetworkEvents & FD_CLOSE)
        {
            if (ne.iErrorCode[FD_CLOSE_BIT])
                printf("FD_CLOSE error: %d\n", 
                    ne.iErrorCode[FD_CLOSE_BIT]);
            else
                printf("FD_CLOSE ...\n");
            closesocket(s);
            WSACloseEvent(hEvent);
            return;
        }
        if (ne.lNetworkEvents & FD_QOS)
        {
            char        buf[QOS_BUFFER_SZ];
            QOS        *lpqos = NULL;
            DWORD       dwBytes;
            BOOL        bRecvRESV = FALSE;

            if (ne.iErrorCode[FD_QOS_BIT])
            {
                printf("FD_QOS error: %d\n", 
                    ne.iErrorCode[FD_QOS_BIT]);
                if (ne.iErrorCode[FD_QOS_BIT] == WSA_QOS_RECEIVERS)
                    bRecvRESV = TRUE;
            }
            else
                printf("FD_QOS\n");

            lpqos = (QOS *)buf;
            ret = WSAIoctl(s, SIO_GET_QOS, NULL, 0,
                    buf, QOS_BUFFER_SZ, &dwBytes, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                printf("WSAIoctl(SIO_GET_QOS) failed: %d\n", 
                    WSAGetLastError());
                return;
            }
            PrintQos(lpqos);
            //
            // Check to see if there is a status object returned 
            // in the QOS structure which may also contain the
            // WSA_QOS_RECEIVERS flag
            //
            if (ChkForQosStatus(lpqos, WSA_QOS_RECEIVERS))
                bRecvRESV = TRUE;

            if (iSetQos == SET_QOS_EVENT)
            {
                lpqos->SendingFlowspec.ServiceType = 
                    clientQos.SendingFlowspec.ServiceType;
                ret = WSAIoctl(s, SIO_SET_QOS, lpqos, dwBytes,
                        NULL, 0, &dwBytes, NULL, NULL);
                if (ret == SOCKET_ERROR)
                {
                    printf("WSAIoctl(SIO_SET_QOS) failed: %d\n", 
                        WSAGetLastError());
                    return;
                }
                //
                // Change iSetQos so we don't set QOS again if we 
                // receive another FD_QOS event
                //
                iSetQos = SET_QOS_BEFORE;
            }

            if (bWaitToSend && bRecvRESV)
            {
                wbuf.buf = databuf;
                wbuf.len = DATA_BUFFER_SZ;

                for(i = 0; i < 1; i++)
                {
                    ret = WSASend(s, &wbuf, 1, &dwBytesSent, 0, 
                        NULL, NULL);
                    if (ret == SOCKET_ERROR)
                    {
                        printf("WSASend() failed: %d\n", 
                            WSAGetLastError());
                        return;
                    }
                    printf("Sent: %d bytes\n", dwBytesSent);
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
//    Initialize Winsock, parse command line arguments, create
//    a QOS TCP socket, and call the appropriate handler 
//    routine depending on the arguments supplied
//
int main(int argc, char **argv)
{
    WSADATA           wsd;
    WSAPROTOCOL_INFO *pinfo = NULL;
    SOCKET            s;

    // Parse the command line
    ValidateArgs(argc, argv);
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("Unable to load Winsock: %d\n", GetLastError());
        return -1;
    }
    pinfo = FindProtocolInfo(AF_INET, SOCK_STREAM, IPPROTO_TCP,
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

    if (bServer)
        Server(s);
    else
        Client(s);

    closesocket(s);
    WSACleanup();
    return 0;
}
