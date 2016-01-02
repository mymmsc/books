Attribute VB_Name = "startup"
'
' Project: vbaccept
'
' Description:
'    This is a simple project to illustrate the use of the SO_ACCEPTCONN
'    socket option. This option will return whether the API listen has
'    been called on the given socket (which means the socket is waiting
'    for incoming connections).
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This routine loads Winsock, creates a TCP socket, bind the socket,
'    put the socket in listening mode, and then call the socket option
'    to verify that it is listening for client connections.
'
Sub Main()
    Dim s As Long
    Dim ret As Long, iSize As Long
        
    Dim addr As sockaddr
    Dim bOpt As Long
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    
    s = socket(AF_INET, SOCK_STREAM, 0)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    
    addr.sin_family = AF_INET
    addr.sin_port = htons(5150)
    addr.sin_addr = INADDR_ANY
    '
    ' Check the value of SO_ACCEPTCONN before we listen
    '
    iSize = LenB(bOpt)
    ret = getsockopt(s, SOL_SOCKET, SO_ACCEPTCONN, bOpt, iSize)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SO_ACCEPTCONN failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    
    If CBool(bOpt) = True Then
        MsgBox "SO_ACCEPTCONN returned TRUE before listen"
    Else
        MsgBox "SO_ACCEPTCONN returned FALSE before listen"
    End If
    '
    ' Now bind and listen
    '
    ret = bind(s, addr, LenB(addr))
    If ret = SOCKET_ERROR Then
        MsgBox "bind failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    
    listen s, 7
    
    '
    ' Check the value again. This time it should be true
    '
    iSize = LenB(bOpt)
    ret = getsockopt(s, SOL_SOCKET, SO_ACCEPTCONN, bOpt, iSize)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SO_ACCEPTCONN failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    
    If CBool(bOpt) = True Then
        MsgBox "SO_ACCEPTCONN returned TRUE after listen"
    Else
        MsgBox "SO_ACCEPTCONN returned FALSE after listen"
    End If
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
