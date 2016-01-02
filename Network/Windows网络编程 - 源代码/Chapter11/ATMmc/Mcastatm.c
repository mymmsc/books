// Module: Mcastatm.c
//
// Description:
//    This sample illustrates multicasting on an ATM network.
//    ATM multicasting features rooted control and data planes.
//    This means our multicast server explicitly invites each
//    leaf not via a WSAJoinLeaf call. The leaf nodes simply
//    wait for on an accept for the invitation.
//
//    For the root node we specify the -s options along with
//    one or more -l:Address options.  The address is the ATM
//    address of the leaf to invite. All ATM addresses are only
//    the first 38 chars. The port (selector) which is the other
//    2 characters of the address are given in the -p:xx option.
//
//    For the client simply call the app with no paramters (the
//    -i:Addr can be used for both client and server).
//
// Compilation:
//     cl -o Mcastatm Mcastatm.c Support.obj ws2_32.lib
//
// Command Line Options/Parametrs:
//     Mcastatm.exe -s -l:ATM-Addr -i:ATM-Addr -p:XX -n:int
//           -s       Act as root\n");
//           -l:str   Leaf address to invite (38 chars)
//                     May be specified multiple times.
//           -i:str   Local interface to bind to (38 chars)
//           -p:xx    Port number (2 hex chars)
//           -n:int   Number of packets to send
//
#include "Support.h"

#include <stdio.h>
#include <stdlib.h>


#define BUFSIZE               1024
#define MAX_ATM_LEAF             4

#define ATM_PORT_OFFSET       ((ATM_ADDR_SIZE*2)-2)
#define MAX_ATM_STR_LEN       (ATM_ADDR_SIZE*2)

DWORD  dwAddrCount = 0,
       dwDataCount = 20;
BOOL   bServer=FALSE,
       bLocalAddress=FALSE;
char   szLeafAddresses[MAX_ATM_LEAF][MAX_ATM_STR_LEN + 1],
       szLocalAddress[MAX_ATM_STR_LEN + 1],
       szPort[3];
SOCKET sLeafSock[MAX_ATM_LEAF];

// Module: usage
//
// Description:
//     Print usage information.
//
void usage(char *progname)
{
    printf("usage: %s [-s]\n", progname);
    printf("     -s       Act as root\n");
    printf("     -l:str   Leaf address to invite (38 chars)\n");
    printf("               May be specified multiple times\n");
    printf("     -i:str   Local interface to bind to (38 chars)\n");
    printf("     -p:xx    Port number (2 hex chars)\n");
    printf("     -n:int   Number of packets to send\n");
    ExitProcess(1);
}

// Module: ValidateArgs
//
// Description:
//     Parse command line arguments.
//
void ValidateArgs(int argc, char **argv)
{
    int      i;

    memset(szLeafAddresses, 0,
		MAX_ATM_LEAF * (MAX_ATM_STR_LEN + 1));
    memset(szPort, 0, sizeof(szPort));

    for(i=1; i < argc ;i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 's':     // Server
                    bServer = TRUE;
                    break;
                case 'l':     // Leaf address
                    if (strlen(argv[i]) > 3)
                    {
                        strncpy(szLeafAddresses[dwAddrCount++], 
                            &argv[i][3], MAX_ATM_STR_LEN - 2);
                    }
                    break;
                case 'i':     // Local interface
                    if (strlen(argv[i]) > 3)
                    {
                        strncpy(szLocalAddress, &argv[i][3],
                            MAX_ATM_STR_LEN-2);
                        bLocalAddress = TRUE;
                    }
                    break;
                case 'p':     // Port address to use
                    if (strlen(argv[i]) > 3)
                        strncpy(szPort, &argv[i][3], 2);
                    break;
                case 'n':     // Number of packets to send
                    if (strlen(argv[i]) > 3)
                        dwDataCount = atoi(&argv[i][3]);
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
// Function: Server
//
// Description:
//    Bind to the local interface and then invite each leaf
//    address that was specified on the command line.
//    Once each connection is made, send some data.
//
void Server(SOCKET s, WSAPROTOCOL_INFO *lpSocketProtocol)
{
    // Server routine
    //
    SOCKADDR_ATM  atmleaf, atmroot;
    WSABUF        wsasend;
    char          sendbuf[BUFSIZE],
                  szAddr[BUFSIZE];
    DWORD         dwBytesSent,
                  dwAddrLen=BUFSIZE,
                  dwNumInterfaces,
                  i;
    int           ret;

    // If no specified local interface is given pick the first
    // one
    //
    memset(&atmroot, 0, sizeof(SOCKADDR_ATM));
    if (!bLocalAddress)
    {
        dwNumInterfaces = GetNumATMInterfaces(s);
        GetATMAddress(s, 0, &atmroot.satm_number);
    }
    else
        AtoH(&atmroot.satm_number.Addr[0], szLocalAddress, 
            ATM_ADDR_SIZE - 1);
    //
    // Set the port number in the address structure
    //
    AtoH(&atmroot.satm_number.Addr[ATM_ADDR_SIZE - 1], szPort, 1);
    //
    // Fill in the rest of the SOCKADDR_ATM structure
    //
    atmroot.satm_family                 = AF_ATM;
    atmroot.satm_number.AddressType     = ATM_NSAP;
    atmroot.satm_number.NumofDigits     = ATM_ADDR_SIZE;
    atmroot.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
    atmroot.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
    atmroot.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;
    //
    // Print out what we're binding to and bind
    //
    if (WSAAddressToString((LPSOCKADDR)&atmroot, 
        sizeof(atmroot), lpSocketProtocol, szAddr, 
        &dwAddrLen))
    {
        printf("WSAAddressToString failed: %d\n", 
            WSAGetLastError());
    }
    printf("Binding to: <%s>\n", szAddr);

    if (bind(s, (SOCKADDR *)&atmroot, 
        sizeof(SOCKADDR_ATM)) == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return;
    }
    // Invite each leaf
    //
    for(i = 0; i < dwAddrCount; i++)
    {
        // Fill in the SOCKADDR_ATM structure for each leaf
        //
        memset(&atmleaf, 0, sizeof(SOCKADDR_ATM));
        AtoH(&atmleaf.satm_number.Addr[0], szLeafAddresses[i],
            ATM_ADDR_SIZE - 1);
        AtoH(&atmleaf.satm_number.Addr[ATM_ADDR_SIZE - 1], szPort,
            1);

        atmleaf.satm_family                 = AF_ATM;
        atmleaf.satm_number.AddressType     = ATM_NSAP;
        atmleaf.satm_number.NumofDigits     = ATM_ADDR_SIZE;
        atmleaf.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
        atmleaf.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
        atmleaf.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;
        //
        // Print out client's address and theN invite it
        //
        if (WSAAddressToString((LPSOCKADDR)&atmleaf, 
            sizeof(atmleaf), lpSocketProtocol, szAddr, 
            &dwAddrLen))
        {
            printf("WSAAddressToString failed: %d\n", 
                WSAGetLastError());
        }
        printf("[%02d] Inviting: <%s>\n", i, szAddr);
   
        if ((sLeafSock[i] = WSAJoinLeaf(s, 
            (SOCKADDR *)&atmleaf, sizeof(SOCKADDR_ATM), NULL, 
            NULL, NULL, NULL, JL_SENDER_ONLY)) 
            == INVALID_SOCKET)
        {
            printf("WSAJoinLeaf() failed: %d\n", 
                WSAGetLastError());
            WSACleanup();
            return;
        }
    }
    // Note that the ATM protocol is a bit different from TCP.
    // When the WSAJoinLeaf (or connect) call completes the
    // peer has not necessarily accepted the connection yet,
    // so immediately sending data will result in an error; 
    // therefore, we wait a short time.
    // 
    printf("Press a key to start sending.");
    getchar();
    printf("\n");
    //
    // Now send some data to the group address, which will
    // be replicated to all clients
    //
    wsasend.buf = sendbuf;
    wsasend.len = 128;
    for(i = 0; i < dwDataCount; i++)
    {
        memset(sendbuf, 'a' + (i%26), 128);
        ret = WSASend(s, &wsasend, 1, &dwBytesSent, 0, NULL, 
            NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSASend() failed: %d\n", WSAGetLastError());
            break;
        }
        printf("[%02d] Wrote: %d bytes\n", i, dwBytesSent);
        Sleep(500);
    }

    for(i = 0; i < dwAddrCount; i++)
        closesocket(sLeafSock[i]);
    return;
}

//
// Function: Client
//
// Description:
//    First, the client binds to the local interface (either one 
//    specified on the command line or the first local ATM address).
//    Next, it waits on an accept call for the root invitation. It
//    then waits to receive data.
//
void Client(SOCKET s, WSAPROTOCOL_INFO *lpSocketProtocol)
{
    SOCKET       sl;
    SOCKADDR_ATM atm_leaf,
                 atm_root;
    DWORD        dwNumInterfaces,
                 dwBytesRead,
                 dwAddrLen = BUFSIZE,
                 dwFlags,
                 i;
    WSABUF       wsarecv;
    char         recvbuf[BUFSIZE],
                 szAddr[BUFSIZE];
    int          iLen = sizeof(SOCKADDR_ATM),
                 ret;

    // Setup the local interface
    //
    memset(&atm_leaf, 0, sizeof(SOCKADDR_ATM));
    if (!bLocalAddress)
    {
        dwNumInterfaces = GetNumATMInterfaces(s);
        GetATMAddress(s, 0, &atm_leaf.satm_number);
    }
    else
        AtoH(&atm_leaf.satm_number.Addr[0], szLocalAddress, 
            ATM_ADDR_SIZE - 1);
    AtoH(&atm_leaf.satm_number.Addr[ATM_ADDR_SIZE - 1], 
        szPort, 1);
    //
    // Fill in the SOCKADDR_ATM structure
    //
    atm_leaf.satm_family                 = AF_ATM;
    atm_leaf.satm_number.AddressType     = ATM_NSAP;
    atm_leaf.satm_number.NumofDigits     = ATM_ADDR_SIZE;
    atm_leaf.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
    atm_leaf.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
    atm_leaf.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;
    //
    // Print the address we're binding to and bind
    //
    if (WSAAddressToString((LPSOCKADDR)&atm_leaf, 
        sizeof(atm_leaf), lpSocketProtocol, szAddr, 
        &dwAddrLen))
    {
        printf("WSAAddressToString failed: %d\n", 
            WSAGetLastError());
    }
    printf("Binding to: <%s>\n", szAddr);

    if (bind(s, (SOCKADDR *)&atm_leaf, sizeof(SOCKADDR_ATM)) 
            == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return;
    }
    listen(s, 1);
    //
    // Wait for the invitation
    //
    memset(&atm_root, 0, sizeof(SOCKADDR_ATM));
    if ((sl = WSAAccept(s, (SOCKADDR *)&atm_root, &iLen, NULL, 
        0)) == INVALID_SOCKET)
    {
        printf("WSAAccept() failed: %d\n", WSAGetLastError());
        return;
    }
    printf("Received a connection!\n");

    // Receive some data
    //
    wsarecv.buf = recvbuf;
    for(i = 0; i < dwDataCount; i++)
    {
        dwFlags = 0;
        wsarecv.len = BUFSIZE;
        ret = WSARecv(sl, &wsarecv, 1, &dwBytesRead, &dwFlags, 
            NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSARecv() failed: %d\n", WSAGetLastError());
            break;
        }
        if (dwBytesRead == 0)
            break;
        recvbuf[dwBytesRead] = 0;
        printf("[%02d] READ %d bytes: '%s'\n", i, dwBytesRead, 
            recvbuf);
    }
    closesocket(sl);

    return;
}

//
// Function: main
//
// Description:
//    This function loads Winsock library, parses command line 
//    arguments, creates the appropriate socket (with the right
//    root or leaf flags), and starts the client or the server
//    functions depending on the specified flags
//
int main(int argc, char **argv)
{
    WSADATA             wsd;
    SOCKET              s;
    WSAPROTOCOL_INFO    lpSocketProtocol;
    DWORD               dwFlags;

    ValidateArgs(argc, argv);
    //
    // Load the Winsock library
    //
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("WSAStartup failed\n");
        return -1;
    }
    // Find an ATM capable protocol
    //
    if (FindProtocol(&lpSocketProtocol) == FALSE)
    {
        printf("Unable to find ATM protocol entry!\n");
        return -1;
    }
    // Create the socket using the appripriate root or leaf flags
    //
    if (bServer)
        dwFlags = WSA_FLAG_OVERLAPPED | WSA_FLAG_MULTIPOINT_C_ROOT |
                  WSA_FLAG_MULTIPOINT_D_ROOT;
    else
        dwFlags = WSA_FLAG_OVERLAPPED | WSA_FLAG_MULTIPOINT_C_LEAF |
                  WSA_FLAG_MULTIPOINT_D_LEAF;

    if ((s = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
            FROM_PROTOCOL_INFO, &lpSocketProtocol, 0, 
            dwFlags)) == INVALID_SOCKET)
    {
        printf("socket failed with: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    // Start the correct driver, depending on which flags were
    // supplied on the command line
    //
    if (bServer)
    {
        Server(s, &lpSocketProtocol);
    }
    else
    {
        Client(s, &lpSocketProtocol);
    }
    closesocket(s);

    WSACleanup();
    return 0;
}
