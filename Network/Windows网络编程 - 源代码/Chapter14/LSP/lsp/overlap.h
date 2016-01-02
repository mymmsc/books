
LPWSAOVERLAPPED GetOverlappedStructure(SOCKET CallerSocket,
									   SOCKET ProviderSocket,
									   LPWSAOVERLAPPED lpCallerOverlapped,
									   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCallerCompletionRoutine, 
									   LPWSATHREADID lpCallerThreadId);