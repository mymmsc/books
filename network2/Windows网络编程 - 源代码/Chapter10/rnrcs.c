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
//
#include <winsock2.h>
#include <windows.h>

#include <ws2tcpip.h>
#include <svcguid.h>
#include <wsipx.h>
#include <wsnwlink.h> 
#include <nspapi.h>

#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM_CSADDRS        20
#define MAX_INTERFACE_LIST     20
#define MAX_NUM_SOCKETS        20

#define MAX_BUFFER_SZ         512

#define MAX_SERVER_NAME_SZ     64

#define DEFAULT_SERVER_NAME    "JunkServer"

char    szServerName[MAX_SERVER_NAME_SZ];   // Server name
BOOL    bServer;                            // Client or server?
DWORD   dwNameSpace,                        // Name space to register on
        dwSapId;                            // Used to create a GUID
SOCKET  socks[MAX_NUM_SOCKETS];             // Socket handles for our server
HANDLE  hEvents[MAX_NUM_SOCKETS];

BOOL  ClientSend(int socktype, int proto, SOCKADDR *server, 
         int len, int count);
char *FormatIPXAddr(SOCKADDR_IPX *addr);

//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage(char *progname)
{
    printf("usage: %s  -c:Service -s  -t:ID -n:[NS_ALL|NS_NTDS|NS_SAP]\n",
        progname);
    printf("  -c:ServiceName      Act as client and query for this service name\n");
    printf("  -s:ServiceName      Act as server and register service as this name\n");
    printf("  -t:ID               SAP ID to register under\n");
    printf("                        This is arbitrary...just needed to create a GUID\n");
    printf("  -n:NameSpace        Name space to register service under\n");
    printf("                        Can be one of the strings: NS_ALL, NS_NTDS, or NS_SAP\n");
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
    strcpy(szServerName, DEFAULT_SERVER_NAME);
    dwNameSpace = NS_ALL;
    dwSapId = 200;

    for(i=0; i < MAX_NUM_SOCKETS ;i++)
        socks[i] = SOCKET_ERROR;

    for (i=1; i < argc ;i++) 
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) 
        {
            switch (tolower(argv[i][1])) 
            {
                case 't':        // SAP id used to generate GUID
                    if (strlen(argv[i]) > 3)
                        dwSapId = atoi(&argv[i][3]);
                    break;
                case 'c':        // Client, query for given service
                    bServer = FALSE;
                    if (strlen(argv[i]) > 3)
                        strcpy(szServerName, &argv[i][3]);
                    break;
                case 's':        // Server, register as the given service
                    bServer = TRUE;
                    if (strlen(argv[i]) > 3)
                        strcpy(szServerName, &argv[i][3]);
                    break;
                case 'n':        // Name spaces to register service under
                    if (!strnicmp("NS_NTDS", &argv[i][3], strlen("NS_NTDS")))
                        dwNameSpace = NS_NTDS;
                    else if (!strnicmp("NS_SAP", &argv[i][3], strlen("NS_SAP")))
                        dwNameSpace = NS_SAP;
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
            // Currently we only support SAP or NTDS name spaces
            // so ignore anything else (like DNS)
            //
            case NS_SAP:
            case NS_NTDS: 
                memcpy(&nsinfocpy[j++], &nsinfo[i], sizeof(WSANAMESPACE_INFO));
                break;
        }
    }
    // Free the original buffer and return our copy of useable name spaces
    //
    GlobalFree(pbuf);

    printf("Found %d useable name spaces\n", j);

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
    DWORD                dwZero=0,
                         dwUdpPort=0;
    char                 szServiceClassName[64];
    int                  i,
                         ret;

    // Generate a name of our service class
    //
    wsprintf(szServiceClassName, "ServiceClass: %003d", dwSapId);
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

    for(i=0; i < nscount ;i++)
    {
        // No matter what name space we use we set the connection
        // oriented attribute to false (i.e. connectionless)
        //
        nsclass[i*2].lpszName = SERVICE_TYPE_VALUE_CONN;
        nsclass[i*2].dwNameSpace = nsinfo[i].dwNameSpace;
        nsclass[i*2].dwValueType = REG_DWORD;
        nsclass[i*2].dwValueSize = sizeof(DWORD);
        nsclass[i*2].lpValue = &dwZero;

        if (nsinfo[i].dwNameSpace == NS_NTDS)
        {
            // If NS_NTDS is available we will be running a UDP
            // based service on the given port number
            //
            printf("Setting NS_NTDS info...\n");
            nsclass[(i*2)+1].lpszName = SERVICE_TYPE_VALUE_UDPPORT;
            nsclass[(i*2)+1].dwNameSpace = nsinfo[i].dwNameSpace;
            nsclass[(i*2)+1].dwValueType = REG_DWORD;
            nsclass[(i*2)+1].dwValueSize = sizeof(DWORD);
            nsclass[(i*2)+1].lpValue = &dwUdpPort;            
        }
        else if (nsinfo[i].dwNameSpace == NS_SAP)
        {
            // If NS_SAP is available we will run an IPX based
            // service on the given SAP ID
            //
            printf("Setting NS_SAP info...\n");
            nsclass[(i*2)+1].lpszName = SERVICE_TYPE_VALUE_SAPID;
            nsclass[(i*2)+1].dwNameSpace = nsinfo[i].dwNameSpace;
            nsclass[(i*2)+1].dwValueType = REG_DWORD;
            nsclass[(i*2)+1].dwValueSize = sizeof(DWORD);
            nsclass[(i*2)+1].lpValue = &dwSapId;            
        }
    }
    // Install the service class
    //
    ret = WSAInstallServiceClass(&sci);
    if (ret == SOCKET_ERROR)
    {
        printf("WSAInstallServiceClass() failed: %d\n",
            WSAGetLastError());
        return FALSE;
    }
    GlobalFree(nsclass);

    return TRUE;
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
// Function: GetIPXInterfaceList
//
// Description:
//    This function enumerates the local IPX interfaces and returns
//    them in an array of SOCKADDR_IPX structures. The only way to
//    do this is to use the socket option IPX_MAX_ADAPTER_NUM to
//    find out how many interfaces there are and then call 
//    IPX_ADDRESS on each one to find the address.
//
SOCKADDR_IPX *GetIPXInterfaceList(SOCKET s, int *count)
{
    static SOCKADDR_IPX sa_ipx[MAX_NUM_CSADDRS];
    IPX_ADDRESS_DATA    ipxdata;
    int                 ifcount,
                        nSize,
                        ret,
                        i;
    
    *count = 0;
    ifcount = 0;
    //
    // Find out how many IPX interfaces are available
    //
    nSize = sizeof(ifcount);
    ret = getsockopt(s, NSPROTO_IPX, IPX_MAX_ADAPTER_NUM,
        (char *)&ifcount, &nSize);
    if (ret == SOCKET_ERROR)
    {
        printf("getsockopt(IPX_MAX_ADAPTER_NUM) failed: %d\n",
            WSAGetLastError());
        return NULL;
    }
    // Loop through and get the address of each interface
    //
    for(i=0; i < ifcount ;i++)
    {
        nSize = sizeof(ipxdata);

        memset(&ipxdata, 0, sizeof(ipxdata));
        ipxdata.adapternum = i;
        ret = getsockopt(s, NSPROTO_IPX, IPX_ADDRESS, 
            (char *)&ipxdata, &nSize);
        if (ret == SOCKET_ERROR)
        {
            printf("getsockopt(IPX_ADDRESS) failed: %d\n", WSAGetLastError());
            continue;
        }
        // Make sure that this interface is actually up and running
        //
        if (ipxdata.status == TRUE)
        {
            memcpy(&sa_ipx[i].sa_netnum, &ipxdata.netnum, sizeof(ipxdata.netnum));
            memcpy(&sa_ipx[i].sa_nodenum, &ipxdata.nodenum, sizeof(ipxdata.nodenum));
        }
    }
    *count = ifcount;
    
    return sa_ipx;
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
    int nscount, char *servicename)
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
    qs.dwNameSpace = NS_ALL;
    qs.lpNSProviderId = NULL;
    qs.lpcsaBuffer = csaddrs;
    qs.lpBlob = NULL;

    addrcnt=0;
    //
    // For each valid name space we create an instance of the
    // service and find out what local interfaces are available
    // that the client can connect to and communicate with the server.
    //
    for (i=0; i < nscount ;i++)
    {
        if (nsinfo[i].dwNameSpace == NS_NTDS)
        {
            SOCKADDR_IN     localip;
            SOCKADDR_IN    *iflist=NULL;
            int             ipifcount;

            // Create a UDP based server
            //
            printf("Setting up NTDS entry...\n");
            socks[i] = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP,
                NULL, 0, WSA_FLAG_OVERLAPPED);
            if (socks[i] == INVALID_SOCKET)
            {
                printf("WSASocket() failed: %d\n", WSAGetLastError());
                return FALSE;
            }

            localip.sin_family = AF_INET;
            localip.sin_port = htons(0);
            localip.sin_addr.s_addr = htonl(INADDR_ANY);
 
            ret = bind(socks[i], (SOCKADDR *)&localip, sizeof(localip));
            if (ret == SOCKET_ERROR)
            {
                printf("bind() failed: %d\n", WSAGetLastError());
                return FALSE;
            } 
	    // Get the port number back since we don't specifically give one
	    //
            iSize = sizeof(localip);
            ret = getsockname(socks[i], (SOCKADDR *)&localip, &iSize);
            if (ret == SOCKET_ERROR)
            {
                printf("getsockname(IP) failed: %d\n", WSAGetLastError());
                return FALSE;
            }
            // Get a list of the IP interfaces   
            //
            iflist = GetIPInterfaceList(socks[i], &ipifcount);
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
                iflist[j].sin_port = localip.sin_port;

                csaddrs[addrcnt].iSocketType = SOCK_DGRAM;
                csaddrs[addrcnt].iProtocol = IPPROTO_UDP;
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
        else if (nsinfo[i].dwNameSpace == NS_SAP)
        {
            SOCKADDR_IPX        localipx,
                               *ipxlist;
            int                 ipxifcount;

            // Create an intance of our IPX server
            //
            printf("Setting up IPX entry...\n");
            socks[i] = WSASocket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX, 
                NULL, 0, WSA_FLAG_OVERLAPPED);
            if (socks[i] == INVALID_SOCKET)
            {
                printf("WSASocket() failed: %d\n", WSAGetLastError());
                return FALSE;
            }
            memset(&localipx, 0, sizeof(SOCKADDR_IPX));
            localipx.sa_family = AF_IPX;
             
            ret = bind(socks[i], (SOCKADDR *)&localipx, sizeof(localipx));
            if (ret == SOCKET_ERROR)
            {
                printf("bind() failed: %d\n", WSAGetLastError());
                return FALSE;
            }
            // Find out the socket number that the IPX server is listening
            // on since we didn't explicitly specify one.
            // 
            iSize = sizeof(localipx);
            ret = getsockname(socks[i], (SOCKADDR *)&localipx, &iSize);
            if (ret == SOCKET_ERROR)
            {
                printf("getsockname(IPX) failed: %d\n", WSAGetLastError());
                return FALSE;
            }
            // Enumerate the local IPX interfaces on which clients can
            // connect to us
            // 
            ipxlist = GetIPXInterfaceList(socks[i], &ipxifcount);
            if (!ipxlist)
            {
                printf("Unable to get the IPX interface list!\n");
                return FALSE;
            }
            printf("Number IPX interfaces: %d\n", ipxifcount);
            for(j=0; j < ipxifcount ;j++)
            {
		ipxlist[j].sa_family = AF_IPX;
                ipxlist[j].sa_socket = localipx.sa_socket;

                csaddrs[addrcnt].iSocketType = SOCK_DGRAM;
                csaddrs[addrcnt].iProtocol = NSPROTO_IPX;
                csaddrs[addrcnt].LocalAddr.lpSockaddr = (SOCKADDR *)&ipxlist[j];
                csaddrs[addrcnt].LocalAddr.iSockaddrLength = sizeof(ipxlist[j]);
                csaddrs[addrcnt].RemoteAddr.lpSockaddr = (SOCKADDR *)&ipxlist[j];
                csaddrs[addrcnt].RemoteAddr.iSockaddrLength = sizeof(ipxlist[j]);

                printf("\t[%d] Local  IPX [%s]\n", j,
                    FormatIPXAddr((SOCKADDR_IPX *)(csaddrs[addrcnt].LocalAddr.lpSockaddr)));
                printf("\t[%d] Remote IPX [%s]\n", j,
                    FormatIPXAddr((SOCKADDR_IPX *)(csaddrs[addrcnt].RemoteAddr.lpSockaddr)));

                addrcnt++;
            }
        }
        // Create an event for the server to use with WSAEventSelect
        //
        hEvents[i] = WSACreateEvent();
        if (hEvents[i] == WSA_INVALID_EVENT)
        {
            printf("WSACreateEvent() failed: %d\n", WSAGetLastError());
            return FALSE;
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
void LookupService(GUID *guid, int sapid, int ns, char *servername)
{
    WSAQUERYSET  qs,
                *pqs;
    AFPROTOCOLS  afp[2] = {{AF_IPX,  NSPROTO_IPX},
                           {AF_INET, IPPROTO_UDP}},
                 afpx[1] = {{AF_IPX, NSPROTO_IPX}};
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
    qs.dwNameSpace = NS_SAP;
    qs.dwNumberOfProtocols = 1; 
    qs.lpafpProtocols = afpx;
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
                case IPPROTO_UDP:
                    printf("IPPROTO_UDP\n");
                    ((SOCKADDR_IN *)pqs->lpcsaBuffer[i].RemoteAddr.lpSockaddr)->sin_family = AF_INET;
                    break;
                case NSPROTO_IPX:
                    printf("NSPROTO_IPX\n");
                    ((SOCKADDR_IPX *)pqs->lpcsaBuffer[i].RemoteAddr.lpSockaddr)->sa_family = AF_IPX;
                    printf("\t[%d] Local  IPX [%s]\n", i,
                        FormatIPXAddr((SOCKADDR_IPX *)(pqs->lpcsaBuffer[i].LocalAddr.lpSockaddr)));
                    printf("\t[%d] Remote IPX [%s]\n", i,
                        FormatIPXAddr((SOCKADDR_IPX *)(pqs->lpcsaBuffer[i].RemoteAddr.lpSockaddr)));
                    break;
                default:
                    printf("Unknown!\n");
                    break;
            }
            // Send data
            //
            ClientSend(pqs->lpcsaBuffer[i].iSocketType, pqs->lpcsaBuffer[i].iProtocol,
                (SOCKADDR *)pqs->lpcsaBuffer[i].RemoteAddr.lpSockaddr, 
                pqs->lpcsaBuffer[i].RemoteAddr.iSockaddrLength, 4);
        }
    }
    WSALookupServiceEnd(hLookup);

    return;
}

//
// Function: ClientSend
//
// Description:
//    Create a socket of the given type and send some data to it
//
BOOL ClientSend(int socktype, int proto, SOCKADDR *server, int len, int count)
{
    WSABUF   wbuf;
    char     sndbuf[MAX_BUFFER_SZ];
    DWORD    dwBytesSent;
    int      i,
             ret;
  
    if (proto == NSPROTO_IPX)
    {
        socks[0] = WSASocket(AF_IPX, socktype, proto, NULL, 0,
            WSA_FLAG_OVERLAPPED);
        if (socks[0] == INVALID_SOCKET)
        {
            printf("WSASocket() failed: %d\n", WSAGetLastError());
            return FALSE;
        }
    }
    else if (proto == IPPROTO_UDP)
    {
        socks[0] = WSASocket(AF_INET, socktype, proto, NULL, 0,
            WSA_FLAG_OVERLAPPED);
        if (socks[0] == INVALID_SOCKET)
        {
            printf("WSASocket() failed: %d\n", WSAGetLastError());
            return FALSE;
        }
    }
    memset(sndbuf, '%', MAX_BUFFER_SZ);
    for(i=0; i < count ;i++)
    {
        wbuf.buf = sndbuf;
        wbuf.len = MAX_BUFFER_SZ -1;

        ret = WSASendTo(socks[0], &wbuf, 1, &dwBytesSent,
            0, server, len , NULL, NULL);
        if (ret == SOCKET_ERROR)
        {
            printf("WSASendTo() failed: %d\n", WSAGetLastError());
            closesocket(socks[0]);
            return FALSE;
        }
    }
    closesocket(socks[0]);

    return TRUE;
}

//
// Function: ServerRecv
//
// Description;
//    Read data pending on the given socket.
//
BOOL ServerRecv(int index)
{
    WSABUF   wbuf;
    char     rcvbuf[MAX_BUFFER_SZ];
    SOCKADDR from;
    DWORD    dwBytesRecv,
             dwFlags,
             dwFromLen;
    int      ret;

    wbuf.buf = rcvbuf;
    dwFlags = 0;
    dwFromLen = sizeof(from);
    wbuf.len = MAX_BUFFER_SZ-1;
        
    ret = WSARecvFrom(socks[index], &wbuf, 1, &dwBytesRecv, &dwFlags,
        &from, &dwFromLen, NULL, NULL);
    if (ret == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            return TRUE;
        else
        {
            printf("WSARecvFrom() failed: %d\n", WSAGetLastError());
            return FALSE;
        }
    }
    rcvbuf[dwBytesRecv] = 0;
    printf("Read: [%s]\n", rcvbuf);

    return TRUE;
}

//
// Function: FormatIPXAddr
//
// Description:
//    Print an IPX address to stdout. We don't use the function
//    WSAAddressToString because it doesn't support IPX.
//
char *FormatIPXAddr(SOCKADDR_IPX *addr)
{
    static char dest[128];
    wsprintf(dest, "%02X%02X%02X%02X.%02X%02X%02X%02X%02X%02X:%04X", 
        (unsigned char)addr->sa_netnum[0],
        (unsigned char)addr->sa_netnum[1],
        (unsigned char)addr->sa_netnum[2],
        (unsigned char)addr->sa_netnum[3],
        (unsigned char)addr->sa_nodenum[0],
        (unsigned char)addr->sa_nodenum[1],
        (unsigned char)addr->sa_nodenum[2],
        (unsigned char)addr->sa_nodenum[3],
        (unsigned char)addr->sa_nodenum[4],
        (unsigned char)addr->sa_nodenum[5],
        ntohs(addr->sa_socket));
    return dest;
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
    // Create the GUID that everyone will use, client and server
    //
    SET_NETWARE_SVCID(&svcguid, dwSapId); 

    StringFromGUID2(&svcguid, szTemp, 256);
    printf("GUID: [%S]\n", szTemp);
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
        // Register our services for the read event
        //
        for(i=0; i < nscount ;i++)
        {
            ret = WSAEventSelect(socks[i], hEvents[i], FD_READ);
            if (ret == SOCKET_ERROR)
            {
                printf("WSAEventSelect() failed: %d\n", WSAGetLastError());
                return -1;
            }
        }
        while (1)
        {
            // Read any incoming data
            //
            ret = WSAWaitForMultipleEvents(nscount, hEvents, FALSE, 
                INFINITE, TRUE);
            ServerRecv(ret - WSA_WAIT_EVENT_0);
        }
    }
    else
    {
        // Lookup the service
        //
        LookupService(&svcguid, dwSapId, dwNameSpace, szServerName);
    }
    HeapFree(GetProcessHeap(), 0, nsinfo);

    WSACleanup();

    return 0;
}
