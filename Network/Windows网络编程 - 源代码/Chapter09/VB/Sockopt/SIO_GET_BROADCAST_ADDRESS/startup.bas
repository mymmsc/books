Attribute VB_Name = "startup"
'
' Project: vbbcaddr
'
' Description:
'    This is a simple app that obtains the broadcast address
'    for the given socket protocol and type.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the main routine. It loads Winsock, creates a UDP datagram,
'    socket, and calls the ioctl to find the broadcast address for UDP.
'
Sub Main()
    Dim s As Long
    Dim dwBytesRet As Long
    Dim ret As Long
        
    Dim bcast As sockaddr
    Dim Buf As String
    Buf = String(1024, 0)
    
    dwBytesRet = 0
    
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Create a UDP socket. Note that you should be able to query for
    '  broadcast addresses for other address families as well (assuming
    '  they support the notion of broadcast data).
    '
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Make the call to get the broadcast address
    '
    ret = WSAIoctl(s, SIO_GET_BROADCAST_ADDRESS, ByVal 0, 0, bcast, LenB(bcast), dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_GET_BROADCAST_ADDRESS failed. Error: " & Err.LastDllError
        Exit Sub
    End If
   
    lstrcpy1 Buf, inet_ntoa(bcast.sin_addr)
    Buf = Trim(Buf)
    
    MsgBox "SIO_GET_BROADCAST_ADDRESS returns " & Buf
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
