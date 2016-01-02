// Module Name: Client.cpp
//
// Purpose:
//
//     This program is a simple named pipe client that demonstrates
//     the API calls needed to successfully develop a basic named
//     pipe client application. When this application successfully
//     connects to a named pipe, the message "This is a test" is
//     written to the server.
//
//     There are four basic steps needed to implement a client:
//
//     1. Wait for a Named Pipe instance to become available using
//        the WaitNamedPipe() API function.
//     2. Connect to the Named Pipe using the CreateFile() API
//        function.
//     3. Send data to or receive data from the server using
//        the WriteFile() and ReadFile() API functions.
//     4. Close the Named Pipe session using the CloseHandle() API
//        functions.
//
//
// Compile:
//     cl -o Client Client.cpp
//
// Command Line Options:
//     None
//

#include <windows.h>
#include <stdio.h>

#define PIPE_NAME "\\\\.\\Pipe\\Jim"

void main(void) {

	HANDLE PipeHandle;
	DWORD BytesWritten;

	if (WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER) == 0)
	{
		printf("WaitNamedPipe failed with error %d\n",
			GetLastError());
		return;
	}

	// Create the named pipe file handle
	if ((PipeHandle = CreateFile(PIPE_NAME,
		GENERIC_READ | GENERIC_WRITE, 0,
		(LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with error %d\n", GetLastError());
		return;
	}

	if (WriteFile(PipeHandle, "This is a test", 14, &BytesWritten, 
		NULL) == 0)
	{
		printf("WriteFile failed with error %d\n", GetLastError());
		CloseHandle(PipeHandle);
		return;
	}

	printf("Wrote %d bytes", BytesWritten);

	CloseHandle(PipeHandle);
}