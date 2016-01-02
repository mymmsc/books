Attribute VB_Name = "startup"
'
' Project: vbrcvtmo
'
' Description:
'    This simple project illustrates setting a receive timeout value on a
'    blocking socket. What this means is that when a call to a receive
'    function occurs, the call will block either for the specified amount
'    of time or until data is actually read. The recv() call in this
'    projects purposesly fails to show how it times out.
'


Option Explicit

'
' Function: Main
'
' Description:
'    This main routine loads Winsock, creates a UDP socket, sets the timeout
'    on the socket, and performs a read which will fail due to timeout.
'
Sub Main()
    Dim s As Long
    Dim ret As Long
    Dim iVal As Long, fromsz As Long, sz As Long
    Dim from As sockaddr
    Dim rcvbuf As String
    rcvbuf = String(1024, 0)
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Set the receive timeout on the socket to be 100 milliseconds
    '
    iVal = 100
    sz = LenB(iVal)
    ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, iVal, sz)
    
    If ret = SOCKET_ERROR Then
        MsgBox "set SO_RCVTIMEO failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Call getsockopt to verify the receive timeout period
    '
    ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, iVal, sz)
    If ret = SOCKET_ERROR Then
        MsgBox "get SO_RCVTIMEO failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    MsgBox "Timeout is set to " & iVal & "ms"
    from.sin_family = AF_INET
    from.sin_addr = htonl(INADDR_ANY)
    from.sin_port = htons(5100)
    
    ret = bind(s, from, LenB(from))
    If ret = SOCKET_ERROR Then
        MsgBox "bind failed. Error: " & Err.LastDllError
        closesocket s
        Exit Sub
    End If
    '
    ' This call should fail with 10060 (WSAETIMEDOUT).
    '
    fromsz = LenB(from)
    ret = recvfrom(s, rcvbuf, 1024, 0, from, fromsz)
    If ret = SOCKET_ERROR Then
        MsgBox "recvfrom fail with : " & Err.LastDllError
    End If
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
