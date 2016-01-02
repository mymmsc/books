// Module Name: Ircommon.h
//
// Description:
//    This header file simply contains prototypes for two basic
//    functions used by both client and server. The functions
//    themselves are in Ircommon.c
//
int senddata(SOCKET s, char *buf, int *len);
int recvdata(SOCKET s, char *buf, int *len);
