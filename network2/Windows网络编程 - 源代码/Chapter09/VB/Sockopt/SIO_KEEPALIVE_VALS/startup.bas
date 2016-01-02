Attribute VB_Name = "startup"
'
' Project: vbalive
'
' Description:
'    Another not so interesting sample; however it does show you how
'    to use the SIO_KEEPALIVE_VALS option. This is a way of setting
'    TCP keepalive on a per socket basis. Using the socket option
'    SO_KEEPALIVE does set keepalive on a given socket but the timeout
'    period is system wide and is specified in the registery. Therefore,
'    if you change the registry values, you change the behavior of
'    keepalives for every socket on the system. This option lets you
'    tune the timeout value on a per-socket basis.
'
'    One way of testing this is to set this option to a low value (say
'    60 seconds) and then create a socket to connect to the server.
'    Then kill the server process (using the Task Manager) and see if
'    after 60 seconds the client socket comes back with an error.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the Main routine. It loads Winsock, creates a TCP socket,
'    and then sets the keepalive on the socket.
'
Sub Main()
    Dim s As Long
    Dim dwBytesRet As Long, ret As Long
    Dim alive As tcp_keepalive
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
       
    alive.onoff = 1
    alive.keepalivetime = 7200000  'how often TCP sends a keepalive packet to determine the connection is still valid
    alive.keepaliveinterval = 6000 'interval separating keepalive retransmissions until a response is received

    ret = WSAIoctl(s, SIO_KEEPALIVE_VALS, alive, LenB(alive), ByVal 0, 0, dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_KEEPALIVE_VALS failed. Error: " & Err.LastDllError
        Exit Sub
    End If
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
