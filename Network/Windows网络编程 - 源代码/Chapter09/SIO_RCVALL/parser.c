// Module: parser.c
//
// Description:
//    This file is the companion to rcvall.c and contains
//    the parser routines for printing out IP, UDP, TCP,
//    ICMP, and IGMP packets.
//
// Compile:
//    cl /c parser.c
//
// Command Line Arguments/Parameters
//    None
//
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

extern BOOL bFilter;
//
// A list of protocol types in the IP protocol header
//
char *szProto[] = {"Reserved",     //  0
                   "ICMP",         //  1
                   "IGMP",         //  2
                   "GGP",          //  3
                   "IP",           //  4
                   "ST",           //  5
                   "TCP",          //  6
                   "UCL",          //  7
                   "EGP",          //  8
                   "IGP",          //  9
                   "BBN-RCC-MON",  // 10
                   "NVP-II",       // 11
                   "PUP",          // 12
                   "ARGUS",        // 13
                   "EMCON",        // 14
                   "XNET",         // 15
                   "CHAOS",        // 16
                   "UDP",          // 17
                   "MUX",          // 18
                   "DCN-MEAS",     // 19
                   "HMP",          // 20
                   "PRM",          // 21
                   "XNS-IDP",      // 22
                   "TRUNK-1",      // 23
                   "TRUNK-2",      // 24
                   "LEAF-1",       // 25
                   "LEAF-2",       // 26
                   "RDP",          // 27
                   "IRTP",         // 28
                   "ISO-TP4",      // 29
                   "NETBLT",       // 30
                   "MFE-NSP",      // 31
                   "MERIT-INP",    // 32
                   "SEP",          // 33
                   "3PC",          // 34
                   "IDPR",         // 35
                   "XTP",          // 36
                   "DDP",          // 37
                   "IDPR-CMTP",    // 38
                   "TP++",         // 39
                   "IL",           // 40
                   "SIP",          // 41
                   "SDRP",         // 42
                   "SIP-SR",       // 43
                   "SIP-FRAG",     // 44
                   "IDRP",         // 45
                   "RSVP",         // 46
                   "GRE",          // 47
                   "MHRP",         // 48
                   "BNA",          // 49
                   "SIPP-ESP",     // 50
                   "SIPP-AH",      // 51
                   "I-NLSP",       // 52
                   "SWIPE",        // 53
                   "NHRP",         // 54
                   "unassigned",   // 55
                   "unassigned",   // 56
                   "unassigned",   // 57
                   "unassigned",   // 58
                   "unassigned",   // 59
                   "unassigned",   // 60
                   "any host internal protocol",  // 61
                   "CFTP",         // 62
                   "any local network",           // 63
                   "SAT-EXPAK",    // 64
                   "KRYPTOLAN",    // 65
                   "RVD",          // 66
                   "IPPC",         // 67
                   "any distributed file system", // 68
                   "SAT-MON",    // 69
                   "VISA",       // 70
                   "IPCV",       // 71
                   "CPNX",       // 72
                   "CPHB",       // 73
                   "WSN",        // 74
                   "PVP",        // 75
                   "BR-SAT-MON", // 76
                   "SUN-ND",     // 77
                   "WB-MON",     // 78
                   "WB-EXPAK",   // 79
                   "ISO-IP",     // 80
                   "VMTP",       // 81
                   "SECURE-VMTP",// 82
                   "VINES",      // 83
                   "TTP",        // 84
                   "NSFNET-IGP", // 85
                   "DGP",        // 86
                   "TCF",        // 87
                   "IGRP",       // 88
                   "OSPFIGP",    // 89
                   "Sprite-RPC", // 90
                   "LARP",       // 91
                   "MTP",        // 92
                   "AX.25",      // 93
                   "IPIP",       // 94
                   "MICP",       // 95
                   "SCC-SP",     // 96
                   "ETHERIP",    // 97
                   "ENCAP",      // 98
                   "any private encryption scheme",    // 98
                   "GMTP"        // 99
                  };
//
// The types of IGMP messages
//
char *szIgmpType[] = {"",
                      "Host Membership Query",
                      "HOst Membership Report",
                      "",
                      "",
                      "",
                      "Version 2 Membership Report",
                      "Leave Group"
                     };

//
// Function: PrintRawBytes
//
// Description:
//    This function simply prints out a series of bytes
//    as hexadecimal digits.
//
void PrintRawBytes(BYTE *ptr, DWORD len)
{
    int        i;

    while (len > 0)
    {
        for(i=0; i < 20 ;i++)
        {
            printf("%x%x ", HI_WORD(*ptr), LO_WORD(*ptr));
            len--;
            ptr++;
            if (len == 0)
                break;
        }
        printf("\n");
    }
}

//
// Function: DecodeIGMPHeader
//
// Description:
//    This function takes a pointer to a buffer containing
//    an IGMP packet and prints it out in a readable form.
//
int DecodeIGMPHeader(WSABUF *wsabuf, DWORD iphdrlen)
{
    BYTE          *hdr = (BYTE *)(wsabuf->buf + iphdrlen);
    unsigned short chksum,
                   version,
                   type,
                   maxresptime;
    SOCKADDR_IN    addr;

    version = HI_WORD(*hdr);
    type    = LO_WORD(*hdr);
    hdr++;

    maxresptime = *hdr;
    hdr++;

    memcpy(&chksum, hdr, 2);
    chksum = ntohs(chksum);
    hdr += 2;

    memcpy(&(addr.sin_addr.s_addr), hdr, 4);

    printf("   IGMP HEADER:\n");
    if ((type == 1) || (type == 2))
        version = 1;
    else
        version = 2;
    printf("   IGMP Version = %d\n", version);
    printf("   IGMP Type = %s\n", szIgmpType[type]);
    if (version == 2)
        printf("   Max Resp Time = %d\n", maxresptime);
    printf("   IGMP Grp Addr = %s\n", inet_ntoa(addr.sin_addr));
    //ExitProcess(0);

    return 0;
}

// 
// Function: DecodeUDPHeader
//
// Description:
//    This function takes a buffer which points to a UDP
//    header and prints it out in a readable form.
//
int DecodeUDPHeader(WSABUF *wsabuf, DWORD iphdrlen)
{
    BYTE          *hdr = (BYTE *)(wsabuf->buf + iphdrlen);
    unsigned short shortval,
                   udp_src_port,
                   udp_dest_port,
                   udp_len,
                   udp_chksum;
    
    memcpy(&shortval, hdr, 2);
    udp_src_port = ntohs(shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    udp_dest_port = ntohs(shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    udp_len = ntohs(shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    udp_chksum = ntohs(shortval);

    printf("   UDP HEADER\n");
    printf("   Source Port: %-05d       | Dest Port: %-05d\n",
        udp_src_port, udp_dest_port);
    printf("       UDP Len: %-05d       |    ChkSum: 0x%08x\n",
        udp_len, udp_chksum);
    return 0;
}

//
// Function: DecodeTCPHeader
//
// Description:
//    This function takes a buffer pointing to a TCP header
//    and prints it out in a readable form.
//
int DecodeTCPHeader(WSABUF *wsabuf, DWORD iphdrlen)
{
    BYTE           *hdr = (BYTE *)(wsabuf->buf + iphdrlen);
    unsigned short shortval;
    unsigned int   longval;

    printf("   TCP HEADER\n");
    memcpy(&shortval, hdr, 2);
    shortval = ntohs(shortval);
    printf("   Src Port   : %d\n", shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    shortval = ntohs(shortval);
    printf("   Dest Port  : %d\n", shortval);
    hdr += 2;

    memcpy(&longval, hdr, 4);
    longval = ntohl(longval);
    printf("   Seq Num    : %d\n", longval);
    hdr += 4;

    memcpy(&longval, hdr, 4);
    longval = ntohl(longval);
    printf("   ACK Num    : %d\n", longval);
    hdr += 4;

    printf("   Header Len : %d (bytes %d)\n", HI_WORD(*hdr), 
        (HI_WORD(*hdr) * 4));

    memcpy(&shortval, hdr, 2);
    shortval = ntohs(shortval) & 0x3F;
    printf("   Flags      : ");
    if (shortval & 0x20)
        printf("URG ");
    if (shortval & 0x10)
        printf("ACK ");
    if (shortval & 0x08)
        printf("PSH ");
    if (shortval & 0x04)
        printf("RST ");
    if (shortval & 0x02)
        printf("SYN ");
    if (shortval & 0x01)
        printf("FIN ");
    printf("\n");
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    shortval = ntohs(shortval);
    printf("   Window size: %d\n", shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    shortval = ntohs(shortval);
    printf("   TCP Chksum : %d\n", shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    shortval = ntohs(shortval);
    printf("   Urgent ptr : %d\n", shortval);

    return 0;
}

//
// Function: DecodeIPHeader
//
// Description:
//    This function takes a pointer to an IP header and prints
//    it out in a readable form.
//
int DecodeIPHeader(WSABUF *wsabuf, unsigned int srcip, unsigned short srcport,
        unsigned int destip, unsigned short destport)
{
    BYTE          *hdr = (BYTE *)wsabuf->buf,
                  *nexthdr = NULL;
    unsigned short shortval;
    SOCKADDR_IN    srcaddr,
                   destaddr;
    
    unsigned short ip_version,
                   ip_hdr_len,
                   ip_tos,
                   ip_total_len,
                   ip_id,
                   ip_flags,
                   ip_ttl,
                   ip_frag_offset,
                   ip_proto,
                   ip_hdr_chksum,
                   ip_src_port,
                   ip_dest_port;
    unsigned int   ip_src,
                   ip_dest;
    BOOL           bPrint = TRUE;


    ip_version = HI_WORD(*hdr);
    ip_hdr_len = LO_WORD(*hdr) * 4;
    nexthdr = (BYTE *)(wsabuf->buf + ip_hdr_len);
    hdr++;

    ip_tos = *hdr;
    hdr++;

    memcpy(&shortval, hdr, 2);
    ip_total_len = ntohs(shortval);
    hdr += 2;

    memcpy(&shortval, hdr, 2);
    ip_id = ntohs(shortval);
    hdr += 2;

    ip_flags = ((*hdr) >> 5);

    memcpy(&shortval, hdr, 2);
    ip_frag_offset = ((ntohs(shortval)) & 0x1FFF);
    hdr+=2;

    ip_ttl = *hdr;
    hdr++;

    ip_proto = *hdr;
    hdr++;

    memcpy(&shortval, hdr, 2);
    ip_hdr_chksum = ntohs(shortval);
    hdr += 2;

    memcpy(&srcaddr.sin_addr.s_addr, hdr, 4);
    ip_src = ntohl(srcaddr.sin_addr.s_addr);
    hdr += 4;

    memcpy(&destaddr.sin_addr.s_addr, hdr, 4);
    ip_dest = ntohl(destaddr.sin_addr.s_addr);
    hdr += 4;
    //
    // If packet is UDP, TCP, or IGMP read ahead and
    //  get the port values.
    //
    if (((ip_proto == 2) ||
         (ip_proto == 6) ||
         (ip_proto == 17)) &&
         bFilter)
    {
        memcpy(&ip_src_port, nexthdr, 2);
        ip_src_port = ntohs(ip_src_port);
        memcpy(&ip_dest_port, nexthdr+2, 2);
        ip_dest_port = ntohs(ip_dest_port);

        if ((srcip == ip_src) ||
            (srcport == ip_src_port) ||
            (destip == ip_dest) ||
            (destport == ip_dest_port))
        {
            bPrint = TRUE;
        }
        else
        {
            bPrint = FALSE;
        }
        
    }
    else if (bFilter)
        bPrint = FALSE;

    // Print IP Hdr
    //
    if (bPrint)
    {
        printf("IP HEADER\n");
        printf("   IP Version: %-10d  |  IP Header Len: %2d bytes   |   IP TOS: %X%X (hex)\n",
            ip_version, ip_hdr_len, HI_WORD(ip_tos), LO_WORD(ip_tos));
        printf(" IP Total Len: %-05d bytes | Identification: 0x%08X | IP Flags: %X  (hex)\n",
            ip_total_len, ip_id, ip_flags);
        printf("  Frag Offset: 0x%08X  |            TTL: %-10d | Protocol: %-10s \n",
            ip_frag_offset, ip_ttl, szProto[ip_proto]);
        printf(" Hdr Checksum: 0x%08X\n", ip_hdr_chksum);
        printf("     Src Addr: %-15s\n", inet_ntoa(srcaddr.sin_addr));
        printf("    Dest Addr: %-15s\n", inet_ntoa(destaddr.sin_addr));
    }
    else
        return ip_hdr_len;

    switch (ip_proto)
    {
        case 2:        // IGMP
            DecodeIGMPHeader(wsabuf, ip_hdr_len);
            break;
        case 6:        // TCP
            DecodeTCPHeader(wsabuf, ip_hdr_len);
            break;
        case 17:       // UDP
            DecodeUDPHeader(wsabuf, ip_hdr_len);
            break;
        default:
            printf("   No decoder installed for protocol\n");
            break;
    }
    printf("\n");

    return ip_hdr_len;
}
