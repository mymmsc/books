// Module Name: Client.cpp
//
// Purpose:
//     To demonstrate how to write a mailslot client application
//
// Compile:
//     cl -o Client Client.cpp
//
// Command Line Parameters/Options:
//     <server name> - Specifies what mailslot server to send data
//                     to
//

#include <windows.h>
#include <stdio.h>

void main(int argc, char *argv[])
{
	HANDLE Mailslot;
	DWORD BytesWritten;
	CHAR ServerName[256];

	// Accept a command line argument for the server to send
	// a message to
	if (argc < 2)
	{
		printf("Usage: client <server name>\n");
		return;
	}

	sprintf(ServerName, "\\\\%s\\Mailslot\\Myslot", argv[1]);

	if ((Mailslot = CreateFile(ServerName, GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with error %d\n", GetLastError());
		return;
	}

	if (WriteFile(Mailslot, "This is a test", 14, &BytesWritten, 
		NULL) == 0)
	{
		printf("WriteFile failed with error %d\n", GetLastError());
		return;
	}

	printf("Wrote %d bytes\n", BytesWritten);

	CloseHandle(Mailslot);
}