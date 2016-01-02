// Module: nspinstall.c
//
// Description:
//    This program install and removes our custom namespace
//    provider (mynsp.dll). This is done via the WSCInstallNameSpace
//    and WSCUninstallNameSpace functions.
//
// Compile:
//    cl nspinstall.c ws2_32.lib
//
// Command Line Arguments/Parameters
//    nspinstall.exe install|remove
//
#define UNICODE
#define _UNICODE

#include <winsock2.h>
#include <ws2spi.h>

#include <stdlib.h>
#include <stdio.h>

#include "mynsp.h"

//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage(char *progname)
{
    printf("usage: %s install | remove\n", progname);
    ExitProcess(-1);
}

//
// Function: main
//
// Description:
//    This function parses the command line and executes the
//    appropriate command - either install or remove our name
//    space provider.
//
int main(int argc, char **argv)
{
    WSADATA        wsd;
    char          *ptr; 
    int            ret;

    // Check for the appropriate number of arguments.
    //
    if (argc != 2)
    {
        usage(argv[0]);
        return -1;
    }
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
    // Convert any arguments to lower case
    //
    ptr = argv[1];
    while (*ptr)
        *ptr++ = tolower(*ptr);
    //
    // Install the name space provider
    //
    if (!strncmp(argv[1], "install", 6))
    {
        // Pnstall the provider
        //
        ret = WSCInstallNameSpace(L"Custom Name Space Provider",
                L"%SystemRoot%\\System32\\mynsp.dll", NS_MYNSP, 1, &MY_NAMESPACE_GUID);
        if (ret == SOCKET_ERROR)
        {
            printf("Failed to install name space provider: %d\n",
                WSAGetLastError());
        }
        else
        {
            printf("Successfully installed name space provider\n");
        }
    }
    // Remove the name space provider
    //
    else if (!strncmp(argv[1], "remove", 6))
    {
        ret = WSCUnInstallNameSpace(&MY_NAMESPACE_GUID);
        if (ret == SOCKET_ERROR)
        {
            printf("Failed to remove provider: %d\n", WSAGetLastError());
        }
        else
        {
            printf("Successfully removed name space provider\n");
        }
    }
    else
    {
        usage(argv[0]);
    }

    WSACleanup();
    return 0;
}
