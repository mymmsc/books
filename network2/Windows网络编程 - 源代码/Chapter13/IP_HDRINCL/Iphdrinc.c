// Module Name: Iphdrinc.c
//
// Description:
//    This is a simple app that demonstrates the usage of the 
//    IP_HDRINCL socket option. A raw socket is created of the
//    UDP protocol where we will build our own IP and UDP header
//    that we submit to sendto().
//
// Compile:
//    cl -o Iphdrinc Iphdrinc.c ws2_32.lib
//
// Command Line Parameters/Arguments:
//    iphdrinc [-fp:int] [-fi:str] [-tp:int] [-ti:str] [-n:int] 
//             [-m:str]
//
//           -fp:int   From (sender) port number
//           -fi:IP    From (sender) IP address
//           -tp:int   To (recipient) port number
//           -ti:IP    To (recipient) IP address
//           -n:int    Number of times to send message
//           -m:str    Message to send
//
#pragma pack(1)

#define WIN32_LEAN_AND_MEAN 

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

#define MAX_MESSAGE        4068
#define MAX_PACKET         4096
//
// Setup some default values 
//
#define DEFAULT_PORT       5150
#define DEFAULT_IP         "10.0.0.1"
#define DEFAULT_COUNT      5
#define DEFAULT_MESSAGE    "This is a test"

//
// Define the IP header. Make the version and length field one
// character since we can't declare two 4 bit fields without
// the compiler aligning them on at least a 1 byte boundary.
//
typedef struct ip_hdr
{
    unsigned char  ip_verlen;        // IP version & length
    unsigned char  ip_tos;           // IP type of service
    unsigned short ip_totallength;   // Total length
    unsigned short ip_id;            // Unique identifier 
    unsigned short ip_offset;        // Fragment offset field
    unsigned char  ip_ttl;           // Time to live
    unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
    unsigned short ip_checksum;      // IP checksum
    unsigned int   ip_srcaddr;       // Source address
    unsigned int   ip_destaddr;      // Destination address
} IP_HDR, *PIP_HDR, FAR* LPIP_HDR;
//
// Define the UDP header 
//
typedef struct udp_hdr
{
    unsigned short src_portno;       // Source port number
    unsigned short dst_portno;       // Destination port number
    unsigned short udp_length;       // UDP packet length
    unsigned short udp_checksum;     // UDP checksum (optional)
} UDP_HDR, *PUDP_HDR;

//
// Global variables
//
unsigned long  dwToIP,               // IP to send to
               dwFromIP;             // IP to send from (spoof)
unsigned short iToPort,              // Port to send to
               iFromPort;            // Port to send from (spoof)
DWORD          dwCount;              // Number of times to send
char           strMessage[MAX_MESSAGE]; // Message to send

//
// Function: usage:
//
// Description:
//    Print usage information and exit
//
void usage(char *progname)
{
    printf("usage: %s [-fp:int] [-fi:str] [-tp:int] [-ti:str]\
 [-n:int] [-m:str]\n", progname);
    printf("       -fp:int   From (sender) port number\n");
    printf("       -fi:IP    From (sender) IP address\n");
    printf("       -fp:int   To (recipient) port number\n");
    printf("       -fi:IP    To (recipient) IP address\n");
    printf("       -n:int    Number of times to read message\n");
    printf("       -m:str    Size of buffer to read\n\n");
    ExitProcess(1);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments and set some global flags to
//    indicate the actions to perform
//
void ValidateArgs(int argc, char **argv)
{
    int                i;

    iToPort = DEFAULT_PORT;
    iFromPort = DEFAULT_PORT;
    dwToIP = inet_addr(DEFAULT_IP);
    dwFromIP = inet_addr(DEFAULT_IP); 
    dwCount = DEFAULT_COUNT;
    strcpy(strMessage, DEFAULT_MESSAGE);

    for(i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'f':        // From address
                    switch (tolower(argv[i][2]))
                    {
                        case 'p':
                            if (strlen(argv[i]) > 4)
                                iFromPort = atoi(&argv[i][4]);
                            break;
                        case 'i':
                            if (strlen(argv[i]) > 4)
                                dwFromIP = inet_addr(&argv[i][4]);
                            break;
                        default:
                            usage(argv[0]);
                            break;
                    }    
                    break;
                case 't':        // To address 
                    switch (tolower(argv[i][2]))
                    {
                        case 'p':
                            if (strlen(argv[i]) > 4)
                                iToPort = atoi(&argv[i][4]);
                            break;
                        case 'i':
                            if (strlen(argv[i]) > 4)
                                dwToIP = inet_addr(&argv[i][4]);
                            break;
                        default:
                            usage(argv[0]);
                            break;
                    }    
                    break;
                case 'n':        // Number of times to send message
                    if (strlen(argv[i]) > 3)
                        dwCount = atol(&argv[i][3]);
                    break;
                case 'm':
                    if (strlen(argv[i]) > 3)
                        strcpy(strMessage, &argv[i][3]);
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
// Function: checksum
//
// Description:
//    This function calculates the 16-bit one's complement sum
//    for the supplied buffer
//
USHORT checksum(USHORT *buffer, int size)
{
    unsigned long cksum=0;

    while (size > 1)
    {
        cksum += *buffer++;
        size  -= sizeof(USHORT);   
    }
    if (size)
    {
        cksum += *(UCHAR*)buffer;   
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16); 

    return (USHORT)(~cksum); 
}

// 
// Function: main
//
// Description:
//    First parse command line arguments and load Winsock. Then 
//    create the raw socket and set the IP_HDRINCL option.
//    Following this, assemble the IP and UDP packet headers by
//    assigning the correct values and calculating the checksums.
//    Then fill in the data and send to its destination.
//
int main(int argc, char **argv)
{
    WSADATA            wsd;
    SOCKET             s;
    BOOL               bOpt;
    struct sockaddr_in remote;       // IP addressing structures
    IP_HDR             ipHdr;
    UDP_HDR            udpHdr;
    int                ret;
    DWORD              i;
    unsigned short     iTotalSize,   // Lots of sizes needed to fill
                       iUdpSize,     // the various headers with
                       iUdpChecksumSize,
                       iIPVersion,
                       iIPSize,
                       cksum = 0;
    char               buf[MAX_PACKET],
                      *ptr = NULL;
    IN_ADDR            addr;

    // Parse command line arguments and print them out
    //
    ValidateArgs(argc, argv);
    addr.S_un.S_addr = dwFromIP;
    printf("From IP: <%s>\n     Port: %d\n", inet_ntoa(addr),
        iFromPort);
    addr.S_un.S_addr = dwToIP;
    printf("To   IP: <%s>\n     Port: %d\n", inet_ntoa(addr),
        iToPort);
    printf("Message: [%s]\n", strMessage);
    printf("Count:   %d\n", dwCount);

    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    //  Creating a raw socket
    //
    s = WSASocket(AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, 0,0);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }

    // Enable the IP header include option 
    //
    bOpt = TRUE;
    ret = setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&bOpt, sizeof(bOpt));
    if (ret == SOCKET_ERROR)
    {
        printf("setsockopt(IP_HDRINCL) failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Initalize the IP header
    //
    iTotalSize = sizeof(ipHdr) + sizeof(udpHdr) + strlen(strMessage);

    iIPVersion = 4;
    iIPSize = sizeof(ipHdr) / sizeof(unsigned long);
    //
    // IP version goes in the high order 4 bits of ip_verlen. The
    // IP header length (in 32-bit words) goes in the lower 4 bits.
    //
    ipHdr.ip_verlen = (iIPVersion << 4) | iIPSize;
    ipHdr.ip_tos = 0;                         // IP type of service
    ipHdr.ip_totallength = htons(iTotalSize); // Total packet len
    ipHdr.ip_id = 0;                 // Unique identifier: set to 0
    ipHdr.ip_offset = 0;             // Fragment offset field
    ipHdr.ip_ttl = 128;              // Time to live
    ipHdr.ip_protocol = 0x11;        // Protocol(UDP) 
    ipHdr.ip_checksum = 0 ;          // IP checksum
    ipHdr.ip_srcaddr = dwFromIP;     // Source address
    ipHdr.ip_destaddr = dwToIP;      // Destination address
    //
    // Initalize the UDP header
    //
    iUdpSize = sizeof(udpHdr) + strlen(strMessage);

    udpHdr.src_portno = htons(iFromPort) ;
    udpHdr.dst_portno = htons(iToPort) ;
    udpHdr.udp_length = htons(iUdpSize) ;
    udpHdr.udp_checksum = 0 ;
    // 
    // Build the UDP pseudo-header for calculating the UDP checksum.
    // The pseudo-header consists of the 32-bit source IP address, 
    // the 32-bit destination IP address, a zero byte, the 8-bit
    // IP protocol field, the 16-bit UDP length, and the UDP
    // header itself along with its data (padded with a 0 if
    // the data is odd length).
    //
    iUdpChecksumSize = 0;
    ptr = buf;
    ZeroMemory(buf, MAX_PACKET);

    memcpy(ptr, &ipHdr.ip_srcaddr,  sizeof(ipHdr.ip_srcaddr));  
    ptr += sizeof(ipHdr.ip_srcaddr);
    iUdpChecksumSize += sizeof(ipHdr.ip_srcaddr);

    memcpy(ptr, &ipHdr.ip_destaddr, sizeof(ipHdr.ip_destaddr)); 
    ptr += sizeof(ipHdr.ip_destaddr);
    iUdpChecksumSize += sizeof(ipHdr.ip_destaddr);

    ptr++;
    iUdpChecksumSize += 1;

    memcpy(ptr, &ipHdr.ip_protocol, sizeof(ipHdr.ip_protocol)); 
    ptr += sizeof(ipHdr.ip_protocol);
    iUdpChecksumSize += sizeof(ipHdr.ip_protocol);

    memcpy(ptr, &udpHdr.udp_length, sizeof(udpHdr.udp_length)); 
    ptr += sizeof(udpHdr.udp_length);
    iUdpChecksumSize += sizeof(udpHdr.udp_length);
    
    memcpy(ptr, &udpHdr, sizeof(udpHdr)); 
    ptr += sizeof(udpHdr);
    iUdpChecksumSize += sizeof(udpHdr);

    for(i = 0; i < strlen(strMessage); i++, ptr++)
        *ptr = strMessage[i];
    iUdpChecksumSize += strlen(strMessage);

    cksum = checksum((USHORT *)buf, iUdpChecksumSize);
    udpHdr.udp_checksum = cksum;
    //
    // Now assemble the IP and UDP headers along with the data
    //  so we can send it
    //        
    ZeroMemory(buf, MAX_PACKET);
    ptr = buf;

    memcpy(ptr, &ipHdr, sizeof(ipHdr));   ptr += sizeof(ipHdr);
    memcpy(ptr, &udpHdr, sizeof(udpHdr)); ptr += sizeof(udpHdr);
    memcpy(ptr, strMessage, strlen(strMessage));

    // Apparently, this SOCKADDR_IN structure makes no difference.
    // Whatever we put as the destination IP addr in the IP header
    // is what goes. Specifying a different destination in remote
	 // will be ignored.
    //
    remote.sin_family = AF_INET;
    remote.sin_port = htons(iToPort);
    remote.sin_addr.s_addr = dwToIP;
   
    for(i = 0; i < dwCount; i++)
    {
        ret = sendto(s, buf, iTotalSize, 0, (SOCKADDR *)&remote, 
            sizeof(remote));
        if (ret == SOCKET_ERROR)
        {
            printf("sendto() failed: %d\n", WSAGetLastError());
            break;
        }
        else
            printf("sent %d bytes\n", ret);
    }
    closesocket(s) ;
    WSACleanup() ;

    return 0;
}
