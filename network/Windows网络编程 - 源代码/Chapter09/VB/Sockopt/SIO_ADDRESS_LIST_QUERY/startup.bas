Attribute VB_Name = "startup"
'
' Project: vbaddrq
'
' Description:
'    This simple app uses the SIO_ADDRESS_LIST_QUERY ioctl to obtain
'    the local IP interfaces.
'

Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the only routine in this project. It loads Winsock, creates
'    a socket, and then performs the query. The results are printed out
'    via message boxes.
'
Sub Main()
    Dim s As Long
    Dim dwBytesRet As Long
    Dim ret As Long, i As Long
    Dim slist As SOCKET_ADDRESS_LIST
    Dim Buf As String
    Dim strResult As String
        
    Dim structAddr As sockaddr
    
    Buf = String(1024, 0)
    dwBytesRet = 0
    
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Create a socket. This is required to make an ioctl call.
    '
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Make the ioctl call
    '
    ret = WSAIoctl(s, SIO_ADDRESS_LIST_QUERY, ByVal 0, 0, slist, 1024, dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_ADDRESS_LIST_QUERY failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    
    strResult = "Bytes Returned: " & dwBytesRet & " bytes" & vbCrLf
    strResult = strResult & "      Addr Count: " & slist.iAddressCount & vbCrLf
    MsgBox strResult
    '
    ' Parse the return buffer and output the info
    '
    For i = 0 To slist.iAddressCount - 1
        Dim strSockaddr As String
        Dim ptrAddr As Long
        strSockaddr = String(2560, 0)
        ptrAddr = slist.Address(i).lpSockaddr
        CopyMemory structAddr, ByVal ptrAddr, 16

        lstrcpy1 strSockaddr, inet_ntoa(structAddr.sin_addr)
        strSockaddr = Trim(strSockaddr)
        strResult = "Addr [" & i & "]: " & strSockaddr
        MsgBox strResult
    Next i
    '
    ' Cleanup
    '
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
