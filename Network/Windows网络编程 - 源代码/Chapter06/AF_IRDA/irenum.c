// Module Name: Irenum.c
//
// Description:
//    This sample illustrates how to enumerate infrared devices within
//    range and query whether they contain a service instance of a 
//    given name. The IrSock code is basic but is written for Win98 and
//    NT5. Only minor differences exist between this and Windows CE, 
//    which mainly consists of hint information returned by NT5 and 98.
//    Of course Windows CE presents it own limitations and requirements
//    not covered here.
//
// Compile:
//    cl -o Irenum Irenum.c ws2_32.lib
//    
// Command Line Options:
//    irenum.exe [options]
// 
//    NONE    - Print out IR device names in range
//    -s:str  - Query each device in range for the given service
//    -a:str  - Lookup the given attribute name on each device in
//              range
//
#include <winsock2.h>
#include "af_irda.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_RETRIES			15
#define DEFAULT_ATTRIBUTE	"IrDA:TinyTP:LsapSel"

BOOL    bDoQuery = TRUE;                  // Perform a query?
BOOL	bFindLsap = FALSE;                // Find the Lsap-Sel of the 
                                          // Service in question
char	szClassName[IAS_MAX_CLASSNAME];   // Class name to lookup 
char    szAttribName[IAS_MAX_ATTRIBNAME]; // Attribute name to lookup

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments and set some global flags to
//    indicate what actions to perform.
//
void ValidateArgs(int argc, char**argv)
{
    int		i;

    // This is the default attribute to lookup
    //
    lstrcpy(szAttribName, DEFAULT_ATTRIBUTE);
    for(i = 1; i < argc; i++)
    {
		if ((argv[i][0] == '-') || (argv[i][0] == '/'))
		{
			switch (tolower(argv[i][1]))
			{
				case 's':	// Lookup a service
					bFindLsap = TRUE;
							bDoQuery = TRUE;
					lstrcpy(szClassName, &argv[i][3]);
					break;
				case 'a':   // Attribute name to query
					bDoQuery = TRUE;
					lstrcpy(szAttribName, &argv[i][3]);
					break;
				default:
					printf("usage: irenum [/s:ServiceName]\n");
							printf("              [/a:AttributeName]\n");
				break;
			}
		}
    }
}

//
// Function: main
// 
// Description:
//    Parse the command line, load the Winsock library, enumerate
//    any ir devices in range, and perform the requested query (if
//    any) against each device
//
int main(int argc, char **argv)
{
    WSADATA				wsd;
    SOCKADDR_IRDA		irAddr = {AF_IRDA, 0, 0, 0, 0, "\0"};
    WINDOWS_DEVICELIST	devList;
    WINDOWS_IAS_QUERY   query;
    SOCKET				sock;
    DWORD				dwRetries = 0,
						dwListLen = sizeof(WINDOWS_DEVICELIST),
						dwQueryLen = sizeof(WINDOWS_IAS_QUERY),
						dwRet;
    DWORD				i, j;

    // Parse the arguments and load the proper Winsock library
    //
    ValidateArgs(argc, argv);
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
	fprintf(stderr, "Unable to load Winsock library\n");
	return 0;
    }
    // Create an AF_IRDA socket
    //
    sock = WSASocket(AF_IRDA, SOCK_STREAM, 0, NULL, 0,
			WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET)
    {
		fprintf(stderr, "WSASocket() failed: %d\n", WSAGetLastError());
		return 0;
    }
    // Enumerate any IR devices in range
    //
    devList.numDevice = 0;
    while ((devList.numDevice == 0) && (dwRetries <= MAX_RETRIES))
    {
		dwRet = getsockopt(sock, SOL_IRLMP, IRLMP_ENUMDEVICES,
			(char *)&devList, &dwListLen);
		if (dwRet == SOCKET_ERROR)
		{
			fprintf(stderr, "getsockopt(IRLMP_ENUMDEVICES) failed: "
				"%d\n", WSAGetLastError());
			return 0;
		}
		dwRetries++;
		Sleep(1000);
    }
    // Print the device information of each device discovered
    //
    for(i = 0; i < devList.numDevice; i++)
    {
		printf("Device: %d\n", i);
		printf("\tDevice ID: %x%x%x%x\n", 
			devList.Device[i].irdaDeviceID[0],
	    devList.Device[i].irdaDeviceID[1],
	    devList.Device[i].irdaDeviceID[2],
	    devList.Device[i].irdaDeviceID[3]);
		printf("\tDevice Name: %s\n", devList.Device[i].irdaDeviceName);
		printf("\tHints:\n");
		if (devList.Device[i].irdaDeviceHints1 & LM_HB_Extension)
 		{
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_PnP)
			printf("\t       LM_HB1_PnP\n");
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_PDA_Palmtop)
			printf("\t       LM_HB1_PDA_Palmtop\n");
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_Computer)
			printf("\t       LM_HB1_Computer\n");
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_Printer)
			printf("\t       LM_HB1_Printer\n");
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_Modem)
			printf("\t       LM_HB1_Modem\n");
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_Fax)
			printf("\t       LM_HB1_Fax\n");
			if (devList.Device[i].irdaDeviceHints1 & LM_HB1_LANAccess)
			printf("\t       LM_HB1_LANAccess\n");
			if (devList.Device[i].irdaDeviceHints2 & LM_HB2_Telephony)
			printf("\t       LM_HB2_Telephony\n");
			if (devList.Device[i].irdaDeviceHints2 & LM_HB2_FileServer)
			printf("\t       LM_HB2_FileServer\n");
		}
		else
		    printf("NONE\n");
		// 
		// If the user requested to perform a query, do it
		//
		if (bDoQuery)
		{
			ZeroMemory(&query, sizeof(WINDOWS_IAS_QUERY));

			for(j=0; j < 4; j++)
				query.irdaDeviceID[j] = 
						devList.Device[i].irdaDeviceID[j];
			if (bFindLsap)
				lstrcpy(query.irdaClassName, szClassName);
  			lstrcpy(query.irdaAttribName, szAttribName);

			dwRet = getsockopt(sock, SOL_IRLMP, IRLMP_IAS_QUERY, 
				(char *)&query, &dwQueryLen);
			if (dwRet == SOCKET_ERROR)
			{
				if ((dwRet = WSAGetLastError()) == 10061)
				{
					printf("No such class name registered with IAS\n");
					continue;
				}
				else
				{
					fprintf(stderr, "getsockopt(IRLMP_IAS_QUERY) "
						"failed: %d\n", dwRet);
					return 0;
				}
		    }

			if (query.irdaAttribType == IAS_ATTRIB_NO_CLASS)
				printf("Attrib type: IAS_ATTRIB_NO_CLASS\n");
			else if (query.irdaAttribType == IAS_ATTRIB_NO_ATTRIB)
				printf("Attrib type: IAS_ATTRIB_NO_ATTRIB\n");
			else if (query.irdaAttribType == IAS_ATTRIB_INT)
			{
				printf("Attrib type: IAS_ATTRIB_INT\n");
			printf("             %s:%d\n", query.irdaAttribName, 
						query.irdaAttribute.irdaAttribInt);
			}
			else if (query.irdaAttribType == IAS_ATTRIB_OCTETSEQ)
				printf("Attrib type: IAS_ATTRIB_OCTETSEQ\n");
			else if (query.irdaAttribType == IAS_ATTRIB_STR)
			{
				printf("Attrib type: IAS_ATTRIB_STR\n");
				printf("             Character Set: ");
				switch (query.irdaAttribute.irdaAttribUsrStr.CharSet)
				{
					case LmCharSetASCII:
						printf("ASCII\n");
						printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_1:
						printf("ISO-8859-1\n");
						printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_2:
		    			printf("ISO-8859-2\n");
		    			printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_3:
		    			printf("ISO-8859-3\n");
		    			printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_4:
		    			printf("ISO-8859-4\n");
		    			printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_5:
						printf("ISO-8859-5\n");
						printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_6:
						printf("ISO-8859-6\n");
						printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_7:
		    			printf("ISO-8859-7\n");
		    			printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_8:
		    			printf("ISO-8859-8\n");
		    			printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetISO_8859_9:
		    			printf("ISO-8859-9\n");
		    			printf("             String: '%s'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
					case LmCharSetUNICODE:
						printf("UNICODE\n");
						printf("             String: '%S'\n", 
								   query.irdaAttribute.irdaAttribUsrStr.UsrStr);
						break;
				} // End switch
			} // End else if (query.irdaAttribType == IAS_ATTRIB_STR)
		} // End if (bDoQuery)
    } // End for
    closesocket(sock);
    WSACleanup();

    return 1;
}
