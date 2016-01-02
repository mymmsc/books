Attribute VB_Name = "startup"
'
' Project: vbpinfo
'
' Description:
'    This is another simple app that shows you how to call the
'    SO_PROTOCOL_INFO option.
'

Option Explicit


Sub Main()
    Dim s As Long
    Dim ret As Long
    
    Dim pinf As WSAPROTOCOL_INFO
    Dim iSize As Long
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    's = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
       
    iSize = LenB(pinf)
    ret = getsockopt2(s, SOL_SOCKET, SO_PROTOCOL_INFOA, pinf, iSize)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SO_PROTOCOL_INFOA failed. Error: " & Err.LastDllError
        Exit Sub
    End If
   
    MsgBox "Protocol is " & pinf.iProtocol
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
