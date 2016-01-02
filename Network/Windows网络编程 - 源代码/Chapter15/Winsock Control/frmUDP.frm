VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form frmUDP 
   Caption         =   "Datagram Sockets"
   ClientHeight    =   4770
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   8880
   LinkTopic       =   "Form1"
   ScaleHeight     =   4770
   ScaleWidth      =   8880
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdExit 
      Caption         =   "Exit"
      Height          =   375
      Left            =   6600
      TabIndex        =   22
      Top             =   4080
      Width           =   1455
   End
   Begin VB.Frame Frame3 
      Caption         =   "Winsock Information"
      Height          =   3735
      Left            =   5880
      TabIndex        =   17
      Top             =   120
      Width           =   2895
      Begin VB.Timer Timer1 
         Left            =   2040
         Top             =   3120
      End
      Begin VB.Label lblRemotePort 
         Height          =   255
         Left            =   1320
         TabIndex        =   30
         Top             =   2640
         Width           =   1455
      End
      Begin VB.Label Label12 
         Caption         =   " Remote Port:"
         Height          =   255
         Left            =   240
         TabIndex        =   29
         Top             =   2640
         Width           =   975
      End
      Begin VB.Label lblRemoteIP 
         Height          =   255
         Left            =   1320
         TabIndex        =   28
         Top             =   2280
         Width           =   1455
      End
      Begin VB.Label Label11 
         Caption         =   " Remote IP:"
         Height          =   255
         Left            =   360
         TabIndex        =   27
         Top             =   2280
         Width           =   855
      End
      Begin VB.Label lblReceiverState 
         Caption         =   "sckClosed"
         Height          =   255
         Left            =   1320
         TabIndex        =   26
         Top             =   1440
         Width           =   1455
      End
      Begin VB.Label lblSenderState 
         Caption         =   "sckClosed"
         Height          =   255
         Left            =   1320
         TabIndex        =   25
         Top             =   1080
         Width           =   1455
      End
      Begin VB.Label Label10 
         Caption         =   "Receiver State:"
         Height          =   255
         Left            =   120
         TabIndex        =   24
         Top             =   1440
         Width           =   1215
      End
      Begin VB.Label Label9 
         Caption         =   "Sender State:"
         Height          =   255
         Left            =   240
         TabIndex        =   23
         Top             =   1080
         Width           =   1095
      End
      Begin VB.Label lblLocalIP 
         Height          =   255
         Left            =   1320
         TabIndex        =   21
         Top             =   720
         Width           =   1335
      End
      Begin VB.Label Label8 
         Caption         =   "Local IP:"
         Height          =   255
         Left            =   600
         TabIndex        =   20
         Top             =   720
         Width           =   735
      End
      Begin VB.Label lblHostName 
         Height          =   255
         Left            =   1320
         TabIndex        =   19
         Top             =   360
         Width           =   1455
      End
      Begin VB.Label Label7 
         Caption         =   " Local Name:"
         Height          =   255
         Left            =   240
         TabIndex        =   18
         Top             =   360
         Width           =   975
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Receiver"
      Height          =   2295
      Left            =   120
      TabIndex        =   5
      Top             =   2280
      Width           =   5655
      Begin VB.CommandButton cmdCloseListen 
         Caption         =   "Close Listen"
         Height          =   375
         Left            =   3975
         TabIndex        =   16
         Top             =   1065
         Width           =   1455
      End
      Begin VB.TextBox txtRecvLocalPort 
         Height          =   285
         Left            =   2205
         TabIndex        =   9
         Text            =   "5150"
         Top             =   360
         Width           =   1575
      End
      Begin VB.CommandButton cmdListen 
         Caption         =   "Listen"
         Height          =   375
         Left            =   3975
         TabIndex        =   8
         Top             =   465
         Width           =   1455
      End
      Begin VB.ListBox lstRecvData 
         Height          =   1425
         Left            =   120
         TabIndex        =   6
         Top             =   720
         Width           =   3645
      End
      Begin MSWinsockLib.Winsock sockRecv 
         Left            =   4935
         Top             =   1665
         _ExtentX        =   741
         _ExtentY        =   741
         _Version        =   393216
      End
      Begin VB.Label Label3 
         Caption         =   "Port:"
         Height          =   255
         Left            =   1725
         TabIndex        =   10
         Top             =   360
         Width           =   375
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Sender"
      Height          =   1935
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   5655
      Begin VB.CommandButton cmdCloseSend 
         Caption         =   "Close Socket"
         Height          =   375
         Left            =   3960
         TabIndex        =   15
         Top             =   840
         Width           =   1455
      End
      Begin VB.TextBox txtSendRemotePort 
         Height          =   285
         Left            =   1800
         TabIndex        =   13
         Text            =   "5150"
         Top             =   1080
         Width           =   1335
      End
      Begin VB.TextBox txtSendData 
         Height          =   285
         Left            =   1800
         TabIndex        =   11
         Text            =   "This is a test of the emergency broadcasting..."
         Top             =   1440
         Width           =   3615
      End
      Begin VB.CommandButton cmdSendDgram 
         Caption         =   "Send Datagram"
         Height          =   375
         Left            =   3960
         TabIndex        =   7
         Top             =   360
         Width           =   1455
      End
      Begin VB.TextBox txtSendLocalPort 
         Height          =   285
         Left            =   1800
         TabIndex        =   3
         Text            =   "0"
         Top             =   720
         Width           =   1335
      End
      Begin VB.TextBox txtRecipientIP 
         Height          =   285
         Left            =   1800
         TabIndex        =   1
         Text            =   "157.54.178.12"
         Top             =   360
         Width           =   1695
      End
      Begin MSWinsockLib.Winsock sockSend 
         Left            =   120
         Top             =   1440
         _ExtentX        =   741
         _ExtentY        =   741
         _Version        =   393216
      End
      Begin VB.Label Label5 
         Alignment       =   1  'Right Justify
         Caption         =   "Remote Port:"
         Height          =   255
         Left            =   720
         TabIndex        =   14
         Top             =   1080
         Width           =   975
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Message:"
         Height          =   255
         Left            =   960
         TabIndex        =   12
         Top             =   1440
         Width           =   735
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "Port:"
         Height          =   255
         Left            =   1320
         TabIndex        =   4
         Top             =   720
         Width           =   375
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "Recipient's Name/IP:"
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   360
         Width           =   1575
      End
   End
End
Attribute VB_Name = "frmUDP"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub cmdSendDgram_Click()
    ' If the socket state is closed, we need to bind to a local
    ' port and also to the remote host's IP address and port
    If (sockSend.State = sckClosed) Then
        sockSend.RemoteHost = txtRecipientIP.Text
        sockSend.RemotePort = CInt(txtSendRemotePort.Text)
        sockSend.Bind CInt(txtSendLocalPort.Text)
        
        cmdCloseSend.Enabled = True
    End If
    '
    ' Now we can send the data
    '
    sockSend.SendData txtSendData.Text
End Sub

Private Sub cmdListen_Click()
    ' Bind to the local port
    '
    sockRecv.Bind CInt(txtRecvLocalPort.Text)
    '
    ' Disable this button since it would be an error to bind
    ' twice (a close needs to be done before rebinding occurs)
    '
    cmdListen.Enabled = False
    cmdCloseListen.Enabled = True
End Sub

Private Sub cmdCloseSend_Click()
    ' Close the sending socket, and disable the Close button
    '
    sockSend.Close
    cmdCloseSend.Enabled = False
End Sub

Private Sub cmdCloseListen_Click()
    ' Close the listening socket
    '
    sockRecv.Close
    ' Enable the right buttons
    '
    cmdListen.Enabled = True
    cmdCloseListen.Enabled = False
    lstRecvData.Clear
End Sub

Private Sub Form_Load()
    ' Initialize the socket protocols, and set up some default
    ' labels and values
    '
    sockSend.Protocol = sckUDPProtocol
    sockRecv.Protocol = sckUDPProtocol
    
    lblHostName.Caption = sockSend.LocalHostName
    lblLocalIP.Caption = sockSend.LocalIP
    
    cmdCloseListen.Enabled = False
    cmdCloseSend.Enabled = False
    
    Timer1.Interval = 500
    Timer1.Enabled = True
End Sub

Private Sub sockSend_Error(ByVal Number As Integer, _
        Description As String, ByVal Scode As Long, _
        ByVal Source As String, ByVal HelpFile As String, _
        ByVal HelpContext As Long, CancelDisplay As Boolean)
    MsgBox Description
End Sub

Private Sub sockRecv_DataArrival(ByVal bytesTotal As Long)
    Dim data As String
    
    ' Allocate a string of sufficient size and get the data,
    ' then add it to the list box.
    data = String(bytesTotal + 2, Chr$(0))
    sockRecv.GetData data, , bytesTotal
    lstRecvData.AddItem data
    ' Update the remote IP and port labels
    '
    lblRemoteIP.Caption = sockRecv.RemoteHostIP
    lblRemotePort.Caption = sockRecv.RemotePort
End Sub

Private Sub sockRecv_Error(ByVal Number As Integer, _
        Description As String, ByVal Scode As Long, _
        ByVal Source As String, ByVal HelpFile As String, _
        ByVal HelpContext As Long, CancelDisplay As Boolean)
    MsgBox Description
End Sub

Private Sub Timer1_Timer()
    ' When the timer goes off, update the socket status labels
    '
    Select Case sockSend.State
        Case sckClosed
            lblSenderState.Caption = "sckClosed"
        Case sckOpen
            lblSenderState.Caption = "sckOpen"
        Case sckListening
            lblSenderState.Caption = "sckListening"
        Case sckConnectionPending
            lblSenderState.Caption = "sckConnectionPending"
        Case sckResolvingHost
            lblSenderState.Caption = "sckResolvingHost"
        Case sckHostResolved
            lblSenderState.Caption = "sckHostResolved"
        Case sckConnecting
            lblSenderState.Caption = "sckConnecting"
        Case sckClosing
            lblSenderState.Caption = "sckClosing"
        Case sckError
            lblSenderState.Caption = "sckError"
        Case Else
            lblSenderState.Caption = "unknown"
    End Select
    Select Case sockRecv.State
        Case sckClosed
            lblReceiverState.Caption = "sckClosed"
        Case sckOpen
            lblReceiverState.Caption = "sckOpen"
        Case sckListening
            lblReceiverState.Caption = "sckListening"
        Case sckConnectionPending
            lblReceiverState.Caption = "sckConnectionPending"
        Case sckResolvingHost
            lblReceiverState.Caption = "sckResolvingHost"
        Case sckHostResolved
            lblReceiverState.Caption = "sckHostResolved"
        Case sckConnecting
            lblReceiverState.Caption = "sckConnecting"
        Case sckClosing
            lblReceiverState.Caption = "sckClosing"
        Case sckError
            lblReceiverState.Caption = "sckError"
        Case Else
            lblReceiverState.Caption = "unknown"
    End Select
End Sub
