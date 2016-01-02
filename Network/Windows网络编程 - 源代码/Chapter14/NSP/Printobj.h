#ifndef _PRINT_OBJ_H_
#define _PRINT_OBJ_H_

extern "C" 
{

void PrintBytes(BYTE *bytes, int count);
void PrintGuid(GUID *guid);

void PrintServiceClass(WSASERVICECLASSINFO *sc);
void PrintQuery(WSAQUERYSET *qs);

}

#endif
