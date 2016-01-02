// Module: mynsp.cpp
//
// Description:
//    This file contains the implemenation of an installable name
//    space provider. A name space provider is a DLL which provides
//    entry points for the Winsock 2 RNR functions. This DLL only 
//    provides a starting point for our namespace. We need some 
//    method to persist the information registered by the user.
//    We do this by running a simple Winsock app on the local
//    machine which waits for connections from this DLL. Each
//    routine defined here will connect to this service, send
//    an action command defined what we are to do, and send
//    some parameters. The namespace service will maintain each
//    registered service as well as service classes and queries.
//
//    This file contains only the DLL routines. It does not 
//    provide the mechanism for intalling the namespace on
//    the local computer. See nspinstall.c for namespace
//    installation.
//
// Compile:
//
// Command Line Arguments/Parameters
//    None.
//
#define UNICODE
#define _UNICODE

#include <winsock2.h>
#include <ws2spi.h>
#include <windows.h>

#include "nspsvc.h"

#include <stdio.h>

#include "mynsp.h"

#include "printobj.h"

//
// Function: MyNspConnect
//
// Description:
//    This is a helper function which simply establishes a conneection
//    to the our local MyNsp service in order to perform the desired
//    RNR operation. If successful, a SOCKET handle is returned; 
//    otherwise, SOCKET_ERROR is returned.
//
SOCKET WSAAPI MyNspConnect()
{
    SOCKET        s;
    SOCKADDR_IN   service;
    int           ret;

    // Create a TCP socket, a production quality RNR service would
    // most likely use a faster, less expensive protocol such as
    // UDP.
    //
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        printf("MyNspConnect: socket() failed: %d\n",
            WSAGetLastError());
        WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
    }
    // Setup the address to connect to
    //
    service.sin_family = AF_INET;
    service.sin_port   = htons(5150);
    service.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(s, (SOCKADDR *)&service, sizeof(service));
    if (ret == SOCKET_ERROR)
    {
        printf("MyNspConnect: connect() failed: %d\n", 
            WSAGetLastError());
        WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
    }
    return s;
}

//
// Function: DllMain
//
// Description:
//    This function is the main entry point for our DLL. If we needed
//    to maintain any per DLL data, we would allocate it here.
//
BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            break;
    	case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    	case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

//
// Function: NSPLookupServiceBegin
//
// Description:
//    This function maps to the Winsock call WSALookupServiceBegin.
//    Within this call we connect to our namespace service and 
//    transmit the query parameters (lpqsRestrictions) and a unique
//    handle for this query is returned. Subsequent calls only need
//    to refer to this query by the handle as opposed to specifying
//    the query parameters each time.
//
int WSAAPI NSPLookupServiceBegin(
    LPGUID lpProviderId,
    LPWSAQUERYSETW lpqsRestrictions,
    LPWSASERVICECLASSINFOW lpServiceClassInfo,
    DWORD dwControlFlags,   
    LPHANDLE lphLookup)
{
    SOCKET      s;
    int         ret,
                retcode,
                nLeft,
                bytesread,
                byteswritten;
    BYTE        action;
    char        databuf[MARSHALL_BUFFER_SZ];

    // Set the action and connect to the service
    //
    action = MYNSP_LOOKUP_SERVICE_BEGIN;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR;

    writedata(s, (char *)&action, sizeof(action), &byteswritten);
    //
    // Marshal the query parameters into a contiguous buffer
    //
    ret = MarshallServiceInfo(lpqsRestrictions, databuf, &nLeft);
    //
    // Write the query parameters and read the return code (handle)
    //
    writedata(s, (char *)&nLeft, sizeof(nLeft), &byteswritten);
    writedata(s, databuf, nLeft, &byteswritten);
	
    bytesread = sizeof(retcode);
    readdata(s, (char *)&retcode, sizeof(retcode), &bytesread);
   
    if (retcode == MYNSP_ERROR)
    {
        closesocket(s);
        WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
    }
    *lphLookup = (HANDLE)retcode;
    closesocket(s);

    return NO_ERROR;
}

//
// Function: NSPLookupServiceNext
//
// Description:
//    This function maps to the Winsock call WSALookupServiceNext.
//    This routine takes a handle to a previously defined query and
//    attempts to locate a service matching the criteria defined by
//    the query. If so, that instance is returned in the lpqsResults
//    parameter. This function accomplishes this by connecting to 
//    our service and writing the query handle. The service maintains
//    a state for the handle and begins searching for matching 
//    services. If found, the WSAQUERYSET defining the service is
//    marshalled and written back at which point it is demarshalled
//    into the lpqsResults buffer.
//
int WSAAPI NSPLookupServiceNext(  
    HANDLE hLookup,
    DWORD dwControlFlags,
    LPDWORD lpdwBufferLength,
    LPWSAQUERYSET lpqsResults)
{

    SOCKET      s;
    int         bytesread,
                byteswritten,
                bytes2follow;
    BYTE        action;
    char        buf[MARSHALL_BUFFER_SZ];

    // Set the action and connect to the service
    //
    action = MYNSP_LOOKUP_SERVICE_NEXT;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR;

    writedata(s, (char *)&action, sizeof(action), &byteswritten);
    //
    // Write the handle so the service knows which query we are
    //  executing
    //
    writedata(s, (char *)&hLookup, sizeof(hLookup), &byteswritten);
    //
    // Read the number of bytes to follow. If this value is 0 or
    //  -1 then it means an error occured
    // 
    bytesread = sizeof(bytes2follow);
    readdata(s, (char *)&bytes2follow, sizeof(bytes2follow), &bytesread);
    
    if (bytes2follow == -1)         // Service not found
    {
        closesocket(s);
        WSASetLastError(WSASERVICE_NOT_FOUND);
        return SOCKET_ERROR;
    }
    if (bytes2follow == 0)          // No more data
    {
        closesocket(s);
        WSASetLastError(WSA_E_NO_MORE);
        return SOCKET_ERROR;
    }
    // Make sure the user supplied buffer is large enough
    //
    if ((DWORD)bytes2follow > *lpdwBufferLength)
    {		
        *lpdwBufferLength = bytes2follow;
        closesocket(s);
        WSASetLastError(WSAEFAULT);
        return SOCKET_ERROR;
    }
    readdata(s, buf, MARSHALL_BUFFER_SZ, &bytes2follow);

    DeMarshallServiceInfo(lpqsResults, buf);

    *lpdwBufferLength = bytesread;

    closesocket(s);

    return NO_ERROR;
}

//
// Function: NSPLookupServiceEnd
//
// Description:
//    This function maps to the Winsock call WSALookupServiceEnd.
//    Once the user process has finished is query (usually indicated
//    when WSALookupServiceNext returns the error WSA_E_NO_MORE) a
//    call to this function is made to release any allocated
//    resources associated with the query. In our case, we connect
//    to our service and write the query handle. The service will
//    remove its copy of the query parameters and return a status
//    code.
//
int WSAAPI NSPLookupServiceEnd(HANDLE hLookup)
{
    SOCKET      s;
    int         retcode,
                bytesread,
                byteswritten;
    BYTE        action;
    
    // Set the action and connect to the service
    //
    action = MYNSP_LOOKUP_SERVICE_NEXT;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR;

    writedata(s, (char *)&action, sizeof(action), &byteswritten);
    //
    // Write the handle to the query we wish to close
    //
    writedata(s, (char *)&hLookup, sizeof(hLookup), &byteswritten);

    bytesread = sizeof(retcode);
    readdata(s, (char *)&retcode, sizeof(retcode), &bytesread);

    if (retcode == MYNSP_ERROR)        // Invalid handle
    {
        closesocket(s);
        SetLastError(WSA_INVALID_HANDLE);
        return SOCKET_ERROR;
    }
    closesocket(s);
 
    return NO_ERROR;
}

//
// Function: NSPSetService
//
// Description:
//    This function maps to the Winsock call WSASetService.
//    This routine is called when the user wants to register 
//    or deregister an instance of a server with our service. 
//    For registration, the user needs to associate the server 
//    with a service class. For deregistration the service class
//    is required along with the servicename. The lpqsRegInfo 
//    parameter contains a WSAQUERYSET structure defining the
//    server (such as protocol and address where it is). This 
//    routine connects to the server and writes the service
//    class and service information. The namespace service
//    returns a status code.
//
int WSAAPI NSPSetService (  
    LPGUID lpProviderId,                        
    LPWSASERVICECLASSINFOW lpServiceClassInfo,   
    LPWSAQUERYSETW lpqsRegInfo,                  
    WSAESETSERVICEOP essOperation,               
    DWORD dwControlFlags)
{
    SOCKET      s;
    BYTE        action;
    int         ret,    
                nLeft,
                retcode,
                bytesread,
                byteswritten;
    char        databuf[MARSHALL_BUFFER_SZ];

    // Set the appropriate action and connect to our namespace
    // server.
    //
    if (essOperation == RNRSERVICE_REGISTER)
        action = MYNSP_REGISTER_SERVICE;
    else if (essOperation == RNRSERVICE_DELETE)
        action = MYNSP_DEREGISTER_SERVICE;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR;

    writedata(s, (char *)&action, sizeof(action), &byteswritten);
    //
    // Marshall the WSAQUERYSET into a contiguous buffer
    //
    ret = MarshallServiceInfo(lpqsRegInfo, databuf, &nLeft);

    writedata(s, (char *)&nLeft, sizeof(nLeft), &byteswritten);
    
    writedata(s, databuf, nLeft, &byteswritten);
    //
    // Read the status code from the service
    //
    bytesread = sizeof(retcode);
    readdata(s, (char *)&retcode, sizeof(retcode), &bytesread);

    if (retcode == MYNSP_ERROR)
    {
        closesocket(s);
        WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
    }
    closesocket(s);
    
    return NO_ERROR;
}

//
// Function: NSPInstallServiceClass
//
// Description:
//    This function maps to the Winsock call WSAInstallServiceClass.
//    This routine is used to install a service class which is
//    used to define certain characteristics for a group of
//    services. After a service class is registered, an actual
//    instance of a server may be registered. This call connects
//    to the namespace service and sends the WSASERVICECLASSINFO
//    structure. The service will return a status code.
//
int WSAAPI NSPInstallServiceClass( 
    LPGUID lpProviderId,
    LPWSASERVICECLASSINFOW lpServiceClassInfo)
{
    SOCKET      s;
    char        databuf[MARSHALL_BUFFER_SZ];
    int         ret, 
                retcode, 
                nLeft, 
                bytesread,
                byteswritten;
    BYTE        action;

#ifdef DEBUG
    printf("Registering Service Class Name: %S\n", 
        lpServiceClassInfo->lpszServiceClassName);
#endif
    // Set the action and connect to the namespace service
    //
    action = MYNSP_REGISTER_CLASS;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR;

    writedata(s, (char *)&action, sizeof(action), &byteswritten);
    //
    // Marshall the WSASERVICECLASSINFO into a contiguous buffer and
    // send to the service
    //
    ret = MarshallServiceClassInfo(lpServiceClassInfo, databuf, &nLeft);

    writedata(s, (char *)&nLeft, sizeof(nLeft), &byteswritten);

    writedata(s, databuf, nLeft, &byteswritten);
    //
    // Read the status code
    //
    bytesread = sizeof(retcode);
    readdata(s, (char *)&retcode, sizeof(retcode), &bytesread);

    if (retcode == MYNSP_ERROR)        // Service class already exists
    {
        closesocket(s);
        WSASetLastError(WSAEALREADY);
        return SOCKET_ERROR;
    }
    closesocket(s);

    return NO_ERROR;
}

//
// Function: NSPRemoveServiceClass
//
// Description:
//    This function maps to the Winsock call WSARemoveServiceClass.
//    This routine removes a previously registered service class.
//    This is accomplished by connecting to the namespace service
//    and writing the GUID which defines the given service class.
//    The namespace service removes this entry and returns a
//    status code.
//
int WSAAPI NSPRemoveServiceClass(  
    LPGUID lpProviderId,
    LPGUID lpServiceClassId)
{
    SOCKET      s;
    int         byteswritten,
                bytesread,
                retcode;
    BYTE        action;
    
    // Set the action and connect to the namespace service
    // 
    action = MYNSP_DEREGISTER_CLASS;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR;

    writedata(s, (char *)&action, sizeof(action), &byteswritten);

    writedata(s, (char *)lpServiceClassId, sizeof(GUID), &byteswritten);
    //
    // Read the return code
    //
    bytesread = sizeof(retcode);
    readdata(s, (char *)&retcode, sizeof(retcode), &bytesread);
    
    if (retcode == MYNSP_ERROR)        // service class not found
    {
        closesocket(s);
        WSASetLastError(WSATYPE_NOT_FOUND);
        return SOCKET_ERROR;
    }
    closesocket(s);    
    
    return NO_ERROR;
}

//
// Function: NSPGetServiceClassInfo
//
// Description:
//    This function maps to the Winsock call WSAGetServiceClassInfo.
//    This routine returns the information associated with a given
//    service class. This is done by connecting to the namespace
//    service and sending the guid defined for that service class.
//    The service will return the associated WSASERVICECLASSINFO
//    structure.
//
int WSAAPI NSPGetServiceClassInfo(    
    LPGUID lpProviderId,                           
    LPDWORD lpdwBufSize,
    LPWSASERVICECLASSINFOW lpServiceClassInfo)
{
    SOCKET      s;
    int         byteswritten,
                bytesread,
                bytes2follow;
    BYTE        action;
    char        databuf[MARSHALL_BUFFER_SZ];

    // Set the action and connect to the namespace service
    //
    action = MYNSP_LOOKUP_CLASS;
    s = MyNspConnect();
    if (s == SOCKET_ERROR)
        return SOCKET_ERROR; 
    
    writedata(s, (char *)&action, sizeof(action), &byteswritten);

    writedata(s, (char *)lpProviderId, sizeof(GUID), &byteswritten);
    //
    // Read the number of byets to follow. If this is -1 it means
    //  an error occured (i.e. the service class was not found).
    //
    bytesread = sizeof(bytes2follow);
    readdata(s, (char *)&bytes2follow, sizeof(bytes2follow), &bytesread);

    if (bytes2follow == -1)        // Service class not found
    {
        WSASetLastError(WSATYPE_NOT_FOUND);
        return SOCKET_ERROR;
    }
    // Make sure the supplied buffer is large enough
    //
    if ((DWORD)bytes2follow > *lpdwBufSize)
    {
        *lpdwBufSize = bytes2follow;
        WSASetLastError(WSAEFAULT);
        return SOCKET_ERROR;
    }
    readdata(s, databuf, MARSHALL_BUFFER_SZ, &bytes2follow);
    
    DeMarshallServiceClassInfo(lpServiceClassInfo, databuf);

    closesocket(s);
    return NO_ERROR;
}

//
// Function: NSPCleanup
//
// Description:
//    This function is called when our namespace DLL is unloaded.
//    Since we didn't perform any custom initialization in NSPStartup
//    we don't need to clean anything up here.
//    
int WSAAPI NSPCleanup (LPGUID lpProviderId)
{
    return NO_ERROR;
}

//
// Function: NSPStartup
//
// Description:
//    When our namespace DLL is loaded, this function is automatically
//    called. It sets up an NSP_ROUTINE structure which maps our 
//    RNR functions back to the Winsock calls. Also if we had any
//    startup or intialization specific to our name space provider
//    we could put it here.
//
int WSAAPI NSPStartup (  
    LPGUID lpProviderId,
    LPNSP_ROUTINE lpnspRoutines)
{
    lpnspRoutines->cbSize = sizeof(NSP_ROUTINE);
    lpnspRoutines->dwMajorVersion = 4;
    lpnspRoutines->dwMinorVersion = 4;
    lpnspRoutines->NSPCleanup             = NSPCleanup;
    lpnspRoutines->NSPLookupServiceBegin  = NSPLookupServiceBegin;
    lpnspRoutines->NSPLookupServiceNext   = NSPLookupServiceNext;
    lpnspRoutines->NSPLookupServiceEnd    = NSPLookupServiceEnd;
    lpnspRoutines->NSPSetService          = NSPSetService;
    lpnspRoutines->NSPInstallServiceClass = NSPInstallServiceClass;
    lpnspRoutines->NSPRemoveServiceClass  = NSPRemoveServiceClass;
    lpnspRoutines->NSPGetServiceClassInfo = NSPGetServiceClassInfo;

    return NO_ERROR;
}
