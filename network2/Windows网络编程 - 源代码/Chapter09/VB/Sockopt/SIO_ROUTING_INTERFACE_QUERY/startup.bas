Attribute VB_Name = "startup"
'
' Project: vbifqry
'
' Description:
'    This project illustrates the use of the SIO_ROUTING_INTERFACE_QUERY
'    ioctl. This ioctl will determine which local interface would be
'    used to reach the given destination IP address.
'
'    Note that with plug and play it is possible for the local IP
'    interfaces to change (either new ones come available or old
'    interfaces are removed). In conjunction with this ioctl you
'    can use the SIO_ROUTING_INTERFACE_CHANGE ioctl to notify you
'    when the interfaces change. See chapter 9 for more information.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the main routine. It prompts the user for a remote
'    IP address and port to query which interface would be used
'    to reach that destination. It first loads Winsock, creates
'    a socket, and then performs the ioctl query.
'
Sub Main()
    Dim s As Long
    Dim ret As Long, dwBytesRet As Long, nret As Long, i As Long
        
    Dim inputaddr As sockaddr
    Dim routeaddr(10) As sockaddr
    Dim Buf(1023) As Byte
    
    dwBytesRet = 0
    
    Dim remoteIPStr As String
    Dim RemotePortStr As String
    '
    ' Get the IP address and port to make the query for.
    '
    remoteIPStr = InputBox("Please Enter Remote IP")
    RemotePortStr = InputBox("Please Enter Remote IP")
    
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
    ' Setup the address to query for
    '
    inputaddr.sin_family = AF_INET     ' address family, internet: 2
    inputaddr.sin_port = htons(CLng(RemotePortStr))
    inputaddr.sin_addr = GetHostByNameAlias(remoteIPStr)
       
    ret = WSAIoctl(s, SIO_ROUTING_INTERFACE_QUERY, inputaddr, LenB(inputaddr), Buf(0), 1024, dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_ROUTING_INTERFACE_QUERY failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    
    CopyMemory routeaddr(0), Buf(0), 10 * LenB(routeaddr(0))
    '
    ' Parse the results
    '
    nret = dwBytesRet / LenB(routeaddr(0))
    For i = 0 To nret - 1
        Dim strSockaddr As String
        strSockaddr = String(256, 0)

        lstrcpy1 strSockaddr, inet_ntoa(routeaddr(i).sin_addr)
        strSockaddr = Trim(strSockaddr)
        MsgBox "This routing interface for the target IP and Port is: " & vbLf & "sin_family = " & routeaddr(i).sin_family & " sin_addr = " & strSockaddr
    Next i
       
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
