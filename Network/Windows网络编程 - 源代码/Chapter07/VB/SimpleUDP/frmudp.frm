VERSION 5.00
Begin VB.Form frmPeerB 
   Caption         =   "Form1"
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
   Begin VB.TextBox txtLocalPort 
      Height          =   375
      Left            =   1800
      TabIndex        =   11
      Top             =   1200
      Width           =   1455
   End
   Begin VB.TextBox txtRemotePort 
      Height          =   285
      Left            =   1560
      TabIndex        =   9
      Top             =   600
      Width           =   1695
   End
   Begin VB.CommandButton cmdBind 
      Caption         =   "Bind"
      Height          =   375
      Left            =   2040
      TabIndex        =   7
      Top             =   1800
      Width           =   1335
   End
   Begin VB.CommandButton cmdSend 
      Caption         =   "Send"
      Height          =   375
      Left            =   240
      TabIndex        =   6
      Top             =   3840
      Width           =   1695
   End
   Begin VB.TextBox txtRemotePeer 
      Height          =   285
      Left            =   1440
      TabIndex        =   4
      Text            =   "weipiii"
      Top             =   120
      Width           =   1815
   End
   Begin VB.TextBox txtRecv 
      Height          =   3975
      Left            =   4560
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
      Caption         =   "4. Bind to local port"
      Height          =   375
      Left            =   120
      TabIndex        =   12
      Top             =   1920
      Width           =   1575
   End
   Begin VB.Label Label5 
      Caption         =   "3. local port to bind:"
      Height          =   375
      Left            =   120
      TabIndex        =   10
      Top             =   1200
      Width           =   1455
   End
   Begin VB.Label Label4 
      Caption         =   "2.  remote port: "
      Height          =   375
      Left            =   120
      TabIndex        =   8
      Top             =   600
      Width           =   1215
   End
   Begin VB.Label Label3 
      Caption         =   "1. remote peer"
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   1095
   End
   Begin VB.Label Label2 
      Caption         =   "receive"
      Height          =   375
      Left            =   3600
      TabIndex        =   3
      Top             =   360
      Width           =   855
   End
   Begin VB.Label Label1 
      Caption         =   "Send:"
      Height          =   255
      Left            =   480
      TabIndex        =   2
      Top             =   2760
      Width           =   495
   End
End
Attribute VB_Name = "frmPeerB"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' Project: vbudp
'
' Description:
'    This VB project illustrates sending and receiving UDP datagrams.
'    Because VB doesn't reliably implement threads and because blocking
'    sockets doesn't lend itself to single threaded applications with a
'    GUI we will be using WSAEventSelect to check for incoming datagrams.
'    This is triggered by a timer. Note that this isn't the most efficient
'    way to use sockets but we are somewhat limited by VB.
'

Option Explicit

Dim msg_sock As Long, msgstr As String
Dim addr As sockaddr, remote_addr As sockaddr, from_addr As sockaddr
Dim fromlen As Long
Dim hEvent As Long
Dim dwRet As Long, dwyes As Long, dwRc As Long

'
' Subroutine: cmdClose_Click
'
' Description:
'    This routine is triggered by the "Close Socket" button. It simply
'    closed the UDP socket that was being used to send and receive
'    UDP datagrams.
'
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
'    This routine is triggered by the "Send" button. It will send the data
'    from the Send box as a UDP datagram to the remote host and port.
'
Private Sub cmdSend_Click()
    ' Setup the remote host and port
    '
    remote_addr.sin_family = AF_INET     ' address family, internet: 2
    remote_addr.sin_port = htons(CLng(txtRemotePort.Text))
    remote_addr.sin_addr = GetHostByNameAlias(txtRemotePeer.Text)
    
    ' Send the data
    dwRet = sendto(msg_sock, ByVal txtSend.Text, Len(txtSend.Text), 0, remote_addr, LenB(remote_addr))
    If dwRet = SOCKET_ERROR Then
        MsgBox "sendto failed. Error: " & Err.LastDllError
    End If
End Sub

'
' Subroutine: CmdBind_Click
'
' Description:
'    This routines is triggered by the "Bind" button. When the user hits
'    this button, we bind to the specified local port. This is for receiving
'    datagrams as we will only receive messaged send to this port number.
'
Private Sub CmdBind_Click()
   'get socket handle
   msg_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
   
   If msg_sock = INVALID_SOCKET Then
       MsgBox "Couldn't create socket(). Error: " & Err.LastDllError
   Else
       ' Setup the local address to listen for UDP datagrams
       '
       addr.sin_family = AF_INET
       addr.sin_port = htons(CLng(txtLocalPort.Text))
       addr.sin_addr = INADDR_ANY
       
       ' The following block is to allow 1: send and recv broadcast (SO_BROADCAST)
       ' and 2: for broadcast allow more than one receivers bind to the same port
       ' (SO_USEADDR)
       ' if not broadcast, the two setsockopt calls are not necessary
       dwyes = 1
       dwRet = setsockopt(msg_sock, SOL_SOCKET, SO_REUSEADDR, dwyes, LenB(dwyes))
       If dwRet = SOCKET_ERROR Then
            MsgBox "SO_REUSERADDR failed. Error: " & Err.LastDllError
       End If
       dwyes = 1
       dwRet = setsockopt(msg_sock, SOL_SOCKET, SO_BROADCAST, dwyes, LenB(dwyes))
       If dwRet = SOCKET_ERROR Then
            MsgBox "SO_BROADCAST failed. Error: " & Err.LastDllError
       End If
       
       'bind to socket
       If bind(msg_sock, addr, Len(addr)) = SOCKET_ERROR Then
           MsgBox "Couldn't bind() to socket locally. Error: " & Err.LastDllError
           closesocket (msg_sock)
           msg_sock = INVALID_SOCKET
       Else
           cmdSend.Enabled = True
           cmdBind.Enabled = False
           cmdClose.Enabled = True
           Timer1.Enabled = True
       End If
    End If
   
End Sub

'
' Subroutine: Form_Load
'
' Description:
'    This routine initializes Winsock as well as the form.
'
Private Sub Form_Load()
    If TCPIPStartup Then
        cmdBind.Enabled = True
        cmdSend.Enabled = False
        cmdClose.Enabled = False
        Timer1.Enabled = False
    Else
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError
    End If
    '
    ' Create the event we'll use for detecting incoming datagrams.
    '
    msg_sock = INVALID_SOCKET
    hEvent = WSACreateEvent
    If hEvent = 0 Then
        MsgBox "Failed to create event. Error: " & Err.LastDllError
    End If
    
End Sub

'
' Subroutine: Form_Unload
'
' Description:
'    This routine unloads Winsock in addition to freeing any resources.
'
Private Sub Form_Unload(Cancel As Integer)
    If msg_sock <> INVALID_SOCKET Then
        closesocket msg_sock
    End If
    TCPIPShutDown
    If hEvent <> 0 Then WSACloseEvent hEvent
End Sub

'
' Subroutine: Timer1_Timer
'
' Description:
'    This is the callback routine for the Timer object. When the
'    timer is triggered this function registers for the FD_READ
'    event and then calls WSAWaitForMultipleEvents to determine
'    if this event condition exists (meaning there is data to be
'    read). If not then leave the routine. Otherwise call the
'    recvfrom function to read the datagram.
'
Private Sub Timer1_Timer()
    If msg_sock = INVALID_SOCKET Then
        Debug.Print "Create Socket First"
        Exit Sub
    End If
    
    Dim RetMsg As String
    
    RetMsg = String(7000, 0)
    ' Register to see if data is present
    '
    dwRc = WSAEventSelect(msg_sock, hEvent, FD_READ)
    If (dwRc = SOCKET_ERROR) Then
        MsgBox "Failed to select event. Error: " & Err.LastDllError
        Exit Sub
    End If
    '
    ' See if there is data to be read but don't block (that is,
    '  the wait parameter is 0 meaning return immediately).
    '
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

