// Module Name: Server1.cpp
//
// Purpose:
//     Demonstrates how to write a mailslot server application
//
// Compile:
//     cl -o Server1 Server1.cpp
//
// Command Line Options:
//     None
//

#include <windows.h>
#include <stdio.h>

void main(void)
{
	HANDLE Mailslot;
	char buffer[256];
	DWORD NumberOfBytesRead;

	// Create the mailslot
	if ((Mailslot = CreateMailslot("\\\\.\\Mailslot\\Myslot", 0,
		MAILSLOT_WAIT_FOREVER, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("Failed to create a mailslot %d\n", GetLastError());
		return;
	}

	// Read data from the mailslot forever!
	while(ReadFile(Mailslot, buffer, 256, &NumberOfBytesRead,
		NULL) != 0)
	{
		buffer[NumberOfBytesRead] = 0;
		printf("%s\n", buffer);
	}
}