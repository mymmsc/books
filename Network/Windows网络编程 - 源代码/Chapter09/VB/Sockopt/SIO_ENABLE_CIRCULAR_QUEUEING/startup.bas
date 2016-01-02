Attribute VB_Name = "startup"
'
' Project: vbcq
'
' Description:
'    The only real point to this sample is to illustrate setting an
'    ioctl command on a socket because actually verifying that the
'    option SIO_ENABLE_CIRCULAR_QUEUING is working is rather difficult.
'    This option affects datagrams received on the local host.
'    Normally when the local buffers are full and more datagrams
'    are received the new datagrams are dropped. If this ioctl is
'    set then the oldest datagrams sitting in the queue is discarded
'    and the newly received one is enqueued.

Option Explicit

Sub Main()
    Dim s As Long
    Dim dwBytesRet As Long
    Dim ret As Long
    Dim bOpt As Boolean
        
    dwBytesRet = 0
        
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Create a socket
    '
    s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Set the ioctl
    '
    bOpt = True
    ret = WSAIoctl(s, SIO_ENABLE_CIRCULAR_QUEUEING, bOpt, LenB(bOpt), ByVal 0, ByVal 0, dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_ADDRESS_LIST_QUERY failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    '
    ' For grins, lets make sure it is set by querying the value of this ioctl
    '
    ret = WSAIoctl(s, SIO_ENABLE_CIRCULAR_QUEUEING, ByVal 0, 0, bOpt, LenB(bOpt), dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_ADDRESS_LIST_QUERY failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    
    If bOpt = True Then
        MsgBox "Circular queueing is TRUE"
    Else
        MsgBox "Circular queueing is FALSE"
    End If
   
    closesocket s
    
    TCPIPShutDown
    MsgBox "Done! "

End Sub
