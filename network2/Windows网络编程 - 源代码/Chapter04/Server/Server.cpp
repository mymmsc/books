// Module Name: Server.cpp
//
// Purpose:
//
//     This program is a simple named pipe server that demonstrates
//     the API calls needed to successfully develop a basic named
//     pipe server application. When this application receives a
//     client connection, it reads the data from the pipe and
//     reports the received message.
//
//     You need five basic steps to write a named pipe server:
//
//     1. Create a named pipe instance handle using the 
//        CreateNamedPipe() API function.
//     2. Listen for a client connection on a pipe instance using
//        the ConnectNamedPipe() API function.
//     3. Receive from and send data to the client using the 
//        ReadFile() and WriteFile() API functions.
//     4. Close down the named pipe connection using the
//        DisconnectNamedPipe() API function.
//     5. Close the named pipe instance handle using the
//        CloseHandle() API function.
//
// Compile:
//     cl -o Server Server.cpp
//
// Command Line Options:
//     None
//

#include <windows.h>
#include <stdio.h>

void main(void)
{
	HANDLE PipeHandle;
	DWORD BytesRead;
	CHAR buffer[256];

	if ((PipeHandle = CreateNamedPipe("\\\\.\\Pipe\\Jim",
		PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 1,
		0, 0, 1000, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("CreateNamedPipe failed with error %d\n",
			GetLastError());
		return;
	}

	printf("Server is now running\n");

	if (ConnectNamedPipe(PipeHandle, NULL) == 0)
	{
		printf("ConnectNamedPipe failed with error %d\n",
			GetLastError());
		CloseHandle(PipeHandle);
		return;
	}

	if (ReadFile(PipeHandle, buffer, sizeof(buffer),
		&BytesRead,  NULL) <= 0)
	{
		printf("ReadFile failed with error %d\n", GetLastError());
		CloseHandle(PipeHandle);
		return;
	}

	printf("%.*s\n", BytesRead, buffer);

	if (DisconnectNamedPipe(PipeHandle) == 0)
	{
		printf("DisconnectNamedPipe failed with error %d\n",
			GetLastError());
		return;
	}

	CloseHandle(PipeHandle);
}


