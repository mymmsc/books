Attribute VB_Name = "startup"
'
' Project: vbsotype
'
' Description:
'    This project illustrates the use of the SO_TYPE socket
'    option. This option returns the socket type of the socket handle
'    passed into the getsockopt call.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the main routine. It loads the Winsock DLL, creates a simple UDP
'    socket, and calls getsockopt with SO_TYPE on the socket. This should
'    return SOCK_DGRAM.
'
Sub Main()
    Dim s As Long
    Dim ret As Long
    Dim iVal As Long
    Dim iSize As Long
    
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
    ' Get the socket type back.
    '
    iSize = LenB(iVal)
    ret = getsockopt(s, SOL_SOCKET, SO_TYPE, iVal, iSize)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SO_TYPE failed. Error: " & Err.LastDllError
        closesocket s
        Exit Sub
    End If
   
    Select Case iVal
    Case SOCK_STREAM
        MsgBox "SOCK_STREAM"
    Case SOCK_DGRAM
        MsgBox "SOCK_DGRAM"
    Case SOCK_RDM
        MsgBox "SOCK_RDM"
    Case SOCK_SEQPACKET
        MsgBox "SOCK_SEQPACKET"
    Case SOCK_RAW
        MsgBox "SOCK_RAW"
    Case Else
        MsgBox "Unknown"
    End Select
    
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
