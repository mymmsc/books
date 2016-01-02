VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "COMCTL32.OCX"
Begin VB.Form Form1 
   Caption         =   "TCP Client/Server"
   ClientHeight    =   6165
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9960
   LinkTopic       =   "Form1"
   ScaleHeight     =   6165
   ScaleWidth      =   9960
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer Timer1 
      Left            =   5880
      Top             =   5640
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "Exit"
      Height          =   375
      Left            =   7080
      TabIndex        =   3
      Top             =   5640
      Width           =   1575
   End
   Begin VB.Frame Frame3 
      Caption         =   "Winsock Information"
      Height          =   5295
      Left            =   5640
      TabIndex        =   2
      Top             =   120
      Width           =   4215
      Begin ComctlLib.ListView lstStates 
         Height          =   3735
         Left            =   120
         TabIndex        =   19
         Top             =   1440
         Width           =   3975
         _ExtentX        =   7011
         _ExtentY        =   6588
         View            =   3
         LabelWrap       =   -1  'True
         HideSelection   =   0   'False
         _Version        =   327682
         ForeColor       =   -2147483640
         BackColor       =   -2147483643
         BorderStyle     =   1
         Appearance      =   1
         NumItems        =   3
         BeginProperty ColumnHeader(1) {0713E8C7-850A-101B-AFC0-4210102A8DA7} 
            Key             =   ""
            Object.Tag             =   ""
            Text            =   "Socket"
            Object.Width           =   1940
         EndProperty
         BeginProperty ColumnHeader(2) {0713E8C7-850A-101B-AFC0-4210102A8DA7} 
            SubItemIndex    =   1
            Key             =   ""
            Object.Tag             =   ""
            Text            =   "State"
            Object.Width           =   3069
         EndProperty
         BeginProperty ColumnHeader(3) {0713E8C7-850A-101B-AFC0-4210102A8DA7} 
            SubItemIndex    =   2
            Key             =   ""
            Object.Tag             =   ""
            Text            =   "Port"
            Object.Width           =   2540
         EndProperty
      End
      Begin VB.Label Label8 
         Caption         =   "State Information:"
         Height          =   255
         Left            =   120
         TabIndex        =   24
         Top             =   1200
         Width           =   1575
      End
      Begin VB.Label lblLocalHostIP 
         Height          =   255
         Left            =   1680
         TabIndex        =   23
         Top             =   720
         Width           =   2175
      End
      Begin VB.Label lblLocalHostname 
         Height          =   255
         Left            =   1680
         TabIndex        =   22
         Top             =   360
         Width           =   2175
      End
      Begin VB.Label Label7 
         Caption         =   "Local Host Name:"
         Height          =   255
         Left            =   240
         TabIndex        =   21
         Top             =   360
         Width           =   1290
      End
      Begin VB.Label Label6 
         Caption         =   "Local IP:"
         Height          =   255
         Left            =   885
         TabIndex        =   20
         Top             =   705
         Width           =   735
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "TCP Client"
      Height          =   2535
      Left            =   120
      TabIndex        =   1
      Top             =   3480
      Width           =   5295
      Begin VB.CommandButton cmdSendData 
         Caption         =   "Send Data"
         Height          =   375
         Left            =   3840
         TabIndex        =   14
         Top             =   1920
         Width           =   1335
      End
      Begin VB.CommandButton cmdDisconnect 
         Caption         =   "Disconnect"
         Height          =   375
         Left            =   3840
         TabIndex        =   13
         Top             =   720
         Width           =   1335
      End
      Begin VB.CommandButton cmdConnect 
         Caption         =   "Connect"
         Height          =   375
         Left            =   3840
         TabIndex        =   12
         Top             =   240
         Width           =   1335
      End
      Begin VB.TextBox txtSendData 
         Height          =   975
         Left            =   120
         MultiLine       =   -1  'True
         TabIndex        =   10
         Top             =   1440
         Width           =   3495
      End
      Begin VB.TextBox txtPort 
         Height          =   285
         Left            =   1200
         TabIndex        =   8
         Text            =   "5150"
         Top             =   720
         Width           =   1095
      End
      Begin VB.TextBox txtServerName 
         Height          =   285
         Left            =   1200
         TabIndex        =   6
         Text            =   "riven"
         Top             =   360
         Width           =   1935
      End
      Begin MSWinsockLib.Winsock sockClient 
         Left            =   4560
         Top             =   1200
         _ExtentX        =   741
         _ExtentY        =   741
         _Version        =   393216
      End
      Begin VB.Label Label4 
         Caption         =   "Message:"
         Height          =   255
         Left            =   120
         TabIndex        =   11
         Top             =   1200
         Width           =   1095
      End
      Begin VB.Label Label3 
         Caption         =   "Remote Port:"
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Top             =   720
         Width           =   975
      End
      Begin VB.Label Label2 
         Caption         =   "Server Name:"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   360
         Width           =   975
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "TCP Server"
      Height          =   3135
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   5295
      Begin VB.CommandButton cmdCloseListen 
         Caption         =   "Close Listen"
         Height          =   375
         Left            =   3960
         TabIndex        =   17
         Top             =   720
         Width           =   1215
      End
      Begin VB.CommandButton cmdListen 
         Caption         =   "Listen"
         Height          =   375
         Left            =   3960
         TabIndex        =   16
         Top             =   240
         Width           =   1215
      End
      Begin VB.ListBox lstMessages 
         Height          =   1620
         Left            =   120
         TabIndex        =   15
         Top             =   1200
         Width           =   5055
      End
      Begin VB.TextBox txtServerPort 
         Height          =   285
         Left            =   1080
         TabIndex        =   5
         Text            =   "5150"
         Top             =   360
         Width           =   1575
      End
      Begin MSWinsockLib.Winsock sockServer 
         Index           =   0
         Left            =   3120
         Top             =   360
         _ExtentX        =   741
         _ExtentY        =   741
         _Version        =   393216
      End
      Begin VB.Label Label5 
         Caption         =   "Messages:"
         Height          =   255
         Left            =   120
         TabIndex        =   18
         Top             =   960
         Width           =   1215
      End
      Begin VB.Label Label1 
         Caption         =   "Port:"
         Height          =   255
         Left            =   600
         TabIndex        =   4
         Top             =   360
         Width           =   375
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

' The index value of the last Winsock control dynamically loaded
' in the sockServer array
Private ServerIndex As Long

Private Sub cmdCloseListen_Click()
    Dim itemx As Object
    ' Close the server's listening socket. No more
    ' clients will be allowed to connect.
    '
    sockServer(0).Close
    cmdListen.Enabled = True
    cmdCloseListen.Enabled = False
    
    Set itemx = lstStates.ListItems.Item(2)
    itemx.SubItems(2) = "-1"
End Sub

Private Sub cmdConnect_Click()
    ' Have the client control attempt to connect to the
    ' specified server on the given port number
    '
    sockClient.LocalPort = 0
    sockClient.RemoteHost = txtServerName.Text
    sockClient.RemotePort = CInt(txtPort.Text)
    sockClient.Connect
    
    cmdConnect.Enabled = False
End Sub

Private Sub cmdDisconnect_Click()
    Dim itemx As Object
    ' Close the client's connection and set up the command
    ' buttons for subsequent connections
    '
    sockClient.Close
    
    cmdConnect.Enabled = True
    cmdSendData.Enabled = False
    cmdDisconnect.Enabled = False
    ' Set the port number to -1 to indicate no connection
    '
    Set itemx = lstStates.ListItems.Item(1)
    itemx.SubItems(2) = "-1"
End Sub

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub cmdListen_Click()
    Dim itemx As Object
    ' Put the server control into listening mode on the given
    ' port number
    '
    sockServer(0).LocalPort = CInt(txtServerPort.Text)
    sockServer(0).Listen
    
    Set itemx = lstStates.ListItems.Item(2)
    itemx.SubItems(2) = sockServer(0).LocalPort
    
    cmdCloseListen.Enabled = True
    cmdListen.Enabled = False
End Sub

Private Sub cmdSendData_Click()
    ' If we're connected, send the given data to the server
    '
    If (sockClient.State = sckConnected) Then
        sockClient.SendData txtSendData.Text
    Else
        MsgBox "Unexpected error! Connection closed"
        Call cmdDisconnect_Click
    End If
End Sub

Private Sub Form_Load()
    Dim itemx As Object

    lblLocalHostname.Caption = sockServer(0).LocalHostName
    lblLocalHostIP.Caption = sockServer(0).LocalIP
    
    ' Initialize the Protocol property to TCP since that's
    ' all we'll be using
    '
    ServerIndex = 0
    sockServer(0).Protocol = sckTCPProtocol
    sockClient.Protocol = sckTCPProtocol
    ' Set up the buttons
    '
    cmdDisconnect.Enabled = False
    cmdSendData.Enabled = False
    cmdCloseListen.Enabled = False
    ' Initialize the ListView control that contains the
    ' current state of all Winsock controls created (not
    ' necessarily connected or being used)
    '
    Set itemx = lstStates.ListItems.Add(1, , "Local Client")
    itemx.SubItems(1) = "sckClosed"
    itemx.SubItems(2) = "-1"
    Set itemx = lstStates.ListItems.Add(2, , "Local Server")
    itemx.SubItems(1) = "sckClosed"
    itemx.SubItems(2) = "-1"
    ' Initialize the timer, which controls the rate of refresh
    ' on the above socket states
    '
    Timer1.Interval = 500
    Timer1.Enabled = True
End Sub


Private Sub sockClient_Close()
    sockClient.Close
End Sub

Private Sub sockClient_Connect()
    Dim itemx As Object
    
    ' The connection was successful: enable the transfer data
    ' buttons
    cmdSendData.Enabled = True
    cmdDisconnect.Enabled = True
    
    Set itemx = lstStates.ListItems.Item(1)
    itemx.SubItems(2) = sockClient.LocalPort
End Sub

Private Sub sockClient_Error(ByVal Number As Integer, _
        Description As String, ByVal Scode As Long, _
        ByVal Source As String, ByVal HelpFile As String, _
        ByVal HelpContext As Long, CancelDisplay As Boolean)
    ' An error occured on the Client control: print a message,
    ' and close the control. An error puts the control in the
    ' sckError state, which is cleared only when the Close
    ' method is called.
    MsgBox Description
    sockClient.Close
    cmdConnect.Enabled = True
End Sub

Private Sub sockServer_Close(index As Integer)
    Dim itemx As Object
    ' Close the given Winsock control
    '
    sockServer(index).Close
    
    Set itemx = lstStates.ListItems.Item(index + 2)
    lstStates.ListItems.Item(index + 2).Text = "---.---.---.---"
    itemx.SubItems(2) = "-1"
            
End Sub

Private Sub sockServer_ConnectionRequest(index As Integer, _
        ByVal requestID As Long)
    Dim i As Long, place As Long, freeSock As Long, itemx As Object
    
    ' Search through the array to see wether there is a closed
    ' control that we can reuse
    freeSock = 0
    For i = 1 To ServerIndex
        If sockServer(i).State = sckClosed Then
            freeSock = i
            Exit For
        End If
    Next i
    ' If freeSock is still 0, there are no free controls
    ' so load a new one
    '
    If freeSock = 0 Then
        ServerIndex = ServerIndex + 1
        Load sockServer(ServerIndex)
    
        sockServer(ServerIndex).Accept requestID
        place = ServerIndex
    Else
        sockServer(freeSock).Accept requestID
        place = freeSock
    End If
    '  If no free controls were found, we added one above.
    '  Create an entry in the ListView control for the new
    '  control.  In either case set the state of the new
    '  connection to sckConnected.
    '
    If freeSock = 0 Then
        Set itemx = lstStates.ListItems.Add(, , _
            sockServer(ServerIndex).RemoteHostIP)
    Else
        Set itemx = lstStates.ListItems.Item(freeSock + 2)
        lstStates.ListItems.Item(freeSock + 2).Text = _
            sockServer(freeSock).RemoteHostIP
    End If
    itemx.SubItems(2) = sockServer(place).RemotePort
    
End Sub

Private Sub sockServer_DataArrival(index As Integer, _
        ByVal bytesTotal As Long)
    Dim data As String, entry As String
    
    ' Allocate a large enough string buffer and get the
    ' data
    '
    data = String(bytesTotal + 2, Chr$(0))
    sockServer(index).GetData data, vbString, bytesTotal
    ' Add the client's IP address to the beginning of the
    ' message and add the message to the list box
    '
    entry = sockServer(index).RemoteHostIP & ": " & data
    lstMessages.AddItem entry
End Sub

Private Sub sockServer_Error(index As Integer, _
        ByVal Number As Integer, Description As String, _
        ByVal Scode As Long, ByVal Source As String, _
        ByVal HelpFile As String, ByVal HelpContext As Long, _
        CancelDisplay As Boolean)
    ' Print the error message and close the specified control.
    ' An error puts the control in the sckError state, which
    ' is cleared only when the Close method is called.
    MsgBox Description
    sockServer(index).Close
End Sub

Private Sub Timer1_Timer()
    Dim i As Long, index As Long, itemx As Object
    
    ' Set the state of the local client Winsock control
    '
    Set itemx = lstStates.ListItems.Item(1)
    Select Case sockClient.State
        Case sckClosed
            itemx.SubItems(1) = "sckClosed"
        Case sckOpen
            itemx.SubItems(1) = "sckOpen"
        Case sckListening
            itemx.SubItems(1) = "sckListening"
        Case sckConnectionPending
            itemx.SubItems(1) = "sckConnectionPending"
        Case sckResolvingHost
            itemx.SubItems(1) = "sckResolvingHost"
        Case sckHostResolved
            itemx.SubItems(1) = "sckHostResolved"
        Case sckConnecting
            itemx.SubItems(1) = "sckConnecting"
        Case sckConnected
            itemx.SubItems(1) = "sckConnected"
        Case sckClosing
            itemx.SubItems(1) = "sckClosing"
        Case sckError
            itemx.SubItems(1) = "sckError"
        Case Else
            itemx.SubItems(1) = "unknown: " & sockClient.State
    End Select
    ' Now set the states for the listening server control as
    ' well as any connected clients
    '
    index = 0
    For i = 2 To ServerIndex + 2
        Set itemx = lstStates.ListItems.Item(i)
        
        Select Case sockServer(index).State
            Case sckClosed
                itemx.SubItems(1) = "sckClosed"
            Case sckOpen
                itemx.SubItems(1) = "sckOpen"
            Case sckListening
                itemx.SubItems(1) = "sckListening"
            Case sckConnectionPending
                itemx.SubItems(1) = "sckConnectionPending"
            Case sckResolvingHost
                itemx.SubItems(1) = "sckResolvingHost"
            Case sckHostResolved
                itemx.SubItems(1) = "sckHostResolved"
            Case sckConnecting
                itemx.SubItems(1) = "sckConnecting"
            Case sckConnected
                itemx.SubItems(1) = "sckConnected"
            Case sckClosing
                itemx.SubItems(1) = "sckClosing"
            Case sckError
                itemx.SubItems(1) = "sckError"
            Case Else
                itemx.SubItems(1) = "unknown"
        End Select
        index = index + 1
    Next i
End Sub
