VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   2775
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9315
   LinkTopic       =   "Form1"
   ScaleHeight     =   2775
   ScaleWidth      =   9315
   Begin VB.Frame Frame2 
      Caption         =   "Winsock Information"
      Height          =   2535
      Left            =   6960
      TabIndex        =   18
      Top             =   120
      Width           =   2295
      Begin VB.Label lblLocalIP 
         Height          =   255
         Left            =   960
         TabIndex        =   22
         Top             =   600
         Width           =   1215
      End
      Begin VB.Label lblState 
         Caption         =   "0"
         Height          =   255
         Left            =   1200
         TabIndex        =   21
         Top             =   240
         Width           =   855
      End
      Begin VB.Label Label5 
         Caption         =   "Socket State:"
         Height          =   255
         Left            =   120
         TabIndex        =   20
         Top             =   240
         Width           =   1095
      End
      Begin VB.Label Label7 
         Caption         =   "Local IP:"
         Height          =   255
         Left            =   120
         TabIndex        =   19
         Top             =   600
         Width           =   735
      End
   End
   Begin VB.CommandButton cmdCloseListen 
      Caption         =   "Close Listen"
      Height          =   255
      Left            =   5640
      TabIndex        =   17
      Top             =   480
      Width           =   1215
   End
   Begin VB.CommandButton cmdListen 
      Caption         =   "Listen"
      Height          =   255
      Left            =   5640
      TabIndex        =   16
      Top             =   120
      Width           =   1215
   End
   Begin VB.ListBox List1 
      Height          =   1230
      Left            =   3600
      TabIndex        =   14
      Top             =   1440
      Width           =   3255
   End
   Begin VB.Timer Timer1 
      Left            =   5520
      Top             =   840
   End
   Begin VB.TextBox txtLocalPort 
      Height          =   285
      Left            =   2640
      TabIndex        =   12
      Text            =   "0"
      Top             =   840
      Width           =   1215
   End
   Begin VB.Frame Frame1 
      Caption         =   "Socket Type"
      Height          =   1095
      Left            =   120
      TabIndex        =   9
      Top             =   0
      Width           =   1215
      Begin VB.OptionButton optIRDA 
         Caption         =   "IrDA"
         Height          =   255
         Left            =   120
         TabIndex        =   11
         Top             =   720
         Width           =   735
      End
      Begin VB.OptionButton optTCP 
         Caption         =   "TCP"
         Height          =   255
         Left            =   120
         TabIndex        =   10
         Top             =   240
         Width           =   735
      End
   End
   Begin VB.TextBox txtSendData 
      Height          =   1215
      Left            =   120
      MultiLine       =   -1  'True
      TabIndex        =   7
      Top             =   1440
      Width           =   3135
   End
   Begin VB.CommandButton cmdSendData 
      Caption         =   "SendData"
      Height          =   255
      Left            =   4080
      TabIndex        =   6
      Top             =   840
      Width           =   1335
   End
   Begin VB.CommandButton cmdDisconnect 
      Caption         =   "Disconnect"
      Height          =   255
      Left            =   4080
      TabIndex        =   5
      Top             =   480
      Width           =   1335
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   255
      Left            =   4080
      TabIndex        =   4
      Top             =   120
      Width           =   1335
   End
   Begin VB.TextBox txtPort 
      Height          =   285
      Left            =   2640
      TabIndex        =   1
      Text            =   "5150"
      Top             =   480
      Width           =   1215
   End
   Begin VB.TextBox txtServerName 
      Height          =   285
      Left            =   2640
      TabIndex        =   0
      Text            =   "riven"
      Top             =   120
      Width           =   1215
   End
   Begin VB.PictureBox WinSock1 
      Height          =   480
      Left            =   6000
      ScaleHeight     =   420
      ScaleWidth      =   1140
      TabIndex        =   23
      Top             =   840
      Width           =   1200
   End
   Begin VB.Label Label6 
      Caption         =   "Incoming Messages:"
      Height          =   255
      Left            =   3600
      TabIndex        =   15
      Top             =   1200
      Width           =   1455
   End
   Begin VB.Label Label4 
      Caption         =   " Local Port:"
      Height          =   255
      Left            =   1800
      TabIndex        =   13
      Top             =   840
      Width           =   855
   End
   Begin VB.Label Label3 
      Caption         =   "Message:"
      Height          =   255
      Left            =   120
      TabIndex        =   8
      Top             =   1200
      Width           =   735
   End
   Begin VB.Label Label2 
      Caption         =   "Port:"
      Height          =   255
      Left            =   2280
      TabIndex        =   3
      Top             =   480
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   " Service/Server:"
      Height          =   255
      Left            =   1440
      TabIndex        =   2
      Top             =   120
      Width           =   1215
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

' This global variable is used to retain the current value
' of the radio buttons.  0 corresponds to TCP while 2 means
' IrDA (infrared).  Note that UDP is not supported by the
' control currently.
Public SocketType

Private Sub cmdCloseListen_Click()
' Close the listening socket and setup the other buttons
' back to the start state
    WinSock1.Close
    
    cmdConnect.Enabled = True
    cmdListen.Enabled = True
    cmdDisconnect.Enabled = False
    cmdSendData.Enabled = False
    cmdCloseListen.Enabled = False
End Sub

Private Sub cmdConnect_Click()
' Check what type of socket type was chosen and initiate
' the given connection.

    If SocketType = 0 Then
        ' Set the protocol and the remote host name and port
        '
        WinSock1.Protocol = 0
        
        WinSock1.RemoteHost = txtServerName.Text
        WinSock1.RemotePort = CInt(txtPort.Text)
        WinSock1.LocalPort = 0
        
        WinSock1.Connect
    ElseIf SocketType = 2 Then
        ' Set the protocol to IrDA and set the service name
        '
        WinSock1.Protocol = 2
        'WinSock1.LocalPort = 0
        'WinSock1.ServiceName = txtServerName.Text
        WinSock1.RemoteHost = txtServerName.Text
        
        WinSock1.Connect
    End If
    ' Make sure the connection was successful, if so
    ' enable/disable some commands
    '
    MsgBox WinSock1.State
    If (WinSock1.State = 7) Then
        cmdConnect.Enabled = False
        cmdListen.Enabled = False
        cmdDisconnect.Enabled = True
        cmdSendData.Enabled = True
    Else
        MsgBox "Connect failed"
        WinSock1.Close
    End If
End Sub

Private Sub cmdDisconnect_Click()
' Close the current client connection and reset the
' buttons back to the start state
    WinSock1.Close
    
    cmdConnect.Enabled = True
    cmdListen.Enabled = True
    cmdDisconnect.Enabled = False
    cmdSendData.Enabled = False
    cmdCloseListen.Enabled = False
End Sub

Private Sub cmdListen_Click()
' Set the socket into listening mode for the given protocl
' type
'
    If SocketType = 0 Then
        WinSock1.Protocol = 0
        WinSock1.LocalPort = CInt(txtLocalPort.Text)
        WinSock1.Listen
    ElseIf SocketType = 2 Then
        WinSock1.Protocol = 2
        WinSock1.ServiceName = txtServerName.Text
        WinSock1.Listen
    End If
    ' If we're not in listening mode now, then something
    ' went wrong
    '
    If (WinSock1.State = 2) Then
        cmdConnect.Enabled = False
        cmdListen.Enabled = False
        cmdCloseListen.Enabled = True
    Else
        MsgBox "Unable to listen!"
    End If
End Sub

Private Sub cmdSendData_Click()
' Send the data in the box on the current connection
'
    WinSock1.SendData txtSendData.Text
End Sub

Private Sub Form_Load()
' Setup the initial values for the buttons, Timer, etc.
'
    optTCP.Value = True
    SocketType = 0
        
    Timer1.Interval = 750
    Timer1.Enabled = True
    
    cmdConnect.Enabled = True
    cmdListen.Enabled = True
    
    cmdDisconnect.Enabled = False
    cmdSendData.Enabled = False
    cmdCloseListen.Enabled = False
    
    lblLocalIP.Caption = WinSock1.LocalIP
End Sub

Private Sub optIRDA_Click()
' Set the socket type to IrDA
'
    optIRDA.Value = True
    SocketType = 2
End Sub

Private Sub optTCP_Click()
' Set the socket type to TCP
'
    optTCP.Value = True
    SocketType = 0
    cmdConnect.Caption = "Connect"
End Sub

Private Sub Timer1_Timer()
' This is the event that gets called each time the
' timer expires. Update the socket state label.
'
    Select Case WinSock1.State
        Case 0
            lblState.Caption = "sckClosed"
        Case 1
            lblState.Caption = "sckOpen"
        Case 2
            lblState.Caption = "sckListening"
        Case 3
            lblState.Caption = "sckConnectionPending"
        Case 4
            lblState.Caption = "sckResolvingHost"
        Case 5
            lblState.Caption = "sckHostResolved"
        Case 6
            lblState.Caption = "sckConnecting"
        Case 7
            lblState.Caption = "sckConnected"
        Case 8
            lblState.Caption = "sckClosing"
        Case 9
            lblState.Caption = "sckError"
    End Select
End Sub

Private Sub WinSock1_Close()
' The other side initiated a close so we'll close our end.
' Reset the buttons back to their initial state.
'
    WinSock1.Close
    
    cmdConnect.Enabled = True
    cmdListen.Enabled = True
    
    cmdDisconnect.Enabled = False
    cmdSendData.Enabled = False
    cmdCloseListen.Enabled = False
End Sub

Private Sub WinSock1_ConnectionRequest()
' We got a client connection, accept it on the listening
' socket
'
    WinSock1.Accept
End Sub

Private Sub WinSock1_DataArrival(ByVal bytesTotal)
' This is the event for data arrival.  Get the data and
' add it to the listbox.
'
    Dim rdata
    
    WinSock1.GetData rdata
    List1.AddItem rdata
End Sub

Private Sub WinSock1_Error(ByVal number, ByVal description)
' An error occured, display the message and close the socket
'
    MsgBox description
    Call WinSock1_Close
End Sub

