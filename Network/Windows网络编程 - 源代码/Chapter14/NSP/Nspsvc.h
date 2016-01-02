#ifndef _NSPSVC_H_
#define _NSPSVC_H_

#define MARSHALL_BUFFER_SZ        4096

#define MYNSP_REGISTER_CLASS         0
#define MYNSP_DEREGISTER_CLASS       1
#define MYNSP_LOOKUP_CLASS           2
#define MYNSP_REGISTER_SERVICE       3
#define MYNSP_DEREGISTER_SERVICE     4
#define MYNSP_LOOKUP_SERVICE_BEGIN   5
#define MYNSP_LOOKUP_SERVICE_NEXT    6
#define MYNSP_LOOKUP_SERVICE_END     7

extern "C"
{
int MarshallServiceClassInfo(WSASERVICECLASSINFO *sc, char *buf, int *size);

int DeMarshallServiceClassInfo(WSASERVICECLASSINFO *sc, char *buf);

int MarshallServiceInfo(WSAQUERYSET *sc, char *buf, int *size);

int DeMarshallServiceInfo(WSAQUERYSET *sc, char *buf);

int readdata(SOCKET s, char *buffer, int numwrite, int *byteswritten);

int writedata(SOCKET s, char *buffer, int numwrite, int *byteswritten);
}

#endif
