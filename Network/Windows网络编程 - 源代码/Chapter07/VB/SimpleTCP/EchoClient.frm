VERSION 5.00
Begin VB.Form EchoCli 
   Caption         =   "TCP Echo Client"
   ClientHeight    =   3525
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5775
   LinkTopic       =   "Form1"
   ScaleHeight     =   3525
   ScaleWidth      =   5775
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdDisconnect 
      Caption         =   "Disconnect"
      Height          =   495
      Left            =   3960
      TabIndex        =   8
      Top             =   2640
      Width           =   1335
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   495
      Left            =   3960
      TabIndex        =   7
      Top             =   1080
      Width           =   1335
   End
   Begin VB.TextBox txtPort 
      Height          =   285
      Left            =   5040
      TabIndex        =   6
      Text            =   "5000"
      Top             =   240
      Width           =   735
   End
   Begin VB.TextBox txtServer 
      Height          =   375
      Left            =   1800
      TabIndex        =   3
      Text            =   "weipiii"
      Top             =   240
      Width           =   1575
   End
   Begin VB.CommandButton cmdSendRecv 
      Caption         =   "Send && Receive Text"
      Default         =   -1  'True
      Enabled         =   0   'False
      Height          =   495
      Left            =   3960
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   1920
      Width           =   1335
   End
   Begin VB.TextBox txtRecv 
      Height          =   975
      Left            =   240
      MultiLine       =   -1  'True
      TabIndex        =   1
      Top             =   2280
      Width           =   3135
   End
   Begin VB.TextBox txtSend 
      Height          =   975
      Left            =   240
      MultiLine       =   -1  'True
      TabIndex        =   0
      Top             =   960
      Width           =   3135
   End
   Begin VB.Label Label4 
      Caption         =   "Recv:"
      Height          =   255
      Left            =   240
      TabIndex        =   10
      Top             =   2040
      Width           =   615
   End
   Begin VB.Label Label3 
      Caption         =   "Send:"
      Height          =   255
      Left            =   240
      TabIndex        =   9
      Top             =   720
      Width           =   1215
   End
   Begin VB.Label Label2 
      Caption         =   "Remote Port:"
      Height          =   375
      Left            =   3600
      TabIndex        =   5
      Top             =   240
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "Remote Server:"
      Height          =   255
      Left            =   240
      TabIndex        =   4
      Top             =   240
      Width           =   1335
   End
End
Attribute VB_Name = "EchoCli"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: vbtcp
'
' Description:
'    This application contains both a TCP server and client. The server creates
'    a TCP listening socket and waits for client connections. Once a connection
'    is established it reads data and echoes it back to the client. Note that the
'    server uses blocking sockets to accept connections as well as send and
'    receive data. This means once the server is listening the UI will not
'    refresh.
'
'    The client side is simple. The user specifies the hostname and port where
'    the echo server is running and hits the connect button. This establishes
'    a connection to the echo server and the user may enter text into the send
'    box and hit the send and receive button. This causes the text in the send
'    box to be sent to the server. It then reads the data back and places it
'    into the received text box.

'    This is a full fledged Winsock echo client application. It does NOT rely
'    on the Winsock control.
'
Option Explicit

Dim soc As Long, dwRc As Long
Dim RemoteAddr As sockaddr
Dim RetMsg As String

'
' Subroutine: cmdSendRecv_Click
'
' Description:
'    This routine is the handler for the "Send & Receive Text" button.
'    When the user hits this button the message in the "Send" box is
'    sent to the echo server. It then performs a read which should read
'    the message back as it was echoed from the server.
'
Private Sub cmdSendRecv_Click()
    If soc = INVALID_SOCKET Then
        MsgBox "Create Socket First"
        Exit Sub
    End If
    
    RetMsg = String(7000, 0)
    '
    ' send data
    dwRc = send(soc, ByVal txtSend.Text, Len(txtSend.Text), 0)
    If dwRc = SOCKET_ERROR Then
        MsgBox "Couldn't send data to remote Socket. Error: " & Err.LastDllError
    Else
        ' receive data
        dwRc = recv(soc, ByVal RetMsg, 7000, 0)
        If dwRc = SOCKET_ERROR Then
            MsgBox "Couldn't recieve data from remote Socket. Error: " & Err.LastDllError
        Else
            txtRecv.Text = Left(RetMsg, InStr(RetMsg, Chr(0)))
        End If
    End If
End Sub

'
' Subroutine: cmdConnect_Click
'
' Description:
'    This routine is called when the user hits the "Connect" button.
'    When this occurs, we attempt a connection to the server name
'    given as "Remote Server" on the specified port ("Remote Port").
'    The function blocks until either a connection is established
'    or an error occurs (timeout, ect.).
'
Private Sub cmdConnect_Click()
        
    soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
    If soc = INVALID_SOCKET Then
        MsgBox "Couldn't Create Socket. Error: " & Err.LastDllError
    Else
        RemoteAddr.sin_family = AF_INET     ' address family, internet: 2
        RemoteAddr.sin_port = htons(CLng(txtPort.Text))     ' port 7, must convert
        RemoteAddr.sin_addr = GetHostByNameAlias(txtServer.Text)
        
        'connect
        dwRc = connect(soc, RemoteAddr, sockaddr_size)
            
        If dwRc = SOCKET_ERROR Then
            MsgBox "Couldn't connect to remote Socket. Error: " & Err.LastDllError
        Else
            cmdConnect.Enabled = False
            cmdSendRecv.Enabled = True
            cmdDisconnect.Enabled = True
        End If

    End If
End Sub

'
' Subroutine: cmdDisconnect_Click
'
' Description:
'    This routine is the handler for the "Disconnect" button. When the
'    user hits this button the connection to the echo server is closed.
'
Private Sub cmdDisconnect_Click()
    txtSend.Text = ""
    txtRecv.Text = ""
    If soc <> INVALID_SOCKET Then closesocket (soc)
    cmdConnect.Enabled = True
    cmdSendRecv.Enabled = False
    cmdDisconnect.Enabled = False
    soc = INVALID_SOCKET
End Sub

'
' Subroutine: Form_Load
'
' Description:
'    This routine makes sure the Winsock DLL has been properly loaded
'    in addition to setting the form up in its initial state.
'
Private Sub Form_Load()
    If TCPIPStartup Then
        cmdSendRecv.Enabled = True
    Else
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError
    End If
    soc = INVALID_SOCKET
    cmdConnect.Enabled = True
    cmdSendRecv.Enabled = False
    cmdDisconnect.Enabled = False
End Sub

'
' Subroutine: Form_Unload
'
' Description:
'    This routine unloads the Winsock DLL.
'
Private Sub Form_Unload(Cancel As Integer)
    TCPIPShutDown
End Sub
