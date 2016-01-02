// Module: rcvall.c
//
// Description:
//    This sample shows how to use the ioctls SIO_RCVALL,
//    SIO_RCVALL_MCAST, and SIO_RCVALL_IGMPMCAST. This sample
//    captures all packets of the given type and also is able
//    to set filters on source and destination IP addresses
//    and ports. This sample is Windows 2000 only.
//
// Compile:
//    cl rcvall.c parser.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    rcvall.exe -t:[ip|igmp|multicast] -i:int -sa:IP -sp:port
//               -da:IP -dp:port
//           -t:string           Filter traffic type
//                 ip            Capture all IP packets
//                 igmp          Capture all IGMP packets only
//                 multicast     Capture all multicast IP packets
//           -i:int              Capture on this interface
//                                This is a zero based index of the 
//                                local interfaces
//           -sa:IP              Filter on source address
//           -sp:Port            Filter on source port
//           -da:IP              Filter on dest address
//           -dp:Port            Filter on dest port
//
//
#include "parser.h"

#include <mstcpip.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

DWORD  dwIoControlCode=SIO_RCVALL,
       dwProtocol=IPPROTO_IP,
       dwInterface=0;

//
// Filters
//
unsigned int   uiSourceAddr=0,
               uiDestAddr=0;
unsigned short usSourcePort = 0,
               usDestPort = 0;
BOOL           bFilter=FALSE;

void PrintInterfaceList();
int  GetInterface(SOCKET s, SOCKADDR_IN *ifx, int num);

//
// Function: usage
// 
// Description:
//    Prints usage information.
//
void usage(char *progname)
{
    printf("usage: %s -t:traffic-type [interface-num]\n\n", progname);
    printf("       -t:string           Filter traffic type\n");
    printf("             Available traffic types:\n");
    printf("               ip          Capture all IP packets\n");
    printf("               igmp        Capture all IGMP packets only\n");
    printf("               multicast   Capture all multicast IP packets\n");
    printf("       -i:int              Capture on this interface\n");
    printf("             Available interfaces:\n");
    PrintInterfaceList();
    printf("       -sa:IP              Filter on source address\n");
    printf("       -sp:Port            Filter on source port\n");
    printf("       -da:IP              Filter on dest address\n");
    printf("       -dp:Port            Filter on dest port\n");

    WSACleanup();
    ExitProcess(-1);
}

//
// Function: ValidateArgs
// 
// Description:
//    This function parses the command line arguments and
//    sets global variables to indicate how the app should act.
//
void ValidateArgs(int argc, char **argv)
{
    int   i;
    char *ptr;

    for(i=1; i < argc ;i++)
    {
        if (strlen(argv[i]) < 2)
            continue;
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 't':        // traffic type
                    ptr = &argv[i][3];
                    while (*ptr)
                        *ptr++ = tolower(*ptr);

                    if (!strcmp(&argv[i][3], "ip"))
                    {
                        dwIoControlCode = SIO_RCVALL;
                        dwProtocol = IPPROTO_IP;
                    }
                    else if (!strcmp(&argv[i][3], "igmp"))
                    {
                        dwIoControlCode = SIO_RCVALL_IGMPMCAST;
                        dwProtocol = IPPROTO_IGMP;
                    }
                    else if (!strcmp(&argv[i][3], "multicast"))
                    {
                        dwIoControlCode = SIO_RCVALL_MCAST;
                        dwProtocol = IPPROTO_IGMP;
                    }
                    else
                        usage(argv[0]);
                    break;
                case 'i':        // interface number
                    dwInterface = atoi(&argv[i][3]);
                    break;
                case 's':        // Filter on source ip or port
                    if (tolower(argv[i][2]) == 'a')
                        uiSourceAddr = ntohl(inet_addr(&argv[i][4]));
                    else if (tolower(argv[i][2]) == 'p')
                        usSourcePort = (unsigned short)atoi(&argv[i][4]);
                    else
                        usage(argv[0]);
                    bFilter = TRUE;
                    break;
                case 'd':        // Filter on dest ip or port
                    if (tolower(argv[i][2]) == 'a')
                        uiDestAddr = ntohl(inet_addr(&argv[i][4]));
                    else if (tolower(argv[i][2]) == 'p')
                        usDestPort = (unsigned short)atoi(&argv[i][4]);
                    else
                        usage(argv[0]);
                    bFilter = TRUE;
                    break;
                default:
                    usage(argv[0]);
            }
        }
    }
    return;
}

//
// Function: PrintInterfaceList
//
// Description:
//    This function prints all local IP interfaces.
//
void PrintInterfaceList()
{
    SOCKET_ADDRESS_LIST *slist=NULL;
    SOCKET               s;
    char                 buf[2048];
    DWORD                dwBytesRet;
    int                  ret,
                         i;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s == SOCKET_ERROR)
    {
        printf("socket() failed: %d\n", WSAGetLastError());
        return;
    }
    ret = WSAIoctl(s, SIO_ADDRESS_LIST_QUERY, NULL, 0, buf, 2048,
                &dwBytesRet, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_ADDRESS_LIST_QUERY) failed: %d\n",
            WSAGetLastError());
        return;
    }
    slist = (SOCKET_ADDRESS_LIST *)buf;
    for(i=0; i < slist->iAddressCount ;i++)
    {
        printf("               %d [%s]\n", i, 
            inet_ntoa(((SOCKADDR_IN *)slist->Address[i].lpSockaddr)->sin_addr));
    }
    closesocket(s);
    return;
}

//
// Function: GetInterface
//
// Description:
//    This function retrieves a zero based index and returns
//    the IP interface corresponding to that.
//
int GetInterface(SOCKET s, SOCKADDR_IN *ifx, int num)
{
    SOCKET_ADDRESS_LIST *slist=NULL;
    char                 buf[2048];
    DWORD                dwBytesRet;
    int                  ret;

    ret = WSAIoctl(s, SIO_ADDRESS_LIST_QUERY, NULL, 0, buf, 2048,
                &dwBytesRet, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_ADDRESS_LIST_QUERY) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    slist = (SOCKET_ADDRESS_LIST *)buf;

    if (num >= slist->iAddressCount)
        return -1;

    ifx->sin_addr.s_addr = ((SOCKADDR_IN *)slist->Address[num].lpSockaddr)->sin_addr.s_addr;

    return 0;
}

//
// Function: main
//
// Description:
//    This function loads Winsock, parses the command line, and
//    begins receiving packets. Once a packet is received they
//    are decoded. Because we are receiving IP datagrams, the
//    receive call will return whole datagrams.
//
int main(int argc, char **argv)
{
    SOCKET        s;
    WSADATA       wsd;
    SOCKADDR_IN   if0;
    int           ret,
                  count;
    unsigned int  optval;
    DWORD         dwBytesRet,
                  dwFlags,
                  nproc;
    char          rcvbuf[MAX_IP_SIZE];
    WSABUF        wbuf;

    // Load Winsock
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Parse the command line
    //
    ValidateArgs(argc, argv);
    if (bFilter)
    {
        printf("Source Port: %d\n", usSourcePort);
        printf("Dest   Port: %d\n", usDestPort);
    }
    // Create a raw socket for receiving IP datagrams
    //
    s = WSASocket(AF_INET, SOCK_RAW, dwProtocol, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Get an interface to read IP packets on
    //
    if (GetInterface(s, &if0, dwInterface) != 0)
    {
        printf("Unable to obtain an interface\n");
        return -1;
    }
    printf("Binding to IF: %s\n", inet_ntoa(if0.sin_addr));
    //
    // This socket must be bound before calling the ioctl
    //
    if0.sin_family = AF_INET;
    if0.sin_port = htons(0);

    if (bind(s, (SOCKADDR *)&if0, sizeof(if0)) == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return -1;
    }
    //
    // Set the SIO_RCVALLxxx ioctl
    //
    optval = 1;
    if (WSAIoctl(s, dwIoControlCode, &optval, sizeof(optval),
            NULL, 0, &dwBytesRet, NULL, NULL) == SOCKET_ERROR)
    {
        printf("WSAIotcl(%d) failed; %d\n", dwIoControlCode,
            WSAGetLastError());
        return -1;
    }
    // Start receiving IP datagrams until interrupted
    // 
    count = 0;
    while (1)
    {
        wbuf.len = MAX_IP_SIZE;
        wbuf.buf = rcvbuf;
        dwFlags  = 0;

        ret = WSARecv(s, &wbuf, 1, &dwBytesRet, &dwFlags, NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSARecv() failed: %d\n", WSAGetLastError());
            return -1;
        }
        // Decode the IP header
        //
        if (!(nproc = DecodeIPHeader(&wbuf, uiSourceAddr, usSourcePort,
                uiDestAddr, usDestPort)))
        {
            printf("Error decoding IP header!\n");
            break;
        }
    }
    // Cleanup
    //
    closesocket(s);
    WSACleanup();
    return 0;
}
