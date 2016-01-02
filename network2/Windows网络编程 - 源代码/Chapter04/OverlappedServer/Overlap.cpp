// Module Name: Overlap.cpp
//
// Purpose:
//     This sample demonstrates how to develop an advanced named
//     pipe server that is capable of servicing 5 named pipe
//     instances. The application is an echo server where data is
//     received from a client and echoed back to the client. All
//     the pipe instances are serviced in the main application
//     thread using Win32 overlapped I/O.
//
// Compile:
//     cl -o overlap overlap.cpp
//
// Command Line Options:
//     None
//

#include <windows.h>
#include <stdio.h>

#define NUM_PIPES 5
#define BUFFER_SIZE 256

void main(void)
{
	HANDLE PipeHandles[NUM_PIPES];
	DWORD BytesTransferred;
	CHAR Buffer[NUM_PIPES][BUFFER_SIZE];
	INT i;
	OVERLAPPED Ovlap[NUM_PIPES];
	HANDLE Event[NUM_PIPES];

	// For each pipe handle instance, the code must maintain the 
	// pipes' current state, which determines if a ReadFile or 
	// WriteFile is posted on the named pipe. This is done using
	// the DataRead variable array. By knowing each pipe's 
	// current state, the code can determine what the next I/O 
	// operation should be.
	BOOL DataRead[NUM_PIPES];

	DWORD Ret;
	DWORD Pipe;

	for(i = 0; i < NUM_PIPES; i++)
	{
		// Create a named pipe instance
		if ((PipeHandles[i] = CreateNamedPipe("\\\\.\\PIPE\\jim",
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, NUM_PIPES,
			0, 0, 1000, NULL)) == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe for pipe %d failed "
				"with error %d\n", i, GetLastError());
			return;
		}

		// Create an event handle for each pipe instance. This
		// will be used to monitor overlapped I/O activity on 
		// each pipe.
		if ((Event[i] = CreateEvent(NULL, TRUE, FALSE, NULL)) 
			== NULL)
		{
			printf("CreateEvent for pipe %d failed with error %d\n",
				i, GetLastError());
			continue;
		}

		// Maintain a state flag for each pipe to determine when data
		// is to be read from or written to the pipe
		DataRead[i] = FALSE;

		ZeroMemory(&Ovlap[i], sizeof(OVERLAPPED));
		Ovlap[i].hEvent = Event[i];

		// Listen for client connections using ConnectNamedPipe()
		if (ConnectNamedPipe(PipeHandles[i], &Ovlap[i]) == 0)
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				printf("ConnectNamedPipe for pipe %d failed with",
				    " error %d\n", i, GetLastError());
				CloseHandle(PipeHandles[i]);
				return;
			}
		}
	}

	printf("Server is now running\n");


	// Read and echo data back to Named Pipe clients forever
	while(1) 
	{
		if ((Ret = WaitForMultipleObjects(NUM_PIPES, Event, 
			FALSE, INFINITE)) == WAIT_FAILED)
		{
			printf("WaitForMultipleObjects failed with error %d\n",
				GetLastError());
			return;
		}

		Pipe = Ret - WAIT_OBJECT_0;

		ResetEvent(Event[Pipe]);

		// Check overlapped results, and if they fail, reestablish 
		// communication for a new client; otherwise, process read 
		// and write operations with the client

		if (GetOverlappedResult(PipeHandles[Pipe], &Ovlap[Pipe],
			&BytesTransferred, TRUE) == 0)
		{
			printf("GetOverlapped result failed %d start over\n", 
				GetLastError());

			if (DisconnectNamedPipe(PipeHandles[Pipe]) == 0)
			{
				printf("DisconnectNamedPipe failed with error %d\n",
					GetLastError());
				return;
			}

			if (ConnectNamedPipe(PipeHandles[Pipe],
				&Ovlap[Pipe]) == 0)
			{
				if (GetLastError() != ERROR_IO_PENDING)
				{
					// Severe error on pipe. Close this
					// handle forever.
					printf("ConnectNamedPipe for pipe %d failed with"
						"error %d\n", i, GetLastError());
					CloseHandle(PipeHandles[Pipe]);
				}
			}

			DataRead[Pipe] = FALSE;
		} 
		else
		{
			// Check the state of the pipe. If DataRead equals 
			// FALSE, post a read on the pipe for incoming data.
			// If DataRead equals TRUE, then prepare to echo data 
			// back to the client.

			if (DataRead[Pipe] == FALSE)
			{
				// Prepare to read data from a client by posting a
				// ReadFile operation

				ZeroMemory(&Ovlap[Pipe], sizeof(OVERLAPPED));
				Ovlap[Pipe].hEvent = Event[Pipe];

				if (ReadFile(PipeHandles[Pipe], Buffer[Pipe],
					BUFFER_SIZE, NULL, &Ovlap[Pipe]) == 0)
				{
					if (GetLastError() != ERROR_IO_PENDING)
					{
						printf("ReadFile failed with error %d\n",
						GetLastError());
					}
				}

				DataRead[Pipe] = TRUE;
			}
			else
			{
				// Write received data back to the client by
				// posting a WriteFile operation.
				printf("Received %d bytes, echo bytes back\n",
					BytesTransferred);

				ZeroMemory(&Ovlap[Pipe], sizeof(OVERLAPPED));
				Ovlap[Pipe].hEvent = Event[Pipe];

				if (WriteFile(PipeHandles[Pipe], Buffer[Pipe],
					BytesTransferred, NULL, &Ovlap[Pipe]) == 0)
				{
					if (GetLastError() != ERROR_IO_PENDING)
					{
						printf("WriteFile failed with error %d\n",
						GetLastError());
					}
				}

				DataRead[Pipe] = FALSE;
			}
		}
	}		
}


