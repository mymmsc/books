// Module Name: Receiver.c
//
// Description:
//    This sample receives UDP datagrams by binding to the specified
//    interface and port number and then blocking on a recvfrom() 
//    call
//
// Compile:
//    cl -o Receiver Receiver.c ws2_32.lib
//
// Command line options:
//    sender [-p:int] [-i:IP][-n:x] [-b:x]
//           -p:int   Local port
//           -i:IP    Local IP address to listen on
//           -n:x     Number of times to send message
//           -b:x     Size of buffer to send
//
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT            5150
#define DEFAULT_COUNT           25
#define DEFAULT_BUFFER_LENGTH   4096

int   iPort    = DEFAULT_PORT;          // Port to receive on
DWORD dwCount  = DEFAULT_COUNT,         // Number of messages to read
      dwLength = DEFAULT_BUFFER_LENGTH; // Length of receiving buffer
BOOL  bInterface = FALSE;               // Use an interface other than
                                        // default
char  szInterface[32];            // Interface to read datagrams from

//
// Function: usage:
//
// Description:
//    Print usage information and exit
//
void usage()
{
    printf("usage: sender [-p:int] [-i:IP][-n:x] [-b:x]\n\n");
    printf("       -p:int   Local port\n");
    printf("       -i:IP    Local IP address to listen on\n");
    printf("       -n:x     Number of times to send message\n");
    printf("       -b:x     Size of buffer to send\n\n");
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
    int                i;

    for(i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'p':   // Local port
                    if (strlen(argv[i]) > 3)
                        iPort = atoi(&argv[i][3]);
                    break;
                case 'n':   // Number of times to receive message
                    if (strlen(argv[i]) > 3)
                        dwCount = atol(&argv[i][3]);
                    break;
                case 'b':   // Buffer size
                    if (strlen(argv[i]) > 3)
                        dwLength = atol(&argv[i][3]);
                    break;
				case 'i':	// Interface to receive datagrams on
					if (strlen(argv[i]) > 3)
					{
						bInterface = TRUE;
					strcpy(szInterface, &argv[i][3]);
					}
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
//    line arguments, create a socket, bind it to a local interface 
//    and port, and then read datagrams.
//
int main(int argc, char **argv)
{
    WSADATA        wsd;
    SOCKET         s;
    char          *recvbuf = NULL;
    int            ret,
                   i;
    DWORD          dwSenderSize;
    SOCKADDR_IN    sender,
				   local;

    // Parse arguments and load Winsock
    //
    ValidateArgs(argc, argv);

    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup failed!\n");
        return 1;
    }
    // Create the socket and bind it to a local interface and port
    //
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
    {
        printf("socket() failed; %d\n", WSAGetLastError());
        return 1;
    }
    local.sin_family = AF_INET;
    local.sin_port = htons((short)iPort);
    if (bInterface)
        local.sin_addr.s_addr = inet_addr(szInterface);
    else
		local.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
    {
		printf("bind() failed: %d\n", WSAGetLastError());
		return 1;
    }
    // Allocate the receive buffer
    //
    recvbuf = GlobalAlloc(GMEM_FIXED, dwLength);
    if (!recvbuf)
    {
        printf("GlobalAlloc() failed: %d\n", GetLastError());
        return 1;
    }
    // Read the datagrams
    //
    for(i = 0; i < dwCount; i++)
    {
        dwSenderSize = sizeof(sender);
        ret = recvfrom(s, recvbuf, dwLength, 0, 
                (SOCKADDR *)&sender, &dwSenderSize);
        if (ret == SOCKET_ERROR)
        {
            printf("recvfrom() failed; %d\n", WSAGetLastError());
            break;
        }
        else if (ret == 0)
            break;
		else
		{
			recvbuf[ret] = '\0';
			printf("[%s] sent me: '%s'\n", 
				inet_ntoa(sender.sin_addr), recvbuf);
		}
    }
    closesocket(s);

    GlobalFree(recvbuf);
    WSACleanup();
    return 0;
}
