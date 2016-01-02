// Module: mynspsvc.cpp
//
// Description:
//    This is the implementation of our namespace service. This
//    is what persists the data registered by applications. This
//    is implemented as a simple Winsock application which listens
//    for connections on a specific port (5150). Our namespace DLL
//    connects to this service and sends commands indicating to
//    register and lookup services as well as register and remove 
//    service classes, etc. This service maintains a list of all
//    registered service classes and services as well as queries
//    intiated by clients.
//
//    This service is implemented using the TCP protocol for the
//    sake of simplicity. A real name space would most likely use
//    UDP for the sake of speed. Of course, this would require
//    more logic to ensure that operations are successfully sent
//    and received by both this service and the namespace DLL.
//
// Compile:
//    cl mynspsvc.cpp nspsvc.cpp printobj.cpp ws2_32.lib
//
// Command Line Arguments/Parameters
//    None
//
#define UNICODE
#define _UNICODE

#include <winsock2.h>
#include <windows.h>

#include "nspsvc.h"
#include "printobj.h"


#include "mynsp.h"

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SZ        4096

#define MAX_SERVICE_CLASSES        25
#define MAX_SERVICES               50

#define MAX_SERVICE_NAME_SZ        128

#define MAX_QUERIES                50

// These two globals contain all service classes registered
// by the namespace DLL
//
WSASERVICECLASSINFO *ServiceClasses[MAX_SERVICE_CLASSES];
DWORD                dwNumServiceClasses = 0;

// These two globals contain all services registered by the
// namespace DLL
//
WSAQUERYSET         *Services[MAX_SERVICES];
DWORD                dwNumServices = 0;

// This structure maintains each client query as well as the
// current state of the query
//
typedef struct _QUERYINFO
{
    HANDLE           hHandle;  // query handle
    int              iIndex;   // last position in Services which
                               //  we found a service instance
    WSAQUERYSET     *Query;    // query parameters
} QUERYINFO;

// These two globals maintain those queries currently registered
//
QUERYINFO            QueryInfo[MAX_QUERIES];
DWORD                dwNumQueries = 0;
DWORD                dwHandleValue = 0;  // Next handle to return

//
// Function: DeregisterServiceClass
//
// Description:
//    This routine removes the given service class. The GUID 
//    corresponding to the service class is the only parameter.
//
int DeregisterServiceClass(GUID *guid)
{
    DWORD   i, j;

    for(i=0; i < dwNumServiceClasses ;i++)
    {
        // Try to find a match
        //
        if (!memcmp(guid, &ServiceClasses[i]->lpServiceClassId, sizeof(GUID)))
        {
            for(j=0; j < dwNumServices ;j++)
            {
                // If an installed services references this service class
                //  we can't delete it.
                //
                if (!memcpy(guid, &Services[j]->lpServiceClassId, sizeof(GUID)))
                    return MYNSP_ERROR;
                
            }
            // Free the memory
            //
            GlobalFree(ServiceClasses[i]);

            for(j=i; j < dwNumServiceClasses-1 ;j++)
            {
                ServiceClasses[j] = ServiceClasses[j+1];
            }
            dwNumServiceClasses--;
        
            return MYNSP_SUCCESS;
        }
    }
    return MYNSP_ERROR;
}

//
// Function: LookupServiceClass
//
// Description:
//    This function simply searches through the registered service
//    classes for a specific instance. If found that service class
//    structure is returned.
//
WSASERVICECLASSINFO *LookupServiceClass(GUID *guid)
{
    for(DWORD i = 0; i < dwNumServiceClasses; i++)
    {
        if (!memcmp(guid, ServiceClasses[i]->lpServiceClassId, sizeof(GUID)))
        {
            return ServiceClasses[i];
        }
    }
    return NULL;
}

// 
// Function: LookupService
//
// Description:
//    This function looks up a service. First it makes sure
//    that the service class specified in the query does exist.
//    If it does then search through all registered service
//    instances for a service of the given name. If so that
//    service is returned.
//
WSAQUERYSET *LookupService(GUID *guid, WCHAR *service)
{
    WSASERVICECLASSINFO        *si=NULL;

    // Make sure the service class exists
    //
    si = LookupServiceClass(guid);
    if (!si)
        return NULL;
    //
    // Search through the registered services
    //
    for(DWORD i=0; i < dwNumServices ;i++)
    {
        if (!lstrcmpW(service, Services[i]->lpszServiceInstanceName))
        {
            return Services[i];
        }
    }
    return NULL;
}

// 
// Function: DeregisterService
//
// Description:
//    This function removes an instance of a service of the given
//    service name. First a search is performed to find the service.
//    If found, the last service in the array is copied into the
//    just removed service's location.
//
int DeregisterService(GUID *guid, TCHAR *service)
{
    WSASERVICECLASSINFO    *si=NULL;

    // Make sure the given service class exists
    //
    si = LookupServiceClass(guid);
    if (!si)
    {
        return MYNSP_ERROR;
    }

    for(DWORD i=0; i < dwNumServices ;i++)
    {
        // Try to find a match
        //
        if (!lstrcmp(service, Services[i]->lpszServiceInstanceName))
        {
            // Free the memory 
            //
            GlobalFree(Services[i]);
            Services[i] = NULL;

            Services[i] = Services[dwNumServices-1];
            dwNumServices--;

            return MYNSP_SUCCESS;
        }
    }
    return MYNSP_ERROR;
}

//
// Function: RegisterQuery
//
// Description:
//    This function registers a user query and returns a unique
//    handle to identify the query.
//
BOOL RegisterQuery(WSAQUERYSET *qs, HANDLE *handle)
{
    if (dwNumQueries >= MAX_QUERIES)
        return FALSE;
    //
    // Assign the next handle value to this query. Set the iIndex
    //  to -1 to indicate no query has actually been performed
    //  yet.
    //
    QueryInfo[dwNumQueries].hHandle = (HANDLE)++dwHandleValue;
    QueryInfo[dwNumQueries].iIndex  = -1;
    QueryInfo[dwNumQueries].Query   = qs;
    dwNumQueries++;

    *handle = (HANDLE)dwHandleValue;

    return TRUE;
}

//
// Function: FindQuery
//
// Description:
//    This function find a query based on the query handle. If
//    found, the index of the QueryInfo array which contains the
//    match is returned.
//
int FindQuery(HANDLE hQuery)
{
    for(DWORD i=0; i < dwNumQueries ;i++)
    {
        if (QueryInfo[i].hHandle == hQuery)
            return i;
    }
    return -1;
}

// Function: PerformQuery
//
// Description:
//    This funcion performs the query for the given query handle.
//    First the actual query is retrieved based on its handle.
//    Next we iterate through all registered services looking
//    for those that match the search criteria.
//    NOTE! For simplicity our service only attempts to match 
//    service names (either an exact match or through the wildcard
//    asterisk). To make a full fledged namespace provider you need 
//    to match all the criteria supplied in the query!
//
int PerformQuery(HANDLE hQuery, int *iError)
{
    BOOL       bFound;
    int        index;

    // Lookup the query parameters
    //
    *iError = 0;
    index = FindQuery(hQuery);
    //
    // Search for an instance of the service starting where
    //  we left off (i.e. the iIndex value of the QueryInfo
    //  structure).
    // 
    bFound = FALSE;
    for(DWORD i=QueryInfo[index].iIndex+1; i < dwNumServices ;i++)
    {
        // If the query is a wildcard we automatically match names
        //
        if (QueryInfo[index].Query->lpszServiceInstanceName[0] == '*')
        {
            bFound = TRUE;
        }
        else
        {
            if (!lstrcmpW(QueryInfo[index].Query->lpszServiceInstanceName,
                Services[i]->lpszServiceInstanceName))
            {
                bFound = TRUE;
            }
        }
        // If we found a match update the index value
        //
        if (bFound)
        {
            QueryInfo[index].iIndex = i;
            return i;
        }
    }
    if (QueryInfo[index].iIndex == -1)
        *iError = WSASERVICE_NOT_FOUND;
    else
        *iError = WSA_E_NO_MORE;

    return -1;
}

// 
// Function: DeleteQuery
//
// Description:
//    This function removes a query associated with a query handle.
//    We first make sure the query is valid and we then move the
//    last query in the array into the position of the deleted
//    query.
// 
int DeleteQuery(HANDLE hQuery)
{
    int     index;

    // Make sure the query handle is valid
    //
    index = FindQuery(hQuery);
    if (index == -1)
        return MYNSP_ERROR;

    GlobalFree(QueryInfo[index].Query);
    QueryInfo[index].Query   = NULL;
    QueryInfo[index].hHandle = QueryInfo[dwNumQueries-1].hHandle;
    QueryInfo[index].iIndex  = QueryInfo[dwNumQueries-1].iIndex;
    QueryInfo[index].Query   = QueryInfo[dwNumQueries-1].Query;
               
    dwNumQueries--;

    return MYNSP_SUCCESS;
}

// 
// Function: main
// 
// Description:
//    The main function contains the heart of the namespace service.
//    It establishes a listening socket and then waits for client
//    connections. All client requests are handle in the main loop
//    which means all clients are handle sequentially and one at a
//    time. This is for simplicity. A real name space provider would
//    most like be able to service multiple client requests at a
//    time.
//
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        s, ns;
    SOCKADDR_IN   local, client;
    char          buf[BUFFER_SZ];
    int           clientsz,
                  ret,
                  bytesread,
                  bytes2follow,
                  byteswritten;
    BYTE          action;

    // Load Winsock and create our listening socket
    //
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        printf("socket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    local.sin_family = AF_INET;
    local.sin_port   = htons(5150);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    //
    // bind and listen on this socket
    //
    if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return -1;
    }
    if (listen(s, 7) == SOCKET_ERROR)
    {
        printf("listen() failed: %d\n", WSAGetLastError());
        return -1;
    }
    printf("listening...\n");
    while (1)
    {
        // Wait for a client connection
        //
        ns = accept(s, (SOCKADDR *)&client, &clientsz);
        if (ns == INVALID_SOCKET)
        {
            printf("accept() failed: %d\n", WSAGetLastError());
            return -1;
        }
        printf("accepted client: %s:%d\n", inet_ntoa(client.sin_addr), 
            ntohs(client.sin_port));
        //
        // Read the command
        //
        bytesread = sizeof(action);
        readdata(ns, (char *)&action, sizeof(action), &bytesread);
        //
        // Decipher the command and take appropriate action
        //
        memset(buf, 0, BUFFER_SZ);
        //
        // Decode the action
        //
        switch (action)
        {
            case MYNSP_REGISTER_CLASS:     // Register a service class
                {
                    WSASERVICECLASSINFO *sc=NULL, *sc2=NULL;
#ifdef DEBUG
                    printf("register service class\n");
#endif 
                    bytesread = sizeof(bytes2follow);
                    readdata(ns, (char *)&bytes2follow, sizeof(bytes2follow), &bytesread);

                    readdata(ns, buf, BUFFER_SZ, &bytes2follow);

                    sc = (WSASERVICECLASSINFO *)GlobalAlloc(GPTR, bytes2follow);

                    DeMarshallServiceClassInfo(sc, buf);
#ifdef DEBUG
                    PrintServiceClass(sc);
#endif
                    //
                    // See if this class already exists
                    //
                    sc2 = LookupServiceClass(sc->lpServiceClassId);
                    if (!sc2)
                    {
                        if (dwNumServiceClasses >= MAX_SERVICES)
                            ret = MYNSP_SUCCESS;
                        else
                        {
                            ServiceClasses[dwNumServiceClasses++] = sc; 
                            ret = MYNSP_SUCCESS;
                        }
                    }
                    else 
                        ret = MYNSP_ERROR;
                    //
                    // Return error or success
                    //
                    writedata(ns, (char *)&ret, sizeof(ret), &byteswritten);

                    closesocket(ns);
                    break;
                }
            case MYNSP_DEREGISTER_CLASS:    // remove a service class
                {
                    GUID           classguid;
#ifdef DEBUG
                    printf("deregister service class\n");
#endif
                    bytesread = sizeof(classguid);
                    readdata(ns, (char *)&classguid, sizeof(classguid), &bytesread);

                    ret = DeregisterServiceClass(&classguid);

                    writedata(ns, (char *)&ret, sizeof(ret), &byteswritten);

                    closesocket(ns);
                    break;
                }
            case MYNSP_LOOKUP_CLASS:       // Lookup a service class
                {
                    WSASERVICECLASSINFO *sc2=NULL;
                    GUID                 classguid;
                    int                  buflen = BUFFER_SZ;
#ifdef DEBUG
                    printf("lookup service class\n");
#endif
                    bytesread = sizeof(classguid);
                    readdata(ns, (char *)&classguid, sizeof(classguid), &bytesread);

                    sc2 = LookupServiceClass(&classguid);
                    if (!sc2)
                        ret = MYNSP_ERROR;
                    else
                        ret = MYNSP_SUCCESS;
                    writedata(ns, (char *)&ret, sizeof(ret), &byteswritten);
                        
                    if (!sc2)
                    {
                        MarshallServiceClassInfo(sc2, buf, &buflen);
                        writedata(ns, buf, buflen, &byteswritten);
                    }
                    closesocket(ns);
                    break;
                }
            case MYNSP_REGISTER_SERVICE:    // Register a service
                {
                    WSAQUERYSET        *si=NULL,
                                       *si2=NULL;
#ifdef DEBUG
                    printf("REGISTER SERVICE\n");
#endif
                    bytesread = sizeof(bytes2follow);
                    readdata(ns, (char *)&bytes2follow, sizeof(bytes2follow), 
                        &bytesread);

                    readdata(ns, buf, BUFFER_SZ, &bytes2follow);

                    si = (WSAQUERYSET *)GlobalAlloc(GPTR, bytes2follow);

                    DeMarshallServiceInfo(si, buf);
                    //
                    // See if there's already a service registered with this
                    //  name
                    //
                    si2 = LookupService(si->lpServiceClassId, si->lpszServiceInstanceName);
                    if (!si2)
                    {
#ifdef DEBUG
                        printf("Successfully registered on index: %d\n", dwNumServices);
#endif
                        Services[dwNumServices++] = si;
                        ret = MYNSP_SUCCESS;
                    }
                    else
                    {
#ifdef DEBUG
                        printf("Registration failed\n");
#endif
                        ret = MYNSP_ERROR;
                    }
                    writedata(ns, (char *)&ret, sizeof(ret), &byteswritten);
                    
                    closesocket(ns);
                    break;
                }
            case MYNSP_DEREGISTER_SERVICE:   // deregister a service
                {
                    WSAQUERYSET        *si=NULL,
                                       *si2=NULL;
#ifdef DEBUG
                    printf("deregister service\n");
#endif 
                    bytesread = sizeof(bytes2follow);
                    readdata(ns, (char *)&bytes2follow, sizeof(bytes2follow), &bytesread);

                    readdata(ns, buf, BUFFER_SZ, &bytes2follow);

                    si = (WSAQUERYSET *)GlobalAlloc(GPTR, bytes2follow);

                    DeMarshallServiceInfo(si, buf);

                    ret = DeregisterService(si->lpServiceClassId, si->lpszServiceInstanceName);
                    
                    writedata(ns, (char *)&ret, sizeof(ret), &byteswritten);

                    closesocket(ns);
                    break;
                }
            case MYNSP_LOOKUP_SERVICE_BEGIN:   // start a query
                {
                    WSAQUERYSET        *si=NULL;
                    HANDLE              handle;
#ifdef DEBUG
                    printf("lookup service begin\n");
#endif
                    bytesread = sizeof(bytes2follow);
                    readdata(ns, (char *)&bytes2follow, sizeof(bytes2follow), &bytesread);

                    si = (WSAQUERYSET *)GlobalAlloc(GPTR, bytes2follow);

                    readdata(ns, buf, BUFFER_SZ, &bytes2follow);

                    DeMarshallServiceInfo(si, buf);

                    RegisterQuery(si, &handle); 

                    writedata(ns, (char *)&handle, sizeof(handle), &byteswritten);

                    closesocket(ns);
                    break;
                }
            case MYNSP_LOOKUP_SERVICE_NEXT:   // perform the query
                {
                    HANDLE        hQuery;
                    int           nLeft,
                                  iError;

                    bytesread = sizeof(hQuery);
                    readdata(ns, (char *)&hQuery, sizeof(hQuery), &bytesread); 

#ifdef DEBUG
                    printf("lookup service next on HANDLE: %d\n", (int)hQuery);
#endif
                    //
                    // Perform the query, the iError code returned from 
                    //  PerformQuery indicates whether there are no more
                    //  results or whether nothing was found.
                    //
                    ret = PerformQuery(hQuery, &iError);
                    if (ret == -1)
                    {
                        if (iError == WSA_E_NO_MORE)
                            nLeft = 0;
                        else if (iError == WSASERVICE_NOT_FOUND)
                            nLeft = -1;
                    }
                    else
                    {
                        MarshallServiceInfo(Services[ret], buf, &nLeft);
#ifdef DEBUG
                        PrintQuery(Services[ret]);
#endif
                    }
                    writedata(ns, (char *)&nLeft, sizeof(nLeft), &byteswritten);

                    if (nLeft > 0)
                    {
                        writedata(ns, (char *)buf, nLeft, &byteswritten);
                    }

                    closesocket(ns);
                    break;
                }
            case MYNSP_LOOKUP_SERVICE_END:   // end a query
                {
                    HANDLE        hQuery;
        
                    bytesread = sizeof(hQuery);
                    readdata(ns, (char *)&hQuery, sizeof(hQuery), &bytesread);
#ifdef DEBUG
                    printf("lookup service next on HANDLE: %d\n", hQuery);
#endif
                    //
                    // Delete a query
                    //
                    ret = DeleteQuery(hQuery);

                    writedata(ns, (char *)&ret, sizeof(ret), &byteswritten);

                    closesocket(ns);
                    break;
                }
        }
    }
    closesocket(s);

    WSACleanup();
    return 0;
}
