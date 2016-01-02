#ifndef _RCVALL_H_
#define _RCVALL_H_

#include <winsock2.h>
#include <windows.h>

#define MAX_IP_SIZE        65535
#define MIN_IP_HDR_SIZE       20

#define HI_WORD(byte)    (((byte) >> 4) & 0x0F)
#define LO_WORD(byte)    ((byte) & 0x0F)

extern char *szProto[];



void PrintRawBytes   (BYTE *ptr, DWORD len);
int  DecodeIGMPHeader(WSABUF *wsabuf, DWORD iphdrlen);
int  DecodeUDPHeader (WSABUF *wsabuf, DWORD iphdrlen);
int  DecodeTCPHeader (WSABUF *wsabuf, DWORD iphdrlenz);
int  DecodeIPHeader  (WSABUF *wasbuf, unsigned int srcaddr,
        unsigned short srcport, unsigned int destaddr, unsigned short destport);

#endif
