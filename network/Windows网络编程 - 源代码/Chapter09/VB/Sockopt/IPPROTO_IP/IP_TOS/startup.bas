Attribute VB_Name = "startup"
'
' Project: vbtos
'
' Description:
'    This is a simple app that creates a datagram socket and sets the
'    IP_TOS option. Note that you cannot directly verify that the TOS
'    bits are set correctly. You must use either Netmon or the vbnetmon
'    (or its equivalent C sample: rcvall.c) to capture the packet on the
'    wire.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the only routine in this project. It loads Winsock, creates
'    a UDP socket, sets the TOS bits, and sends a datagram.
'
Sub Main()
    Dim msg_sock As Long
    Dim remote_addr As sockaddr
    Dim strBuf As String
    strBuf = "This is a test"
    Dim ret As Long, iTos As Long
    
    Dim remotehost As String
    Dim remoteport As Long
    
    '
    ' This is provided so you can use Netmon or vbnetmon to filter
    '  for this packet on the wire.
    '
    remotehost = InputBox("Enter remote host name or IP", "Remote Host")
    remoteport = CLng(InputBox("Enter remote port", "Remote Port"))
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    msg_sock = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If msg_sock = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Set the TOS bits to 1
    '
    iTos = 1
    ret = setsockopt(msg_sock, IPPROTO_IP, IP_TOS, iTos, LenB(iTos))
    If ret = SOCKET_ERROR Then
        MsgBox "IP_TOS failed. Error: " & Err.LastDllError
    End If
    remote_addr.sin_family = AF_INET     ' address family, internet: 2
    remote_addr.sin_port = htons(remoteport)
    remote_addr.sin_addr = GetHostByNameAlias(remotehost)
    '
    ' Send a datagram
    '
    ret = sendto(msg_sock, ByVal strBuf, Len(strBuf) + 1, 0, remote_addr, LenB(remote_addr))
    If ret = SOCKET_ERROR Then
        MsgBox "sendto failed. Error: " & Err.LastDllError
    End If
    '
    ' cleanup
    '
    closesocket msg_sock
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
