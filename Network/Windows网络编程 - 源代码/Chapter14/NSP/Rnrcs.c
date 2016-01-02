// Module: rnrcs.c
//
// Description:
//    This sample illustrates how a server can create a socket and
//    advertise the existance of the service to clients.  It also
//    shows how a client with knowledge of the server's name only
//    can find out how to connect to the server in order to transmit
//    data. The server accomplishes this by installing a service
//    class which describes the basic characteristics of the service
//    and then registering an instance of the server which references
//    the given class. The service can be registered for multiple
//    name spaces such as IPX and NTDS.
//
//    The client can query the available name spaces for knowledge
//    of a service of the given name. If the query completes 
//    successfully, the address of the service is returned.  All
//    the client needs to do is use that address in either a
//    connect or sendto call.
//
// Compile:
//     cl rnrcs.c ws2_32.lib user32.lib ole32.lib
//
// Command Line Arguments/Parameters
//     rnrcs -c:ServiceName -s:ServiceName -t:ID -n:NameSpace
//       -c:ServiceName      Act as client and query for this service name
//       -s:ServiceName      Act as server and register service as this name
//       -t:ID               SAP ID to register under
//       -n:NameSpace        Name space to register service under
//                             Supported namespaces are NS_ALL, NS_SAP, NS_NTDS
//       -d                  Delete the service if found
//

#include <winsock2.h>
#include <windows.h>

#include <ws2tcpip.h>
#include <nspapi.h>

#include <svcguid.h>

#include "mynsp.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM_CSADDRS        20
#define MAX_INTERFACE_LIST     20
#define MAX_NUM_SOCKETS        20

#define MAX_BUFFER_SZ         512

#define MAX_SERVER_NAME_SZ     64

#define DEFAULT_SERVER_NAME    "JunkServer"

char    szServerName[MAX_SERVER_NAME_SZ];   // Server name
BOOL    bServer,                            // Client or server?
        bDeleteService;
DWORD   dwUniqueId;                         // Used to create a GUID
SOCKET  sock;

//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage(char *progname)
{
    printf("usage: %s  -c:Service -s  -t:ID\n",
        progname);
    printf("  -c:ServiceName Act as client and query for this service name\n");
    printf("  -s:ServiceName Act as server and register service as this name\n");
    printf("  -t:ID          Unique ID to register under\n");
    printf("  -d             Delete the service on a successful query\n");
    ExitProcess(-1);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse command line parameters and set some global variables used
//    by the application.
//
void ValidateArgs(int argc, char **argv)
{
    int    i;

    // Set some default values
    //
    lstrcpy(szServerName, TEXT(DEFAULT_SERVER_NAME));
    dwUniqueId = 200;
    bDeleteService = FALSE;

    for (i=1; i < argc ;i++) 
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) 
        {
            switch (tolower(argv[i][1])) 
            {
                case 't':        // SAP id used to generate GUID
                    //if (lstrlen(argv[i]) > 3)
                    //    dwUniqueId = atoi(&argv[i][3]);
                    break;
                case 'c':        // Client, query for given service
                    bServer = FALSE;
                    if (lstrlen(argv[i]) > 3)
                        lstrcpy(szServerName, &argv[i][3]);
                    break;
                case 's':        // Server, register as the given service
                    bServer = TRUE;
                    if (lstrlen(argv[i]) > 3)
                        lstrcpy(szServerName, &argv[i][3]);
                    break;
                case 'd':
                    bDeleteService = TRUE;
                    break;
                default:
                    usage(argv[0]);
                    return;
            }
        }
        else
            usage(argv[0]);
    }
    return;
}

//
// Function: EnumNameSpaceProviders
//
// Description:
//    Returns an array of those name spaces which our application
//    supports. If one day, NS_DNS becomes dynamic, modify this
//    function to return that structure as well. 
//
WSANAMESPACE_INFO *EnumNameSpaceProviders(int *count)
{
    WSANAMESPACE_INFO *nsinfo,
                      *nsinfocpy;
    BYTE              *pbuf=NULL,
                      *pbufcpy=NULL;
    DWORD              dwBytes;
    int                i, j,
                       ret;

    *count = 0;
    dwBytes = 0;
    //
    // First find out how big of a buffer we need
    //
    ret = WSAEnumNameSpaceProviders(&dwBytes, NULL);
    if (ret == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEFAULT)
        {
            printf("WSAEnumNameSpaceProviders() failed: %d\n",
                WSAGetLastError());
            return NULL;
        }
    }
    // Allocate this buffer and call the function again
    //
    pbuf = (BYTE *)GlobalAlloc(GPTR, dwBytes);
    if (!pbuf)
    {
        printf("GlobaAlloc() failed: %d\n", GetLastError());
        return NULL;
    }
    nsinfo = (WSANAMESPACE_INFO *)pbuf;
    ret = WSAEnumNameSpaceProviders(&dwBytes, nsinfo);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAEnumNameSpaceProviders() failed: %d\n",
            WSAGetLastError());
        HeapFree(GetProcessHeap(), 0, pbuf);
        return NULL;
    }
    // Make a copy buffer which we will return our data in
    //
    pbufcpy = GlobalAlloc(GPTR, dwBytes);
    if (!pbufcpy)
    {
        printf("GlobaAlloc() failed: %d\n", GetLastError());
        return NULL;
    }
    // Loop throug the returned name space structures picking
    // those which our app supports
    //
    nsinfocpy = (WSANAMESPACE_INFO *)pbufcpy;
    printf("%d name spaces are available\n", ret);

    j = 0;
    for(i=0; i < ret ;i++)
    {
        switch (nsinfo[i].dwNameSpace)
        {
            // In this example all we want is our own name space 
            //
            case NS_MYNSP:
                printf("Found a match: %S\n", nsinfo[i].lpszIdentifier); 
                memcpy(&nsinfocpy[j++], &nsinfo[i], sizeof(WSANAMESPACE_INFO));
                break;
	    default:
		break;
        }
    }
    // Free the original buffer and return our copy of useable name spaces
    //
    GlobalFree(pbuf);

    printf("Found %d useable name spaces\n\n", j);

    *count = j;
    return nsinfocpy;
}

//
// Function: InstallServiceClass
//
// Description:
//    This function installs the service class which is required before
//    registering an instance of a service. A service class defines the
//    generic attributes of a class of services such as whether it is
//    connection oriented or not as well as what protocols is supports
//    (IPX, IP, etc).
//
BOOL InstallServiceClass(GUID *svcguid, WSANAMESPACE_INFO *nsinfo, int nscount)
{
    WSASERVICECLASSINFO  sci;
    WSANSCLASSINFO      *nsclass=NULL;
    DWORD                dwOne=1;
    TCHAR                szServiceClassName[64];
    int                  i,
                         ret;
    BOOL                 bRet;

    bRet = TRUE;
    //
    // Generate a name of our service class
    //
    wsprintf(szServiceClassName, TEXT("Service Class/TCP Port: %003d"), dwUniqueId);
    printf("Installing service class: '%s'\n", szServiceClassName);
    //
    // There are 2 attributes we need to set for every name space
    // we want to register in: Connection Oriented/Connectionless as
    // well as the address family of the protocols we support.
    //
    nsclass = GlobalAlloc(GPTR, sizeof(WSANSCLASSINFO) * nscount * 2);
    if (!nsclass)
    {
        printf("GlobalAlloc() failed: %d\n", GetLastError());
        return FALSE;
    }
    // Initialize the structure
    //
    memset(&sci, 0, sizeof(sci));
    sci.lpServiceClassId = svcguid;
    sci.lpszServiceClassName = szServiceClassName;
    sci.dwCount = nscount * 2;
    sci.lpClassInfos = nsclass;

    printf("Namespace count: %d, Namespace: %d\n", nscount, nsinfo[0].dwNameSpace);
    for(i=0; i < nscount ;i++)
    {
        // No matter what name space we use we set the connection
        // oriented attribute to false (i.e. connectionless)
        //
        nsclass[i*2].lpszName = SERVICE_TYPE_VALUE_CONN;
        nsclass[i*2].dwNameSpace = nsinfo[i].dwNameSpace;
        nsclass[i*2].dwValueType = REG_DWORD;
        nsclass[i*2].dwValueSize = sizeof(DWORD);
        nsclass[i*2].lpValue = &dwOne;

        if (nsinfo[i].dwNameSpace == NS_MYNSP)
        {
            // If NS_MYNSP is available we will be running a UDP
            // based service on the given port number
            //
            printf("Setting NS_MYNSP info...\n");
            nsclass[(i*2)+1].lpszName = SERVICE_TYPE_VALUE_TCPPORT;
            nsclass[(i*2)+1].dwNameSpace = nsinfo[i].dwNameSpace;
            nsclass[(i*2)+1].dwValueType = REG_DWORD;
            nsclass[(i*2)+1].dwValueSize = sizeof(DWORD);
            nsclass[(i*2)+1].lpValue = &dwUniqueId;            
        }
    }
    // Install the service class
    //
    ret = WSAInstallServiceClass(&sci);
    if (ret == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEALREADY)
        {
            printf("Service class already registered\n");
            bRet = TRUE;
        }
        else
        {
            printf("WSAInstallServiceClass() failed: %d\n",
                WSAGetLastError());
            bRet = FALSE;
        }
    }
    GlobalFree(nsclass);

    return bRet;
}

//
// Function: GetIPInterfaceList
//
// Description:
//    This function returns an array of SOCKADDR_IN structures,
//    one for every local IP interface on the machine. We use
//    the ioctl command SIO_GET_INTERFACE_LIST to do this although
//    we could have used any number of method such as gethostbyname(),
//    SIO_INTERFACE_LIST_QUERY, or the IP helper APIs.
//
SOCKADDR_IN *GetIPInterfaceList(SOCKET s, int *count)
{
    static SOCKADDR_IN  sa_in[MAX_NUM_CSADDRS];
    INTERFACE_INFO      iflist[MAX_INTERFACE_LIST];
    DWORD               dwBytes;
    int                 ret, 
                        i;
    *count = 0;

    ret = WSAIoctl(s, SIO_GET_INTERFACE_LIST, NULL, 0, &iflist,
        sizeof(iflist), &dwBytes, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl(SIO_GET_INTERFACE_LIST) failed: %d\n",
            WSAGetLastError());
        return NULL;
    }
    // Loop through the interfaces and copy them into the SOCKADDR_IN
    // array. 
    //
    *count = dwBytes / sizeof(INTERFACE_INFO); 
    for(i=0; i < *count ;i++)
    {
        memcpy(&sa_in[i], &iflist[i].iiAddress.AddressIn, sizeof(SOCKADDR_IN));
    }
    
    return sa_in;
}

//
// Function: Advertise
//
// Description:
//    This function advertises an instance of the server. This 
//    function also creates the server for each available name
//    space. To advertise you need all the local interfaces that
//    the client can connect the to the server.  This is done
//    by filling out a WSAQUERYSET structure along with the
//    appropriate CSADDR_INFO structures.  The CSADDR_INFO
//    structures define the interfaces the service is listening on.
//
BOOL Advertise(GUID *guid, WSANAMESPACE_INFO *nsinfo, 
    int nscount, TCHAR *servicename)
{
    WSAQUERYSET     qs;
    CSADDR_INFO     csaddrs[MAX_NUM_CSADDRS];
    int             ret,
                    i, j,
                    iSize,
                    addrcnt;

    // Initialize the WSAQUERYSET structure
    //
    memset(&qs, 0, sizeof(WSAQUERYSET));

    qs.dwSize = sizeof(WSAQUERYSET);
    qs.lpszServiceInstanceName = servicename;
    qs.lpServiceClassId = guid;
    qs.dwNameSpace = NS_MYNSP;
    qs.lpNSProviderId = &nsinfo[0].NSProviderId;
    qs.lpcsaBuffer = csaddrs;
    qs.lpBlob = NULL;
    qs.lpszComment = "Dork this";

    addrcnt=0;
    //
    // For each valid name space we create an instance of the
    // service and find out what local interfaces are available
    // that the client can connect to and communicate with the server.
    //
    for (i=0; i < nscount ;i++)
    {
        if (nsinfo[i].dwNameSpace == NS_MYNSP)
        {
            SOCKADDR_IN     localip;
            SOCKADDR_IN    *iflist=NULL;
            int             ipifcount;

            // Create a TCP based server
            //
            printf("Setting up NS_MYNSP entry...\n");
            sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                NULL, 0, WSA_FLAG_OVERLAPPED);
            if (sock == INVALID_SOCKET)
            {
                printf("WSASocket() failed: %d\n", WSAGetLastError());
                return FALSE;
            }

            localip.sin_family = AF_INET;
            localip.sin_port = htons((short)dwUniqueId);
            localip.sin_addr.s_addr = htonl(INADDR_ANY);
 
            ret = bind(sock, (SOCKADDR *)&localip, sizeof(localip));
            if (ret == SOCKET_ERROR)
            {
                printf("bind() failed: %d\n", WSAGetLastError());
                return FALSE;
            } 
            // Get a list of the IP interfaces   
            //
            iflist = GetIPInterfaceList(sock, &ipifcount);
            if (!iflist)
            {
                printf("Unable to enumerate IP interfaces!\n");
                return FALSE;
            }
            // Fill out the CSADDR_INFO structures with each IP interface
            //
            for (j=0; j < ipifcount ;j++)
            {
		iflist[j].sin_family = AF_INET;
                iflist[j].sin_port = htons((short)dwUniqueId);

                csaddrs[addrcnt].iSocketType = SOCK_STREAM;
                csaddrs[addrcnt].iProtocol = IPPROTO_TCP;
                csaddrs[addrcnt].LocalAddr.lpSockaddr = (SOCKADDR *)&iflist[j];
                csaddrs[addrcnt].LocalAddr.iSockaddrLength = sizeof(iflist[j]);
                csaddrs[addrcnt].RemoteAddr.lpSockaddr = (SOCKADDR *)&iflist[j];
                csaddrs[addrcnt].RemoteAddr.iSockaddrLength = sizeof(iflist[j]);

                printf("\t[%d] Local  IP [%s:%d]\n", j,
                    inet_ntoa(((SOCKADDR_IN *)(csaddrs[addrcnt].LocalAddr.lpSockaddr))->sin_addr),
                    ntohs(((SOCKADDR_IN *)(csaddrs[addrcnt].LocalAddr.lpSockaddr))->sin_port));

                printf("\t[%d] Remote IP [%s:%d]\n", j,
                    inet_ntoa(((SOCKADDR_IN *)(csaddrs[addrcnt].RemoteAddr.lpSockaddr))->sin_addr),
                    ntohs(((SOCKADDR_IN *)(csaddrs[addrcnt].RemoteAddr.lpSockaddr))->sin_port));

                addrcnt++;
            }
        }
    }
    qs.dwNumberOfCsAddrs = addrcnt;
    //
    // Register our service(s)
    //
    ret = WSASetService(&qs, RNRSERVICE_REGISTER, 0L);
    if (ret == SOCKET_ERROR)
    {
        printf("WSASetService() failed: %d\n", WSAGetLastError());
        return FALSE;
    }
    printf("WSASetService() succeeded\n");
    return TRUE;
}

//
// Function: LookupService
//
// Description:
//    This function queries for an instance of the given service
//    running on the network. You can either query for a specific
//    service name or specify the wildcard string "*". If an instance
//    is found, send some data to it.
//
void LookupService(GUID *guid, int sapid, int ns, TCHAR *servername)
{
    WSAQUERYSET  qs,
                *pqs;
    AFPROTOCOLS  afp[1] = { {AF_INET, IPPROTO_TCP} };
    char         querybuf[sizeof(WSAQUERYSET) + 4096];
    DWORD        nSize = sizeof(WSAQUERYSET) + 4096,
                 i;
    HANDLE       hLookup;
    int          ret, err;

    // Initialize the WSAQUERYSET structure
    //
    pqs = (WSAQUERYSET *)querybuf;    
    memset(&qs, 0, sizeof(WSAQUERYSET));

    qs.dwSize = sizeof(WSAQUERYSET);
    qs.lpszServiceInstanceName = servername;
    qs.lpServiceClassId = guid;
    qs.lpNSProviderId =  &MY_NAMESPACE_GUID;
    qs.dwNameSpace = NS_MYNSP;
    qs.dwNumberOfProtocols = ns; 
    qs.lpafpProtocols = afp;
    //
    // Begin the lookup. We want the name and address back
    // 
    ret = WSALookupServiceBegin(&qs, LUP_RETURN_ADDR | LUP_RETURN_NAME,
        &hLookup);
    if (ret == SOCKET_ERROR)
    {
        printf("WSALookupServiceBegin failed: %d\n", WSAGetLastError());
        return;
    }
    while (1)
    {
        // Loop, calling WSALookupServiceNext until WSA_E_NO_MORE is
        // returned.
        //
        nSize = sizeof(WSAQUERYSET) + 4096;
        memset(querybuf, 0, nSize);

        pqs->dwSize = sizeof(WSAQUERYSET);
        ret = WSALookupServiceNext(hLookup, 0, &nSize, pqs);

        if (ret == SOCKET_ERROR)
        {
            err = WSAGetLastError();
            if ((err == WSA_E_NO_MORE) || (err == WSAENOMORE))
            {
                printf("No more data found!\n");
                break;
            }
            else if (err == WSASERVICE_NOT_FOUND)
            {
                printf("Service not found!\n");
                break;
            }
            printf("WSALookupServiceNext() failed: %d\n", WSAGetLastError());
            WSALookupServiceEnd(hLookup);
            return;
        }
        // Now that we've found a server out there, print some info and
        // send some data to it.
        //

        printf("\nFound service: %s\n\n", pqs->lpszServiceInstanceName);
        printf("Returned %d CSADDR structures\n", pqs->dwNumberOfCsAddrs);

        for(i=0; i < pqs->dwNumberOfCsAddrs ;i++)
        {
            switch (pqs->lpcsaBuffer[i].iProtocol)
            {
                case IPPROTO_TCP:
                    printf("IPPROTO_TCP: '%s'\n", inet_ntoa(((SOCKADDR_IN *)pqs->lpcsaBuffer[i].RemoteAddr.lpSockaddr)->sin_addr));
                    ((SOCKADDR_IN *)pqs->lpcsaBuffer[i].RemoteAddr.lpSockaddr)->sin_family = AF_INET;
                    break;
                default:
                    printf("Unknown!: %d\n", pqs->lpcsaBuffer[i].iProtocol);
                    break;
            }
            // Send data
            //
        }
        if (bDeleteService)
            ret = WSASetService(pqs, RNRSERVICE_DELETE, 0L);
    }
    WSALookupServiceEnd(hLookup);

    printf("DONE!\n");

    return;
}

//
// Function: main
//
// Description:
//    Initialize Winsock, parse the arguments, and start either the
//    client or server depending on the arguments.
//
int main(int argc, char **argv)
{
    WSANAMESPACE_INFO *nsinfo=NULL;
    WSADATA            wsd;
    GUID               svcguid;
    int                nscount, 
                       ret,
                       i;
    WCHAR              szTemp[256];

    ValidateArgs(argc, argv);
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Generate a GUID for our service class
    //
    SET_TCP_SVCID(&svcguid, dwUniqueId);

    //
    // Enumerate the name spaces that we can use
    //
    nsinfo = EnumNameSpaceProviders(&nscount);
    if (!nsinfo)
    {
        printf("unable to enumerate name space info!\n");
        return -1;
    }
    for(i=0; i < nscount ;i++)
        printf("Found NS: %s\n", nsinfo[i].lpszIdentifier);
    
    if (bServer)
    {
        if (szServerName[0] == '*')
        {
            printf("You must specify a server name!\n");
            usage(argv[0]);
            return -1;
        }
        // Install the service class
        //
        if (!InstallServiceClass(&svcguid, nsinfo, nscount))
        {
            printf("Unable to install service class!\n");
            return -1;
        }
        // Advertise our service
        //
        if (!Advertise(&svcguid, nsinfo, nscount, szServerName))
        {
            printf("Unable to advertise service!\n");
            return -1;
        }
    }
    else
    {
        // Lookup the service
        //
        LookupService(&svcguid, dwUniqueId, NS_MYNSP, szServerName);

    }
    HeapFree(GetProcessHeap(), 0, nsinfo);

    WSACleanup();

    return 0;
}
