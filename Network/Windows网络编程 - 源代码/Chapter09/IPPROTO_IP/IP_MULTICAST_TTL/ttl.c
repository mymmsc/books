// Module: ttl.c
// 
// Description:
//    This is a simple multicast application which illustrates
//    the use of the IP_TTL option to modify the time-to-live
//    field of the IP header similar to the SIO_MULTIPOINT_SCOPE
//    ioctl does except this applies to anykind of IP traffic.
//
// Compile:
//    cl ttl.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    ttl.exe [s|r] ttl
//      s        Sender
//      r        Receiver
//      ttl      Integer TTL value
//
#include <windows.h>
#include <winsock.h>

#include <stdio.h>
#include <stdlib.h>

#define MAX_BUF             2048

#define MULTICAST_IP       "234.5.6.7"
#define MULTICAST_PORT      24000

//
// Function: usage
//
// Description:
//    Prints usage information
//
void usage(char *progname)
{
    printf("usage: %s s|r ttl\n", progname);
    printf("          s = sender\n");
    printf("          r = receiver\n");
    printf("        ttl = multicast TTL value\n");
    ExitProcess(-1);
}

//
// Function: main
//
// Description:
//    Load Winsock, parse the arguments and start either the
//    multicast sender or receiver. Before sending data
//    set the IP_TTL to the specified value.
//
int main(int argc, char **argv)
{
    WSADATA        wsd;
    SOCKET         s;
    struct ip_mreq mcast;
    SOCKADDR_IN    local,
                   from;
    int            ttl, 
                   ttlsz,
                   fromsz,
                   ret;
    char           databuf[MAX_BUF];
    BOOL           bReceive=FALSE;

    // Parse the command line
    //
    if (argc != 3)
        usage(argv[0]);
    if (tolower(argv[1][0]) == 's')
        bReceive = FALSE;
    else if (tolower(argv[1][0]) == 'r')
        bReceive = TRUE;
    else
        usage(argv[0]);

    ttl = atoi(argv[2]);
    //
    // Load winsock
    //
    if (WSAStartup(MAKEWORD(1,1), &wsd))
    {
        printf("WSAStartup failed!\n");
        return -1;
    }
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
    {
        printf("socket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    if (bReceive)
    {
        local.sin_family = AF_INET;
        local.sin_port = htons(MULTICAST_PORT);
        local.sin_addr.s_addr = INADDR_ANY;

        if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
        {
            printf("bind() failed: %d\n", WSAGetLastError());
            return -1;
        }
    }
    // Join the multicast group
    //
    mcast.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
    mcast.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        (char *)&mcast, sizeof(mcast)) == SOCKET_ERROR)
    {
        printf("setsockopt(IP_ADD_MEMBERSHIP) failed: %d\n",
            WSAGetLastError());
        return -1;
    } 
    // Set the TTL to our value
    //
    ttlsz = sizeof(ttl);
    if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL,
        (char *)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {
        printf("setsockopt(IP_MULTICAST_TTL) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    // Get the TTL to make sure
    //
    if (getsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL,
        (char *)&ttl, &ttlsz) == SOCKET_ERROR)
    {
        printf("getsockopt(IP_MULTICAST_TTL) failed: %d\n",
            WSAGetLastError());
        return -1;
    }
    printf("Multicast TTL is set to: %d\n", ttl);

    if (bReceive)
    {
        // Receive some data
        //
        fromsz = sizeof(from);
        ret = recvfrom(s, databuf, MAX_BUF, 0, 
                (SOCKADDR *)&from, &fromsz);
        if (ret == SOCKET_ERROR)
        {
            printf("recvfrom() failed: %d\n", WSAGetLastError());
            return -1;
        }
        databuf[ret] = 0;
        printf("read: [%s] from [%s]\n", databuf, inet_ntoa(from.sin_addr));
    }
    else
    {
        // Send some data
        //
        SOCKADDR_IN to;

        memset(databuf, '%', MAX_BUF);

        to.sin_family = AF_INET;
        to.sin_port = htons(MULTICAST_PORT);
        to.sin_addr.s_addr = inet_addr(MULTICAST_IP);

        ret = sendto(s, databuf, MAX_BUF-1, 0, (SOCKADDR *)&to,
                sizeof(to));
        if (ret == SOCKET_ERROR)
        {
            printf("sendto() failed: %d\n", WSAGetLastError());
            return -1;
        }
        printf("Sent %d bytes to %s\n", ret, inet_ntoa(to.sin_addr));
    }
    // Cleanup
    //
    closesocket(s);
    WSACleanup();

    return 0;
}
