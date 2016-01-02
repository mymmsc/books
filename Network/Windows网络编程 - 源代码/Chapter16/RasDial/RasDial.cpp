// Module Name: RasDial.cpp
//
// Description:
//
//    This sample demonstrates how to develop a RAS application that is
//    capable of forming RAS connections to a remote server by using the
//    RasDial API. The sample uses RasDial in the asynchronous operating
//    mode which is the preferred mode of establishing RAS connections
//    because you can monitor connection activities as they occur in the
//    RasDial API.
//
// Compile:
//
//    cl -o RasDial RasDial.cpp ras32.lib
//
// Command Line Options:
//
//    RasDial.exe -e [entry name] -p [phone number] -u [username] -z [password] -d [domain]
//

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <string.h>
#include <winbase.h>
#include <time.h>
#include <stdlib.h>

void WINAPI RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError);

HANDLE gTerminalEvent;

// Usage
void Usage(char *progname) 
{
   fprintf(stderr, "Usage\n%s \t-e [entry name] -p [phone number] "
	   "\n\t\t-u [username] -z [password] -d [domain]\n", progname);
   exit(0);
}

void main(int argc, char*argv[])
{
	RASDIALPARAMS	RasDialParams;
	RASCONNSTATUS	RasConnStatus;
	HRASCONN		hRasConn;
	DWORD			Ret;
	DWORD			tcLast;
	INT				i;
	

	// Create the event that indicates a terminal state
	if ((gTerminalEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
	{
		printf("CreateEvent failed with error %d\n", GetLastError());
		return;
	}

	RasDialParams.dwSize = sizeof(RASDIALPARAMS);
	lstrcpy(RasDialParams.szEntryName, "");
	lstrcpy(RasDialParams.szPhoneNumber, "");
	lstrcpy(RasDialParams.szUserName, "");
	lstrcpy(RasDialParams.szPassword, "");
	lstrcpy(RasDialParams.szDomain, "");

	// Copy command line arguments into the RASDIALPARAMS structure
	if (argc >1) 
	{
		for(i=1;i <argc;i++) 
		{
			if ((argv[i][0] == '-') || (argv[i][0] == '/')) 
			{
				switch(tolower(argv[i][1])) 
				{
					case 'e': // Entry name
						lstrcpy(RasDialParams.szEntryName, argv[++i]);
						break;
					case 'p': // Phone number
						lstrcpy(RasDialParams.szPhoneNumber, argv[++i]);
						break;
					case 'u': // User name
						lstrcpy(RasDialParams.szUserName, argv[++i]);
						break;
					case 'z': // Password
						lstrcpy(RasDialParams.szPassword, argv[++i]);
						break;
					case 'd': // Domain name
						lstrcpy(RasDialParams.szDomain, argv[++i]);
						break;
					default:
						Usage(argv[0]);
						break;
				}
			}
			else
				Usage(argv[0]);
		}
	}
	else
		Usage(argv[0]);

	// Dial out asynchronously using RasDial()
	printf("Dialing... %s\n", RasDialParams.szPhoneNumber);
	hRasConn = NULL;

	if (Ret = RasDial(NULL, NULL, &RasDialParams, 0, &RasDialFunc, &hRasConn))
	{
		printf("RasDial failed with error %d\n", Ret);
		return;
	}

	// Wait for RasDial to complete or enter a paused state
	Ret = WaitForSingleObject(gTerminalEvent, 50000);

	switch(Ret)
	{
		case WAIT_TIMEOUT:
		  
			// RasDial timed out
			printf("RasDial Timed out...\n");

		case WAIT_OBJECT_0:

			// Normal completion or Ras Error encountered
			printf("Will hang up in 5 seconds...\n");
			Sleep(5000);

			printf("Calling RasHangUp...\n");
			if (Ret = RasHangUp(hRasConn))
			{
				printf("RasHangUp failed with error %d\n", Ret);
				return;
			}
			
			RasConnStatus.dwSize = sizeof(RASCONNSTATUS);
			
			tcLast = GetTickCount() + 10000;

			while((RasGetConnectStatus(hRasConn, &RasConnStatus) 
				!= ERROR_INVALID_HANDLE) && (tcLast > GetTickCount()))
			{	
				Sleep(50);      
			}

			printf("Connection to %s terminated.\n", RasDialParams.szPhoneNumber);

			break;
	}
}


// Callback function RasDialFunc()

void WINAPI RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError)
{
	char szRasString[256]; // Buffer for storing the error string

	if (dwError)  // Error occurred
	{
		RasGetErrorString((UINT)dwError, szRasString, 256);
		printf("Error: %d - %s\n",dwError, szRasString);
		SetEvent(gTerminalEvent);
		return;
	}

	// Map each of the states of RasDial() and display on the screen
	// the next state that RasDial() is entering
	switch (rasconnstate)
	{
		// Running States
		case RASCS_OpenPort:
			printf ("Opening port...\n");
			break;
		case RASCS_PortOpened:
			printf ("Port opened.\n");
        	break;
		case RASCS_ConnectDevice: 
			printf ("Connecting device...\n");
			break;
		case RASCS_DeviceConnected: 
			printf ("Device connected.\n");
			break;
		case RASCS_AllDevicesConnected:
			printf ("All devices connected.\n");
			break;
		case RASCS_Authenticate: 
			printf ("Authenticating...\n");
			break;
		case RASCS_AuthNotify:
			printf ("Authentication notify.\n");
			break;
		case RASCS_AuthRetry: 
			printf ("Retrying authentication...\n");
			break;
		case RASCS_AuthCallback:
			printf ("Authentication callback...\n");
			break;
		case RASCS_AuthChangePassword: 
			printf ("Change password...\n");
			break;
		case RASCS_AuthProject: 
			printf ("Projection phase started...\n");
			break;
		case RASCS_AuthLinkSpeed: 
			printf ("Negotiating speed...\n");
			break;
		case RASCS_AuthAck: 
			printf ("Authentication acknowledge...\n");
			break;
		case RASCS_ReAuthenticate: 
			printf ("Retrying Authentication...\n");
			break;
		case RASCS_Authenticated: 
			printf ("Authentication complete.\n");
			break;
		case RASCS_PrepareForCallback: 
			printf ("Preparing for callback...\n");
			break;
		case RASCS_WaitForModemReset: 
			printf ("Waiting for modem reset...\n");
			break;
		case RASCS_WaitForCallback:
			printf ("Waiting for callback...\n");
			break;
		case RASCS_Projected:  
			printf ("Projection completed.\n");
			break;
	#if (WINVER >= 0x400) 
		case RASCS_StartAuthentication:
			printf ("Starting authentication...\n");
            break;
		case RASCS_CallbackComplete: 
			printf ("Callback complete.\n");
			break;
		case RASCS_LogonNetwork:
			printf ("Logon to the network.\n");
			break;
	#endif 
		case RASCS_SubEntryConnected:
			printf ("Subentry connected.\n");
			break;
		case RASCS_SubEntryDisconnected:
			printf ("Subentry disconnected.\n");
			break;

		// The RAS Paused States will not occur because
		// we did not use the RASDIALEXTENSIONS structure
		// to set the RDEOPT_PausedState option flag.

		// The Paused States are:

		// RASCS_RetryAuthentication:
		// RASCS_CallbackSetByCaller:
		// RASCS_PasswordExpired:

		// Terminal States
		case RASCS_Connected: 
			printf ("Connection completed.\n");
			SetEvent(gTerminalEvent);
			break;
		case RASCS_Disconnected: 
			printf ("Disconnecting...\n");
			SetEvent(gTerminalEvent);
			break;
		default:
			printf ("Unknown Status = %d\n", rasconnstate);
			break;
	}
} 
