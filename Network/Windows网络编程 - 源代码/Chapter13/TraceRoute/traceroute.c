// Module: Traceroute.c
//
// Description:
//    This sample is fairly similar to the ping.c sample. It 
//    creates ICMP packets and sends them to the intended 
//    recipient. The time-to-live value for the ICMP packet 
//    is initially 1 and is incremented by one everytime
//    an ICMP response is received. This way we can look
//    at the source IP address of these replies to find
//    the route to the intended recipient. Once we receive
//    an ICMP port unreachable message we know that we have
//    reached the inteded recipient and we can stop.
//
// Compile:
//    cl -o Traceroute Traceroute.c ws_32.lib /Zp1
//
// Command Line Options/Parameters
//    traceroute.exe hostname [max-hops]
//    hostname         Hostname or IP dotted decimal address of
//                      the endpoint.
//    max-hops         Maximum number of hops (routers) to 
//                      traverse to the endpoint before quitting.
//
#pragma pack(4)

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//
// Defines for ICMP message types
//
#define ICMP_ECHOREPLY      0
#define ICMP_DESTUNREACH    3
#define ICMP_SRCQUENCH      4
#define ICMP_REDIRECT       5
#define ICMP_ECHO           8
#define ICMP_TIMEOUT       11
#define ICMP_PARMERR       12

#define MAX_HOPS           30

#define ICMP_MIN 8    // Minimum 8 byte icmp packet (just header)

//
// IP Header
//
typedef struct iphdr 
{
    unsigned int   h_len:4;        // Length of the header
    unsigned int   version:4;      // Version of IP
    unsigned char  tos;            // Type of service
    unsigned short total_len;      // Total length of the packet
    unsigned short ident;          // Unique identifier
    unsigned short frag_and_flags; // Flags
    unsigned char  ttl;            // Time to live
    unsigned char  proto;          // Protocol (TCP, UDP etc)
    unsigned short checksum;       // IP checksum
    unsigned int   sourceIP;       // Source IP
    unsigned int   destIP;         // Destination IP
} IpHeader;

//
// ICMP header
//
typedef struct _ihdr 
{
    BYTE   i_type;               // ICMP message type
    BYTE   i_code;               // Sub code
    USHORT i_cksum;              
    USHORT i_id;                 // Unique id
    USHORT i_seq;                // Sequence number
    // This is not the std header, but we reserve space for time
    ULONG timestamp;
} IcmpHeader;

#define DEF_PACKET_SIZE         32
#define MAX_PACKET            1024

//
// Function: usage
//
// Description:
//    Print usage information
//
void usage(char *progname)
{
    printf("usage: %s host-name [max-hops]\n", progname);
    ExitProcess(-1);
}

//
// Function: set_ttl
//
// Description:
//    Set the time to live parameter on the socket. This controls 
//    how far the packet will be forwared before a "timeout" 
//    response will be sent back to us. This way we can see all 
//    the hops along the way to the destination.
//
int set_ttl(SOCKET s, int nTimeToLive)
{
    int     nRet;
    
    nRet = setsockopt(s, IPPROTO_IP, IP_TTL, (LPSTR)&nTimeToLive,
                sizeof(int));
    if (nRet == SOCKET_ERROR)
    {
        printf("setsockopt(IP_TTL) failed: %d\n", 
            WSAGetLastError());
        return 0;
    }
    return 1;
}

//
// Function: decode_resp
//
// Description:
//    The response is an IP packet. We must decode the IP header 
//    to locate the ICMP data.
//
int decode_resp(char *buf, int bytes, SOCKADDR_IN *from, int ttl)
{
    IpHeader       *iphdr = NULL;
    IcmpHeader     *icmphdr = NULL;
    unsigned short  iphdrlen;
    struct hostent *lpHostent = NULL;
    struct in_addr  inaddr = from->sin_addr;

    iphdr = (IpHeader *)buf;
    // Number of 32-bit words * 4 = bytes
	iphdrlen = iphdr->h_len * 4; 

    if (bytes < iphdrlen + ICMP_MIN) 
        printf("Too few bytes from %s\n",
            inet_ntoa(from->sin_addr));

    icmphdr = (IcmpHeader*)(buf + iphdrlen);

    switch (icmphdr->i_type)
    {
        case ICMP_ECHOREPLY:     // Response from destination
            lpHostent = gethostbyaddr((const char *)&from->sin_addr, 
                AF_INET, sizeof(struct in_addr));
            if (lpHostent != NULL)
                printf("%2d  %s (%s)\n", ttl, lpHostent->h_name, 
                    inet_ntoa(inaddr));
            return 1;
            break;
        case ICMP_TIMEOUT:      // Response from router along the way
            printf("%2d  %s\n", ttl, inet_ntoa(inaddr));
            return 0;
            break;
        case ICMP_DESTUNREACH:  // Can't reach the destination at all
            printf("%2d  %s  reports: Host is unreachable\n", ttl, 
                inet_ntoa(inaddr));
            return 1;
            break;
        default:
            printf("non-echo type %d recvd\n", icmphdr->i_type);
            return 1;
            break;
    }
    return 0;
}

//
// Function: checksum
//
// Description:
//    This function calculates the checksum for the ICMP header 
//    which is a necessary field since we are building packets by 
//    hand. Normally, the TCP layer handles all this when you do 
//    sockets, but ICMP is at a somewhat lower level.
//
USHORT checksum(USHORT *buffer, int size) 
{
    unsigned long cksum=0;

    while(size > 1) 
    {
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }
    if(size )
        cksum += *(UCHAR*)buffer;
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    return (USHORT)(~cksum);
}

//
// Function: fill_icmp_data
//
// Description:
//    Helper function to fill in various stuff in our ICMP request.
//
void fill_icmp_data(char * icmp_data, int datasize)
{
    IcmpHeader *icmp_hdr;
    char       *datapart;

    icmp_hdr = (IcmpHeader*)icmp_data;

    icmp_hdr->i_type = ICMP_ECHO;
    icmp_hdr->i_code = 0;
    icmp_hdr->i_id   = (USHORT)GetCurrentProcessId();
    icmp_hdr->i_cksum = 0;
    icmp_hdr->i_seq = 0;
  
    datapart = icmp_data + sizeof(IcmpHeader);
    //
    // Place some junk in the buffer. Don't care about the data...
    //
    memset(datapart,'E', datasize - sizeof(IcmpHeader));
}

//
// Function: main
// 
int main(int argc, char **argv)
{
    WSADATA      wsd;
    SOCKET       sockRaw;
    HOSTENT     *hp = NULL;
    SOCKADDR_IN  dest,
                 from;
    int          ret,
                 datasize,
                 fromlen = sizeof(from),
                 timeout,
                 done = 0,
                 maxhops,
                 ttl = 1;
    char        *icmp_data,
                *recvbuf;
    BOOL         bOpt;
    USHORT       seq_no = 0;

    // Initialize the Winsock2 DLL
    //
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    if (argc < 2) 
        usage(argv[0]);
    if (argc == 3)
        maxhops = atoi(argv[2]);
    else
        maxhops = MAX_HOPS;
    //
    // Create a raw socket that will be used to send the ICMP 
    // packets to the remote host you want to ping
    //
    sockRaw = WSASocket (AF_INET, SOCK_RAW, IPPROTO_ICMP,
                         NULL, 0,WSA_FLAG_OVERLAPPED);
    if (sockRaw == INVALID_SOCKET) 
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        ExitProcess(-1);
    }
    //
    // Set the receive and send timeout values to a second
    //
    timeout = 1000;
    ret = setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, 
            (char *)&timeout, sizeof(timeout));
    if (ret == SOCKET_ERROR)
    {
        printf("setsockopt(SO_RCVTIMEO) failed: %d\n", 
            WSAGetLastError());
        return -1;
    }
    timeout = 1000;
    ret = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, 
        (char *)&timeout, sizeof(timeout));
    if (ret == SOCKET_ERROR)
    {
        printf("setsockopt(SO_SNDTIMEO) failed: %d\n", 
            WSAGetLastError());
        return -1;
    }

    ZeroMemory(&dest, sizeof(dest));
    //
    // We need to resolve the host's ip address.  We check to see 
    // if it is an actual Internet name versus an dotted decimal 
    // IP address string.
    //
    dest.sin_family = AF_INET;
    if ((dest.sin_addr.s_addr = inet_addr(argv[1])) == INADDR_NONE)
    {
        hp = gethostbyname(argv[1]);
        if (hp)
            memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
        else
        {
            printf("Unable to resolve %s\n",argv[1]);
            ExitProcess(-1);
        }
    }
    //
    // Set the data size to the default packet size.
    // We don't care about the data since this is just traceroute/ping
    //
    datasize = DEF_PACKET_SIZE;
        
    datasize += sizeof(IcmpHeader);  
    //
    // Allocate the sending and receiving buffers for ICMP packets
    //
    icmp_data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PACKET);
    recvbuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PACKET);

    if ((!icmp_data) || (!recvbuf))
    {
        printf("HeapAlloc() failed %d\n", GetLastError());
        return -1;
    }
    // Set the socket to bypass the standard routing mechanisms 
    //  i.e. use the local protocol stack to the appropriate network
    //       interface
    //
    bOpt = TRUE;
    if (setsockopt(sockRaw, SOL_SOCKET, SO_DONTROUTE, (char *)&bOpt, 
            sizeof(BOOL)) == SOCKET_ERROR) 
        printf("setsockopt(SO_DONTROUTE) failed: %d\n", 
            WSAGetLastError());

    //  
    // Here we are creating and filling in an ICMP header that is the 
    // core of trace route.
    //
    memset(icmp_data, 0, MAX_PACKET);
    fill_icmp_data(icmp_data, datasize);

    printf("\nTracing route to %s over a maximum of %d hops:\n\n", 
        argv[1], maxhops);

    for(ttl = 1; ((ttl < maxhops) && (!done)); ttl++)
    {
        int bwrote;

        // Set the time to live option on the socket
        //
        set_ttl(sockRaw, ttl);

        //
        // Fill in some more data in the ICMP header
        //
        ((IcmpHeader*)icmp_data)->i_cksum = 0;
        ((IcmpHeader*)icmp_data)->timestamp = GetTickCount();

        ((IcmpHeader*)icmp_data)->i_seq = seq_no++;
        ((IcmpHeader*)icmp_data)->i_cksum = checksum((USHORT*)icmp_data, 
            datasize);
        //
        // Send the ICMP packet to the destination
        //
        bwrote = sendto(sockRaw, icmp_data, datasize, 0, 
                    (SOCKADDR *)&dest, sizeof(dest));
        if (bwrote == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT) 
            {
                printf("%2d  Send request timed out.\n", ttl);
                continue;
            }
            printf("sendto() failed: %d\n", WSAGetLastError());
            return -1;
        }
        // Read a packet back from the destination or a router along 
        // the way.
        //
        ret = recvfrom(sockRaw, recvbuf, MAX_PACKET, 0, 
            (struct sockaddr*)&from, &fromlen);
        if (ret == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT) 
            {
                printf("%2d  Receive Request timed out.\n", ttl);
                continue;
            }
            printf("recvfrom() failed: %d\n", WSAGetLastError());
            return -1;
        }
        //
        // Decode the response to see if the ICMP response is from a 
        // router along the way or whether it has reached the destination.
        //
        done = decode_resp(recvbuf, ret, &from, ttl);
        Sleep(1000);
    }
    HeapFree(GetProcessHeap(), 0, recvbuf);
    HeapFree(GetProcessHeap(), 0, icmp_data);

    return 0;
}
