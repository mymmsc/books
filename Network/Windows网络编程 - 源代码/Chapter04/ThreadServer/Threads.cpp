// Module Name: Threads.cpp
//
// Purpose:
//     This sample demonstrates how to develop an advanced named
//     pipe server that is capable of servicing 5 named pipe
//     instances. The application is an echo server where data is
//     received from a client and echoed back to the client. All
//     the pipe instances are serviced using threads.
//
// Compile:
//     cl -o Threads Threads.cpp
//
// Command Line Options:
//     None
//
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#define NUM_PIPES 5

DWORD WINAPI PipeInstanceProc(LPVOID lpParameter);

void main(void) 
{
	HANDLE ThreadHandle;
	INT i;
	DWORD ThreadId;

	for(i = 0; i < NUM_PIPES; i++)
	{
		// Create a thread to serve each pipe instance  
		if ((ThreadHandle = CreateThread(NULL, 0, PipeInstanceProc,
			NULL, 0, &ThreadId)) == NULL)
		{
			printf("CreateThread failed with error %\n",
				GetLastError());
			return;
		}
		CloseHandle(ThreadHandle);
	}

	printf("Press a key to stop the server\n");
	_getch();
}

//
// Function: PipeInstanceProc
//
// Description:
//     This function handles the communication details of a single
//     named pipe instance.
//     
DWORD WINAPI PipeInstanceProc(LPVOID lpParameter)
{
	HANDLE PipeHandle;
	DWORD BytesRead;
	DWORD BytesWritten;
	CHAR Buffer[256];

	if ((PipeHandle = CreateNamedPipe("\\\\.\\PIPE\\jim",
		PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		NUM_PIPES, 0, 0, 1000, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("CreateNamedPipe failed with error %d\n",
			GetLastError());
		return 0;
	}

	// Serve client connections forever
	while(1) 
	{
		if (ConnectNamedPipe(PipeHandle, NULL) == 0)
		{
			printf("ConnectNamedPipe failed with error %d\n",
				GetLastError());
			break;
		}

		// Read data from and echo data to the client until
		// the client is ready to stop
		while(ReadFile(PipeHandle, Buffer, sizeof(Buffer),
			&BytesRead,  NULL) > 0)
		{
			printf("Echo %d bytes to client\n", BytesRead);

			if (WriteFile(PipeHandle, Buffer, BytesRead,
				&BytesWritten, NULL) == 0)
			{
				printf("WriteFile failed with error %d\n",
					GetLastError());
				break;
			}
		}

		if (DisconnectNamedPipe(PipeHandle) == 0)
		{
			printf("DisconnectNamedPipe failed with error %d\n",
				GetLastError());
			break;
		}
	}

	CloseHandle(PipeHandle);
	return 0;
}
