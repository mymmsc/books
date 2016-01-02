// Module Name: Server2.cpp
//
// Purpose:
//     This application demonstrates how to write a more advanced 
//     mailslot server that works around a Windows 9x problem with 
//     canceling blocking I/O requests.  Mailslot servers use the 
//     ReadFile() API to receive data. If a mailslot is created
//     with the MAILSLOT_WAIT_FOREVER flag, then ReadFile requests 
//     will block indefinitely until data is available. If a server
//     application is terminated when there is an outstanding 
//     ReadFile() request, the application will hang forever. This
//     application shows how to prevent this from occurring by
//     having the server open a handle to its own mailslot in a 
//     separate thread and sending data to break the blocking read
//     request when the application is terminated.  This application
//     will also work on Windows NT even though there is not an I/O
//     blocking limitation.
//     
//
// Compile:
//     cl -o Server2 Server2.cpp
//
// Command Line Options:
//     None
//

#include <windows.h>
#include <stdio.h>
#include <conio.h>

BOOL StopProcessing;

DWORD WINAPI ServeMailslot(LPVOID lpParameter);
void SendMessageToMailslot(void);

void main(void) {

	DWORD ThreadId;
	HANDLE MailslotThread;

	StopProcessing = FALSE;
	MailslotThread = CreateThread(NULL, 0, ServeMailslot, NULL,
		0, &ThreadId);

	printf("Press a key to stop the server\n");
	_getch();

	// Mark the StopProcessing flag to TRUE so when ReadFile 
	// breaks our server thread will end
	StopProcessing = TRUE;


	// Send a message to our mailslot to break the ReadFile call
	// in our server
	SendMessageToMailslot();

	// Wait for our server thread to complete
	if (WaitForSingleObject(MailslotThread, INFINITE) == WAIT_FAILED)
	{
		printf("WaitForSingleObject failed with error %d\n",
			GetLastError());
		return;
	}

}

//
// Function: ServeMailslot
//
// Description:
//     This function is the mailslot server worker function to 
//     process all incoming mailslot I/O
//
DWORD WINAPI ServeMailslot(LPVOID lpParameter)
{
	char buffer[2048];
	DWORD NumberOfBytesRead;
	DWORD Ret;
	HANDLE Mailslot;

	if ((Mailslot = CreateMailslot("\\\\.\\mailslot\\myslot", 2048,
		MAILSLOT_WAIT_FOREVER, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("Failed to create a MailSlot %d\n", GetLastError());
		return 0;
	}

	while((Ret = ReadFile(Mailslot, buffer, 2048,
		&NumberOfBytesRead, NULL)) != 0)
	{
		if (StopProcessing)
			break;

		printf("Received %d bytes\n", NumberOfBytesRead);
	}

	CloseHandle(Mailslot);

	return 0;
}

//
// Function: SendMessageToMailslot
//
// Description:
//     The SendMessageToMailslot function is designed to send a
//     simple message to our server so we can break the blocking
//     ReadFile API call
//
void SendMessageToMailslot(void)
{
	HANDLE Mailslot;
	DWORD BytesWritten;

	if ((Mailslot = CreateFile("\\\\.\\mailslot\\myslot", 
		GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with error %d\n", GetLastError());
		return;
	}

	if (WriteFile(Mailslot, "STOP", 4, &BytesWritten, NULL) == 0)
	{
		printf("WriteFile failed with error %d\n", GetLastError());
		return;
	}

	CloseHandle(Mailslot);
}
