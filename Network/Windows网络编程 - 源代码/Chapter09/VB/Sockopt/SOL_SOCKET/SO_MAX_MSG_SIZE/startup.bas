Attribute VB_Name = "startup"
'
' Projects: vbmaxmsg
'
' Description:
'    This simple project illustrates the SO_MAX_MSG_SIZE socket option.
'    This option returns the maximum byte size of a datagram message
'    of the given socket.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the main routine. The Winsock DLL is loaded, a UDP socket is
'    created, and the max message size possible is obtained via the
'    SO_MAX_MSG_SIZE socket option.
'
Sub Main()
    Dim s As Long
    Dim ret As Long, iSize As Long, iVal As Long
           
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Create a datagram socket
    '
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Get the max message size for the socket (and its protocol)
    '
    iSize = LenB(iVal)
    ret = getsockopt(s, SOL_SOCKET, SO_MAX_MSG_SIZE, iVal, iSize)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SO_MAX_MSG_SIZE failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Print the results
    '
    MsgBox "SO_MAX_MSG_SIZE returns " & iVal
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
