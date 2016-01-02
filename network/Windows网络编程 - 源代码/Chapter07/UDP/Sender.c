// Module Name: Sender.c
//
// Description:
//    This sample sends UDP datagrams to the specified recipient.
//    The -c option first calls connect() to associate the 
//    recipient's IP address with the socket handle so that the 
//    send() function can be used as opposed to the sendto() call.
//
// Compile:
//    cl -o Sender Sender.c ws2_32.lib
//
// Command line options:
//    sender [-p:int] [-r:IP] [-c] [-n:x] [-b:x] [-d:c]
//           -p:int   Remote port
//           -r:IP    Recipient's IP address or hostname
//           -c       Connect to remote IP first
//           -n:x     Number of times to send message
//           -b:x     Size of buffer to send
//           -d:c     Character to fill buffer with
//
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT            5150
#define DEFAULT_COUNT           25
#define DEFAULT_CHAR            'a'
#define DEFAULT_BUFFER_LENGTH   64

BOOL  bConnect = FALSE;                 // Connect to recipient first
int   iPort    = DEFAULT_PORT;          // Port to send data to
char  cChar    = DEFAULT_CHAR;          // Character to fill buffer 
DWORD dwCount  = DEFAULT_COUNT,         // Number of messages to send
      dwLength = DEFAULT_BUFFER_LENGTH; // Length of buffer to send
char  szRecipient[128];                 // Recipient's IP or hostname

//
// Function: usage
//
// Description:
//    Print usage information and exit
//
void usage()
{
    printf("usage: sender [-p:int] [-r:IP] "
		   "[-c] [-n:x] [-b:x] [-d:c]\n\n");
    printf("       -p:int   Remote port\n");
    printf("       -r:IP    Recipients IP address or hostname\n");
    printf("       -c       Connect to remote IP first\n");
    printf("       -n:x     Number of times to send message\n");
    printf("       -b:x     Size of buffer to send\n");
    printf("       -d:c     Character to fill buffer with\n\n");
    ExitProcess(1);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments, and set some global flags to
//    indicate what actions to perform
//
void ValidateArgs(int argc, char **argv)
{
    int i;

    for(i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'p':        // Remote port
                    if (strlen(argv[i]) > 3)
                        iPort = atoi(&argv[i][3]);
                    break;
                case 'r':        // Recipient's IP addr
                    if (strlen(argv[i]) > 3)
                        strcpy(szRecipient, &argv[i][3]);
                    break;
                case 'c':        // Connect to recipients IP addr
                    bConnect = TRUE;
                    break;
                case 'n':        // Number of times to send message
                    if (strlen(argv[i]) > 3)
                        dwCount = atol(&argv[i][3]);
                    break;
                case 'b':        // Buffer size
                    if (strlen(argv[i]) > 3)
                        dwLength = atol(&argv[i][3]);
                    break;
                case 'd':		// Character to fill buffer
                    cChar = argv[i][3];
                    break;
                default:
                    usage();
                    break;
            }
        }
    }
}

//
// Function: main
//
// Description:
//    Main thread of execution. Initialize Winsock, parse the command
//    line arguments, create a socket, connect to the remote IP
//    address if specified, and then send datagram messages to the
//    recipient.
//
int main(int argc, char **argv)
{
    WSADATA        wsd;
    SOCKET         s;
    char          *sendbuf = NULL;
    int            ret,
                   i;
    SOCKADDR_IN    recipient;

    // Parse the command line and load Winsock
    //
    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("WSAStartup failed!\n");
        return 1;
    }
    // Create the socket
    //
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
    {
        printf("socket() failed; %d\n", WSAGetLastError());
        return 1;
    }
    // Resolve the recipient's IP address or hostname
    //
    recipient.sin_family = AF_INET;
    recipient.sin_port = htons((short)iPort);
    if ((recipient.sin_addr.s_addr = inet_addr(szRecipient))
		== INADDR_NONE)
    {
        struct hostent *host=NULL;

        host = gethostbyname(szRecipient);
        if (host)
            CopyMemory(&recipient.sin_addr, host->h_addr_list[0],
                host->h_length);
        else
        {
            printf("gethostbyname() failed: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
    }
    // Allocate the send buffer
    //
    sendbuf = GlobalAlloc(GMEM_FIXED, dwLength);
    if (!sendbuf)
    {
        printf("GlobalAlloc() failed: %d\n", GetLastError());
        return 1;
    }
    memset(sendbuf, cChar, dwLength);
    //
    // If the connect option is set, "connect" to the recipient
    // and send the data with the send() function
    //
    if (bConnect)
    {
        if (connect(s, (SOCKADDR *)&recipient, 
                sizeof(recipient)) == SOCKET_ERROR)
        {
            printf("connect() failed: %d\n", WSAGetLastError());
            GlobalFree(sendbuf);
            WSACleanup();
            return 1;
        }
        for(i = 0; i < dwCount; i++)
        {
            ret = send(s, sendbuf, dwLength, 0);
            if (ret == SOCKET_ERROR)
            {
                printf("send() failed: %d\n", WSAGetLastError());
                break;
            }
            else if (ret == 0)
                break;
            // send() succeeded!
        }
    }
    else
    {
        // Otherwise, use the sendto() function
        //
        for(i = 0; i < dwCount; i++)
        {
            ret = sendto(s, sendbuf, dwLength, 0, 
                    (SOCKADDR *)&recipient, sizeof(recipient));
            if (ret == SOCKET_ERROR)
            {
                printf("sendto() failed; %d\n", WSAGetLastError());
                break;
            }
            else if (ret == 0)
                break;
            // sendto() succeeded!
        }
    }
    closesocket(s);

    GlobalFree(sendbuf);
    WSACleanup();
    return 0;
}
