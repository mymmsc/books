// Module Name: wsockatm.c
//
// Description:
//    This is a simple Winsock ATM client/server echo program.
//    The server side requires a local interface to bind to.
//    This can be accomplished with the -l:interface option.
//    For the client side, simply specify the sever address
//    to connect to. The server handles client connections in
//    the same thread as the listening socket. This sample
//    is meant to illustrate how to use the ATM address family
//    and is not how an actual client/server should be implemented.
//
// Compile:
//    cl -o wsockatm.exe wsockatm.c support.c ws2_32.lib
//
// Command Line Parameters/Options
//    -a        Print out local ATM address
//    -s        Act as the server
//    -l:if     Interface to bind to. Either the index of the
//                interface or an NSAP ATM address.
//    -r:addr   Specify the server's NSAP ATM address.
//    -p:port   The 2 digit hexadecimal port number.
//
#include "support.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_ATM_ADDR_LEN         64
#define MAX_BUFFER             1024

WSAPROTOCOL_INFO  *lpSocketProtocol=NULL;

char szServerAddr[39],        // Server's ATM NSAP address
     szPort[3];               // Port number
BOOL bServer = FALSE;
DWORD dwInterface=-1;         // Which interfce to bind to?

//
// Function: usage
//
// Description:
//    Pring usage information.
//
void usage(char *progname)
{
    printf("usage: %s -a -s -l:interface -r:server-addr -p:port\n", 
        progname);
    printf("       -a        Print a list of ATM interface addresses\n");
    printf("       -s        Act as server\n");
    printf("       -l:if     Interface to bind to:\n");
    printf("                    Interface number (e.g. 0, 1, etc.)\n");
    printf("                    NSAP ATM address (38 character address)\n");
    printf("       -r:addr   NSAP ATM address of server\n"); 
    printf("       -p:port   Port number (2 hexcadecimal digits)\n");
    ExitProcess(1);
}

//
// Function: EnumerateATMAddresses
//
// Description:
//    Enumerate all ATM interfaces and print their addresses.
//
void EnumerateATMAddresses(WSAPROTOCOL_INFO *lpProtocol)
{
    SOCKET             s;
    SOCKADDR_ATM       atm_addr;
    char               szAddress[MAX_ATM_ADDR_LEN];
    DWORD              dwNumInterfaces,
                       dwAddrLen=MAX_ATM_ADDR_LEN,
                       i;

    s = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
            FROM_PROTOCOL_INFO, lpProtocol, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return;
    }
    dwNumInterfaces = GetNumATMInterfaces(s);

    for(i=0; i < dwNumInterfaces ;i++)
    {
        ZeroMemory((PVOID)&atm_addr, sizeof(SOCKADDR_ATM));
        ZeroMemory((PVOID)szAddress, MAX_ATM_ADDR_LEN);

        if (GetATMAddress(s, i, &atm_addr.satm_number) == FALSE)
        {
            printf("GetATMAddress failed!\n");
            break;
        }
        atm_addr.satm_family                 = AF_ATM;
        atm_addr.satm_number.AddressType     = ATM_NSAP;
        atm_addr.satm_number.NumofDigits     = ATM_ADDR_SIZE;
        atm_addr.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
        atm_addr.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
        atm_addr.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;
        if (WSAAddressToString((LPSOCKADDR)&atm_addr, sizeof(atm_addr), 
            lpProtocol, szAddress, &dwAddrLen))
        {
            printf("WSAAddressToString: %d\n", 
                WSAGetLastError());
            break;
        }             
        printf("ATM ADDRESS <%d>: '%s'\n", i,
            szAddress);
    }
    closesocket(s);

    return;
}

//
// Function: ValidateArgs
//
// Description:
//    Parse command line arguments and set global variables accordingly.
//
void ValidateArgs(int argc, char **argv)
{
    int                i;

    for(i=1; i < argc ;i++)
    {
        if (strlen(argv[i]) < 2)
            continue;
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'a':        // Get my ATM address
                    EnumerateATMAddresses(lpSocketProtocol);
                    ExitProcess(1);
                    break;
                case 's':        // Act as server
                    bServer = TRUE;
                    break;
                case 'l':        // Use this local interface
                    if (strlen(argv[i]) == 41)
                        strncpy(szServerAddr, &argv[i][3], 38);
                    else if (strlen(argv[i]) == 4)
                        dwInterface = atoi(&argv[i][3]);
                    else
                        usage(argv[0]);
                    break;
                case 'r':        // server's address
                    if (strlen(argv[i]) == 41)
                        strncpy(szServerAddr, &argv[i][3], 38);
                    else
                        usage(argv[0]);
                    break;
                case 'p':        // server's address
                    if (strlen(argv[i]) == 5)
                        strncpy(szPort, &argv[i][3], 2);
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
// Function: main
//
// Description:
//    This function parses arguments and starts either the client or server
//    depending on the arguments. The server will enumerate local ATM 
//    interfaces if necessary and bind to a local address and wait for
//    client connections. The server handles the client connection in the
//    same thread as the listening socket. The client only handles one 
//    connection and then exits. If running the client, create a socket 
//    and connect to the server. Once connected, send a message and read it 
//    back.
//
int main(int argc, char **argv)
{
    WSADATA      wsd;
    SOCKET       s;
    DWORD        dwAddrLen = sizeof(SOCKADDR_ATM);
    SOCKADDR_ATM atm_addr;
    int          ret, 
                 i;
    char         szAddress[41];

    // Initialize Winsock and parse command line arguments
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed!\n");
        return -1;
    }
    // Find the protocol entry supporting ATM and create a socket
    //
    lpSocketProtocol = FindProtocol();
    if (lpSocketProtocol == NULL)
    {
        printf("FindProtocol returned NULL!\n");
        return -1;
    }

    s = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
                  lpSocketProtocol, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
	return -1;
    }
    ValidateArgs(argc, argv);

    ZeroMemory((PVOID)&atm_addr, sizeof(atm_addr));
    atm_addr.satm_family                 = AF_ATM;
    atm_addr.satm_number.AddressType     = ATM_NSAP;
    atm_addr.satm_number.NumofDigits     = 20;
    atm_addr.satm_blli.Layer2Protocol    = SAP_FIELD_ABSENT;
    atm_addr.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
    atm_addr.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;

    // If an interface number was not supplied (i.e. a 38 character
    // NSAP ATM address was supplied).
    //
    if ((!bServer) || (dwInterface == -1))
    {
        strncpy(&szAddress[0], szServerAddr, 38);
        strncpy(&szAddress[38], szPort, 2);
        szAddress[40] = 0;

        AtoH(&atm_addr.satm_number.Addr[0], szAddress, 20);

    }
    else
    {
        // An index was supplied, look it up and get its address
        //
        if (GetATMAddress(s, dwInterface, &atm_addr.satm_number) == FALSE)
        {
             printf("Unable to get ATM interface\n");
        }
        AtoH(&atm_addr.satm_number.Addr[19], szPort, 1);
    }

    if (bServer)
    {
        SOCKET        sclient;
        SOCKADDR_ATM  atm_client;
        DWORD         atmaddrsz = sizeof(SOCKADDR_ATM),
                      dwClientAddrLen=MAX_ATM_ADDR_LEN,
                      dwAddrLen = MAX_BUFFER;
        struct fd_set fdaccept;
        char          recvbuf[MAX_BUFFER],
                      szClientAddr[MAX_ATM_ADDR_LEN],
                      szAddr[MAX_BUFFER];

        // Print out the address we're binding to
        //
        if (WSAAddressToString((LPSOCKADDR)&atm_addr, sizeof(atm_addr), 
            lpSocketProtocol, szAddr, &dwAddrLen))
        {
            printf("WSAAddressToString failed: %d\n", WSAGetLastError());
        }
        printf("Binding to: <%s>\n", szAddr);

        if (bind(s, (SOCKADDR *)&atm_addr, sizeof(atm_addr)) == SOCKET_ERROR)
        {
            printf("bind() failed; %d\n", WSAGetLastError());
            return -1;
        }
        listen(s, 7);

        FD_ZERO(&fdaccept);
        FD_SET(s, &fdaccept);

        select(0, &fdaccept, NULL, NULL, NULL);

        sclient = WSAAccept(s, (SOCKADDR *)&atm_client, &atmaddrsz, NULL, 0);
        if (sclient == INVALID_SOCKET)
        {
            printf("accept() failed; %d\n", WSAGetLastError());
            return -1;
        }

        ZeroMemory((PVOID)szClientAddr, MAX_ATM_ADDR_LEN);
        if (WSAAddressToString((LPSOCKADDR)&atm_client, sizeof(atm_client), 
            lpSocketProtocol, szClientAddr, &dwClientAddrLen))
        {
            printf("WSAAddressToString: %d\n", WSAGetLastError());
        }             
        printf("Client's ATM ADDRESS: <%s>\n", szClientAddr);        
        //
        // Handle the client connection until it closes.
        //
        while (1)
        {
            ret = recv(sclient, recvbuf, MAX_BUFFER, 0);
            if (ret == SOCKET_ERROR)
            {
                printf("recv() failed: %d\n", WSAGetLastError());
                return -1;
            }
            else if (ret == 0)
            {
                printf("graceful close\n");
                break;
            }
            recvbuf[ret] = '\0';
            printf("READ: '%s'\n", recvbuf);

            ret = send(sclient, recvbuf, ret, 0);
            if (ret == SOCKET_ERROR)
            {
                printf("send() failed; %d\n", WSAGetLastError());
                return -1;
            }
            printf("WROTE %d bytes\n", ret);
        }
        closesocket(sclient);
    }
    else
    {
        DWORD        dwAddrLen = MAX_BUFFER;
        char         sendbuf[MAX_BUFFER],
                     szAddr[MAX_BUFFER];


        // Connect and then send and recv data
        //
        if (WSAAddressToString((LPSOCKADDR)&atm_addr, sizeof(atm_addr), 
            lpSocketProtocol, szAddr, &dwAddrLen))
        {
            printf("WSAAddressToString failed: %d\n", WSAGetLastError());
        }
        printf("Connect to: <%s>\n", szAddr);
        if (connect(s, (SOCKADDR *)&atm_addr, sizeof(atm_addr)) == SOCKET_ERROR)
        {
            printf("connect() failed: %d\n", WSAGetLastError());
            return -1;
        }
        memset(sendbuf, 'a', 512);
        for(i=0; i < 10 ;i++)
        {
            ret = send(s, sendbuf, 64, 0);
            if (ret == SOCKET_ERROR)
            {
                printf("send() failed: %d\n", WSAGetLastError());
                return -1;
            }
            printf("Sent %d bytes\n", ret);
            
            ret = recv(s, sendbuf, ret, 0);
            if (ret == SOCKET_ERROR)
            {
                printf("recv() failed: %d\n", WSAGetLastError());
                return -1;
            }
            else if (ret == 0)
            {
                printf("Graceful close\n");
                break;
            }
            sendbuf[ret] = '\0';
            printf("Read: '%s'\n", sendbuf);
        }
    }
    closesocket(s);

    WSACleanup();
    return 0;
}
