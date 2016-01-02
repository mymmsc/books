// Module: printobj.cpp
//
// Description:
//    This file contains simple support routines to print
//    out the contents of various structures.
//
#include <winsock2.h>
#include <windows.h>

#include "printobj.h"
#include "nspsvc.h"

#include <stdio.h>
#include <stdlib.h>

//
// Function: PrintBytes
//
// Description:
//    Print a number of bytes as hexadecimal characters.
//
void PrintBytes(BYTE *bytes, int count)
{
    int j=0;

    printf("\n\n");
    for(int i=0; i < count ;i++)
    {
        printf("%02x ", bytes[i]);
        j++;
        if (j == 20)
        {
            printf("\n");
            j = 0;
        }
    }
    printf("\n\n");
}

//
// Function: PrintGuid
//
// Description:
//    Print a GUID by printing its member fields as
//    hexadecimal digits.
//
void PrintGuid(GUID *guid)
{
    printf("%x %x %x %x %x %x %x %x %x %x %x\n",
        guid->Data1, guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

//
// Function: PrintServiceClass
//
// Description:
//    This function prints a WSASERVICECLASSINFO structure.
//    If lpClassInfos is not null we don't bother printing out the
//    sub structure.
//
void PrintServiceClass(WSASERVICECLASSINFO *sc)
{
    if (sc == NULL)
    {
        printf("WSASERVICECLASSINFO is NULL!\n");
        return;
    }
    if (sc->lpServiceClassId)
    {
        printf(".lpServiceClassId     = "); 
        PrintGuid(sc->lpServiceClassId);
    }
    else
        printf(".lpServiceClassID     = NULL\n");
    if (sc->lpszServiceClassName)
        printf(".lpszServiceClassName = '%S'\n", sc->lpszServiceClassName);
    else
        printf(".lpszServiceClassName = NULL\n");
    printf(".dwCount              = %d\n", sc->dwCount);
    if (sc->lpClassInfos)
        printf(".lpClassInfos         = ???\n");
    else
        printf(".lpClassInfos         = NULL\n");
    return;
}

//
// Function: PrintQuery
//
// Description:
//    This function prints a WSAQUERYSET structure.
//    Again, we don't print out the sub structures.
//
void PrintQuery(WSAQUERYSET *qs)
{
    printf(".dwSize                  = %d\n", qs->dwSize);
    if (qs->lpszServiceInstanceName)
        printf(".lpszServiceInstanceName = %S\n", qs->lpszServiceInstanceName);
    else
        printf(".lpszServiceInstanceName = NULL\n");
    printf(".lpServiceClassId        = "); PrintGuid(qs->lpServiceClassId);
    if (qs->lpVersion)
        printf(".lpVersion               = ???\n");
    else
        printf(".lpVersion               = NULL\n");
    if (qs->lpszComment) 
        printf(".lpszComment             = %S\n", qs->lpszComment);
    else
        printf(".lpszComment             = NULL\n");
    printf(".dwNameSpace             = %d\n", qs->dwNameSpace);
    printf(".lpNSProviderId          = "); PrintGuid(qs->lpNSProviderId);
    if (qs->lpszContext)
        printf(".lpszContext             = %S\n", qs->lpszContext);
    else
        printf(".lpszContext             = NULL\n");
    printf(".dwNumberOfProtocols     = %d\n", qs->dwNumberOfProtocols);
    if (qs->lpafpProtocols)
        printf(".lpafpProtocols          = ???\n");
    else
        printf(".lpafpProtocols          = NULL\n");
    if (qs->lpszQueryString)
        printf(".lpszQueryString         = %S\n", qs->lpszQueryString);
    else
        printf(".lpszQueryString         = NULL\n");
    printf(".dwNumberOfCsAddrs       = %d \n", qs->dwNumberOfCsAddrs);
    if (qs->lpcsaBuffer)
        printf(".lpcsaBuffer             = ???\n");
    else
        printf(".lpcsaBuffer             = NULL\n");
    printf(".dwOutputFlags           = %d\n", qs->dwOutputFlags);
	if (qs->lpBlob)
	    printf(".lpBlob				     = ???\n");
	else
	    printf(".lpBlob                  = NULL\n");
}
