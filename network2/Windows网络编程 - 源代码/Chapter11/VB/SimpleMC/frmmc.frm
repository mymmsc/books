VERSION 5.00
Begin VB.Form frmmc 
   Caption         =   "VB Multicast"
   ClientHeight    =   4275
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   7650
   LinkTopic       =   "Form1"
   ScaleHeight     =   4275
   ScaleWidth      =   7650
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer Timer1 
      Interval        =   100
      Left            =   3720
      Top             =   1800
   End
   Begin VB.CommandButton cmdClose 
      Caption         =   "Close Socket"
      Height          =   375
      Left            =   2400
      TabIndex        =   13
      Top             =   3840
      Width           =   1215
   End
   Begin VB.TextBox txtTTL 
      Height          =   285
      Left            =   2280
      TabIndex        =   11
      Text            =   "1"
      Top             =   1200
      Width           =   1095
   End
   Begin VB.TextBox txtMCPort 
      Height          =   285
      Left            =   2280
      TabIndex        =   9
      Text            =   "24000"
      Top             =   600
      Width           =   1095
   End
   Begin VB.CommandButton cmdBind 
      Caption         =   "Bind and Join MC"
      Height          =   375
      Left            =   2040
      TabIndex        =   7
      Top             =   1800
      Width           =   1695
   End
   Begin VB.CommandButton cmdSend 
      Caption         =   "Send"
      Height          =   375
      Left            =   240
      TabIndex        =   6
      Top             =   3840
      Width           =   1695
   End
   Begin VB.TextBox txtMCIP 
      Height          =   285
      Left            =   2280
      TabIndex        =   4
      Text            =   "234.5.6.7"
      Top             =   120
      Width           =   1095
   End
   Begin VB.TextBox txtRecv 
      Height          =   3975
      Left            =   4800
      TabIndex        =   1
      Top             =   120
      Width           =   2295
   End
   Begin VB.TextBox txtSend 
      Height          =   1215
      Left            =   1320
      TabIndex        =   0
      Top             =   2400
      Width           =   2415
   End
   Begin VB.Label Label6 
      Caption         =   "4. Bind and Join MC Grp:"
      Height          =   375
      Left            =   120
      TabIndex        =   12
      Top             =   1920
      Width           =   1935
   End
   Begin VB.Label Label5 
      Caption         =   "3. IP_MULTICAST_TTL:"
      Height          =   255
      Left            =   120
      TabIndex        =   10
      Top             =   1200
      Width           =   1935
   End
   Begin VB.Label Label4 
      Caption         =   "2. Multicast Port:"
      Height          =   375
      Left            =   120
      TabIndex        =   8
      Top             =   600
      Width           =   1455
   End
   Begin VB.Label Label3 
      Caption         =   "1. Multicast IP:"
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   1215
   End
   Begin VB.Label Label2 
      Caption         =   "Receive"
      Height          =   270
      Left            =   3960
      TabIndex        =   3
      Top             =   315
      Width           =   675
   End
   Begin VB.Label Label1 
      Caption         =   "Send:"
      Height          =   255
      Left            =   690
      TabIndex        =   2
      Top             =   2595
      Width           =   495
   End
End
Attribute VB_Name = "frmmc"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: vbmcast
'
' Description:
'    This is a simple app that illustrates Winsock 1 multicasting. The user
'    specifies the multicast address and port to join. The user may also
'    specify the time to live for multicasts. Once you have joined the group
'    you may send multicast messages. If any multicast datagrams are received
'    they are placed in the list box. Note that this sample is kept simple and
'    on multihomed machines you may have to add a call to setsockopt and
'    IP_MULTICAST_IF to specify which interface you are performing multicast
'    operations on (otherwise you may never receive any multicast data).
'
Option Explicit

Dim msg_sock As Long, msgstr As String
Dim addr As sockaddr, remote_addr As sockaddr, from_addr As sockaddr, mcast As ip_mreq
Dim fromlen As Long
Dim hEvent As Long
Dim dwRet As Long, dwyes As Long, dwRc As Long
Dim ttl As Long, ttlsz As Long

Private Sub cmdClose_Click()
    If msg_sock <> INVALID_SOCKET Then
        closesocket msg_sock
    End If
    txtRecv.Text = ""
    txtSend.Text = ""
    cmdBind.Enabled = True
    cmdSend.Enabled = False
    cmdClose.Enabled = False
    Timer1.Enabled = False
End Sub

'
' Subroutine: cmdSend_Click
'
' Description:
'     When the user hits this button a multicast message is sent on
'     the socket.
'
Private Sub cmdSend_Click()
    remote_addr.sin_family = AF_INET     ' address family, internet: 2
    remote_addr.sin_port = htons(CLng(txtMCPort.Text))
    remote_addr.sin_addr = GetHostByNameAlias(txtMCIP.Text)
    dwRet = sendto(msg_sock, ByVal txtSend.Text, Len(txtSend.Text), 0, remote_addr, LenB(remote_addr))
    If dwRet = SOCKET_ERROR Then
        MsgBox "sendto failed. Error: " & Err.LastDllError
    End If
End Sub

'
' Subroutine: CmdBnd_Click
'
' Description:
'    This routine creates a multicast socket, binds it to the default
'    interface, and joins the given multicast group. Note that on
'    multihomed machines, you may need to call setsockopt with
'    IP_MULTICAST_IF to specify the explicit interface on which you
'    want to send and receive multicast datagrams.
'
Private Sub CmdBind_Click()
   'get socket handle
   msg_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
   
   If msg_sock = INVALID_SOCKET Then
       MsgBox "Couldn't create socket(). Error: " & Err.LastDllError
   Else
 
       addr.sin_family = AF_INET
       addr.sin_port = htons(CLng(txtMCPort.Text))
       addr.sin_addr = INADDR_ANY
       
       'The following block is to allow 1: send and recv broadcast (SO_BROADCAST)
       'and 2: for broadcast allow more than one receivers bind to the same port (SO_USEADDR)
       'if not broadcast, the two setsockopt calls are not necessary
       '
       ' Note that only administrators can set SO_REUSEADDR
       '
       dwyes = 1
       dwRet = setsockopt(msg_sock, SOL_SOCKET, SO_REUSEADDR, dwyes, LenB(dwyes))
       If dwRet = SOCKET_ERROR Then
            MsgBox "SO_REUSERADDR failed. Error: " & Err.LastDllError
       End If
       
       'bind to socket
       If bind(msg_sock, addr, Len(addr)) = SOCKET_ERROR Then
           MsgBox "Couldn't bind() to socket locally. Error: " & Err.LastDllError
           closesocket (msg_sock)
           msg_sock = INVALID_SOCKET
       Else
           mcast.imr_multiaddr = inet_addr(txtMCIP.Text)
           mcast.imr_interface = INADDR_ANY
           dwRet = setsockopt(msg_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, mcast.imr_multiaddr, LenB(mcast))
           If dwRet = SOCKET_ERROR Then
             MsgBox "IP_ADD_MEMBERSHIP failed. Error: " & Err.LastDllError
           End If
           ttl = CLng(txtTTL.Text)
           ttlsz = LenB(ttl)
           dwRet = setsockopt(msg_sock, IPPROTO_IP, IP_MULTICAST_TTL, ttl, ttlsz)
           If dwRet = SOCKET_ERROR Then
             MsgBox "set IP_MULTICAST_TTL failed. Error: " & Err.LastDllError
           End If
           dwRet = getsockopt(msg_sock, IPPROTO_IP, IP_MULTICAST_TTL, ttl, ttlsz)
           If dwRet = SOCKET_ERROR Then
             MsgBox "get IP_MULTICAST_TTL failed. Error: " & Err.LastDllError
           Else
             MsgBox "Multicast TTL is set to: " & ttl
           End If
         
           cmdSend.Enabled = True
           cmdBind.Enabled = False
           cmdClose.Enabled = True
           Timer1.Enabled = True
       End If
    End If
   
End Sub


Private Sub Form_Load()
    If TCPIPStartup Then
        cmdBind.Enabled = True
        cmdSend.Enabled = False
        cmdClose.Enabled = False
        Timer1.Enabled = False
    Else
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError
    End If
    
    msg_sock = INVALID_SOCKET
    hEvent = WSACreateEvent
    If hEvent = 0 Then
        MsgBox "Failed to create event. Error: " & Err.LastDllError
    End If
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If msg_sock <> INVALID_SOCKET Then
        closesocket msg_sock
    End If
    TCPIPShutDown
    If hEvent <> 0 Then WSACloseEvent hEvent
End Sub


Private Sub Timer1_Timer()
    If msg_sock = INVALID_SOCKET Then
        Debug.Print "Create Socket First"
        Exit Sub
    End If
    
    Dim RetMsg As String
    
    RetMsg = String(7000, 0)
    ' receive data
    dwRc = WSAEventSelect(msg_sock, hEvent, FD_READ)
    If (dwRc = SOCKET_ERROR) Then
        MsgBox "Failed to select event. Error: " & Err.LastDllError
        Exit Sub
    End If
        
    dwRc = WSAWaitForMultipleEvents(1, hEvent, False, 0, False)
       
    Select Case dwRc
    Case WSA_WAIT_TIMEOUT
        Debug.Print "Recv timed out"
        
    Case WSA_WAIT_EVENT_0
        Dim NetworkEvents As WSANETWORKEVENTS
        Dim dwRet As Long
        NetworkEvents.lNetWorkEvents = 0
        dwRet = WSAEnumNetworkEvents(msg_sock, hEvent, NetworkEvents)
        If (dwRet = SOCKET_ERROR) Then
            MsgBox "WSAEnumNetworkEvents failed to select event. Error: " & Err.LastDllError
        Else
                        
            'We are only interested in recv
            If (FD_READ And NetworkEvents.lNetWorkEvents) Then
                fromlen = LenB(from_addr)
                dwRc = recvfrom(msg_sock, ByVal RetMsg, 7000, 0, from_addr, fromlen)
                If dwRc = SOCKET_ERROR Then
                    MsgBox "Couldn't recieve data from remote Socket. Error: " & Err.LastDllError
                Else
                    txtRecv.Text = Left(RetMsg, InStr(RetMsg, Chr(0)))
                End If
            End If
        End If
    
    Case WSA_WAIT_FAILED
        MsgBox "WSAWaitForMultipleEvents failed. Error: " & Err.LastDllError
    Case Else
        MsgBox "Unexpected WSAWaitForMultipleEvents return. Error: " & Err.LastDllError
    End Select
    
    dwRc = WSAEventSelect(msg_sock, hEvent, 0)
    If (dwRc = SOCKET_ERROR) Then
        MsgBox "Failed to select event. Error: " & Err.LastDllError
        Exit Sub
    End If

End Sub

