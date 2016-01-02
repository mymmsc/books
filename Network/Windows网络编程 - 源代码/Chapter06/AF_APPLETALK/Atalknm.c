// Module Name: atalknm.c 
//
// Description:
//    This app returns various information about either registered
//    AppleTalk names or looks up zone information for the current
//    workstation. The various options are:
//                
// Compile:
//    cl -o atalknm atalknm.c ws2_32.lib
//
// Command Line Options:
//    atalknm.exe [Name Lookup] [All Zones] [My Zone]
//
//    Name Lookup
//    -z:ZONE    Specify the zone name to lookup (default is "*")
//    -t:TYPE    Specify a type
//    -o:OBJECT  Specify the object name
//
//    All Zones
//    -lz        List all zones
//
//    My Zone
//    -lz        List my zone
//
#include <winsock.h>
#include <atalkwsh.h>

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_ZONE	      "*"
#define DEFAULT_TYPE	      "Windows Sockets"
#define DEFAULT_OBJECT        "AppleTalk-Server"

char szZone[MAX_ENTITY],     // Zone to lookup
     szType[MAX_ENTITY],     // Type the object belongs to
     szObject[MAX_ENTITY];   // Appletalk object (name) to look for

BOOL bFindName=FALSE,        // Find a particular name
     bListZones=FALSE,       // List all zones
     bListMyZone=FALSE;      // List my zone

//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage()
{
    printf("usage: atlookup [options]\n");
    printf("         Name Lookup:\n");
    printf("           -z:ZONE-NAME\n");
    printf("           -t:TYPE-NAME\n");
    printf("           -o:OBJECT-NAME\n");
    printf("         List All Zones:\n");
    printf("           -lz\n");
    printf("         List My Zone:\n");
    printf("           -lm\n");
    ExitProcess(1);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse command line argumens and setup the global variables.
//
void ValidateArgs(int argc, char **argv)
{
    int                i;

    strcpy(szZone, DEFAULT_ZONE);
    strcpy(szType, DEFAULT_TYPE);
    strcpy(szObject, DEFAULT_OBJECT);

    for(i=1; i < argc ;i++)
    {
        if (strlen(argv[i]) < 2)
            continue;
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'z':        // Specify a zone name
                    if (strlen(argv[i]) > 3)
                        strncpy(szZone, &argv[i][3], MAX_ENTITY);
                    bFindName = TRUE;
                    break;
                case 't':        // Specify a type name
                    if (strlen(argv[i]) > 3)
                        strncpy(szType, &argv[i][3], MAX_ENTITY);
                    bFindName = TRUE;
                    break;
                case 'o':        // Specify an object name
                    if (strlen(argv[i]) > 3)
                        strncpy(szObject, &argv[i][3], MAX_ENTITY);
                    bFindName = TRUE;
                    break;
                case 'l':        // List zones information
                    if (strlen(argv[i]) == 3)
                        if (tolower(argv[i][2]) == 'z')        // list all zones
                            bListZones = TRUE;
                        else if (tolower(argv[i][2]) == 'm')   // list my zone
                            bListMyZone = TRUE;
                    break;
                default:
                    usage();
            }
        }
    }
}

//
// Function: main
//
// Description:
//    Load the Winsock library, parse the arguments, and call
//    appropriate Winsock function for the given command.
//
int main(int argc, char **argv)
{
    WSADATA              wsd;
    char                 cLookupBuffer[16000],
                        *pTupleBuffer=NULL;
    PWSH_NBP_TUPLE       pTuples=NULL;
    PWSH_LOOKUP_NAME	 atlookup;
    PWSH_LOOKUP_ZONES    zonelookup;
    SOCKET               s;
    DWORD                dwSize = sizeof(cLookupBuffer),
                         i;
    SOCKADDR_AT		 ataddr;

    // Load the Winsock library
    //
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("Unable to load Winsock library!\n");
        return 1;
    }

    ValidateArgs(argc, argv);

    atlookup = (PWSH_LOOKUP_NAME)cLookupBuffer;
    zonelookup = (PWSH_LOOKUP_ZONES)cLookupBuffer;

    if (bFindName)
    {
        // Fill in the name to lookup
        //
        strcpy(atlookup->LookupTuple.NbpName.ObjectName, szObject );
        atlookup->LookupTuple.NbpName.ObjectNameLen = strlen(szObject);
        strcpy(atlookup->LookupTuple.NbpName.TypeName, szType);
        atlookup->LookupTuple.NbpName.TypeNameLen = strlen(szType);
        strcpy(atlookup->LookupTuple.NbpName.ZoneName, szZone);
        atlookup->LookupTuple.NbpName.ZoneNameLen = strlen(szZone);
    }
    // Create the AppleTalk socket
    // 
    s = socket(AF_APPLETALK, SOCK_STREAM, ATPROTO_ADSP);
    if (s == INVALID_SOCKET)
    {
        printf("socket() failed: %d\n", WSAGetLastError());
        return 1;
    }
    // We need to bind in order to create an endpoint on the
    //  AppleTalk network to make our query from
    //
    ZeroMemory(&ataddr, sizeof(ataddr));
    ataddr.sat_family = AF_APPLETALK; 
    ataddr.sat_socket = 0;
    if (bind(s, (SOCKADDR *)&ataddr, sizeof(ataddr)) == INVALID_SOCKET)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        return 1;
    }

    if (bFindName)
    {
        printf("Looking up: %s:%s@%s\n", szObject, szType, szZone);
        if (getsockopt(s, SOL_APPLETALK, SO_LOOKUP_NAME,
                   (char *)atlookup, &dwSize) == INVALID_SOCKET)
        {
            printf("getsockopt(SO_LOOKUP_NAME) failed: %d\n",
                WSAGetLastError());
            return 1;
        }
        printf("Lookup returned: %d entries\n", atlookup->NoTuples);
        //
        // Our character buffer now contains an array of WSH_NBP_TUPLE
        //  structures after our WSH_LOOKUP_NAME structure.
        //
        pTupleBuffer = (char *)cLookupBuffer + sizeof(WSH_LOOKUP_NAME);
        pTuples = (PWSH_NBP_TUPLE) pTupleBuffer;
    
        for(i=0; i < atlookup->NoTuples; i++)
        {
            ataddr.sat_family = AF_APPLETALK;
            ataddr.sat_net    = pTuples[i].Address.Network;
            ataddr.sat_node   = pTuples[i].Address.Node;
            ataddr.sat_socket = pTuples[i].Address.Socket;
            printf("server address = %lx.%lx.%lx.\n", ataddr.sat_net,
                    ataddr.sat_node, ataddr.sat_socket); 
        } 
    }
    else if (bListZones)
    {
        // It is very important to pass a sufficiently big buffer for
        //  this option. NT4 SP3 blue screens if it is too small.
        //
        if (getsockopt(s, SOL_APPLETALK, SO_LOOKUP_ZONES,
                   (char *)atlookup, &dwSize) == INVALID_SOCKET)
        {
            printf("getsockopt(SO_LOOKUP_NAME) failed: %d\n",
                WSAGetLastError());
            return 1;
        }
        printf("Lookup returned: %d zones\n", zonelookup->NoZones);
        //
        // The character buffer contains a list of null separated
        //  strings after the WSH_LOOKUP_ZONES structure
        //
        pTupleBuffer = (char *)cLookupBuffer + sizeof(WSH_LOOKUP_ZONES);
        for(i=0; i < zonelookup->NoZones; i++)
        {
            printf("%3d: '%s'\n", i+1, pTupleBuffer);
            while (*pTupleBuffer++);
        } 
    }
    else if (bListMyZone)
    {
        // This option returns a simple string
        //
        if (getsockopt(s, SOL_APPLETALK, SO_LOOKUP_MYZONE,
                   (char *)cLookupBuffer, &dwSize) == INVALID_SOCKET)
        {
            printf("getsockopt(SO_LOOKUP_NAME) failed: %d\n",
                WSAGetLastError());
            return 1;
        }
        printf("My Zone: '%s'\n", cLookupBuffer);
    }
    else
        usage();
    WSACleanup();

    return 0;
}
