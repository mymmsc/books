VERSION 5.00
Begin VB.Form EchoSvr 
   Caption         =   "TCP Echo Sever"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtPort 
      Height          =   375
      Left            =   960
      TabIndex        =   2
      Text            =   "5000"
      Top             =   2520
      Width           =   1215
   End
   Begin VB.CommandButton cmdStart 
      Caption         =   "start server"
      Height          =   735
      Left            =   2520
      TabIndex        =   0
      Top             =   2280
      Width           =   1695
   End
   Begin VB.Label Label3 
      Caption         =   $"EchoSvr.frx":0000
      Height          =   615
      Left            =   240
      TabIndex        =   4
      Top             =   1440
      Width           =   4095
   End
   Begin VB.Label Label2 
      Caption         =   $"EchoSvr.frx":00A4
      Height          =   1095
      Left            =   240
      TabIndex        =   3
      Top             =   120
      Width           =   4095
   End
   Begin VB.Label Label1 
      Caption         =   "Port:"
      Height          =   255
      Left            =   360
      TabIndex        =   1
      Top             =   2520
      Width           =   375
   End
End
Attribute VB_Name = "EchoSvr"
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

'
' Subroutine: Form_Load
'
' Description:
'    This routine makes sure the Winsock DLL has been properly loaded
'    in addition to setting the form up in its initial state.
'
Sub Form_Load()
    If TCPIPStartup Then
        cmdStart.Enabled = True
    Else
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError
    End If
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

'
' Subroutine: CmdStart_Click
'
' Description:
'    This subroutine starts the echo server. When the user hits the
'    "Start Server" button a socket is created that is bound to the
'    given local port. It then waits for incoming client connections
'    at which point it accepts the connection and then reads data.
'    Any data read will be written back to the client. Note that
'    all these calls are blocking Winsock calls which means the
'    server dialog will not refresh until one client is serviced
'    and disconnects.
'
Private Sub CmdStart_Click()
   Dim addr As sockaddr, client_addr As sockaddr, addrlen As Long, client_addrlen As Long, pSockAddr As Long
   Dim msg_sock As Long, accept_sock As Long, msgstr As String
   Dim i As Long
   
   cmdStart.Enabled = False
   
   'get socket handle
   accept_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
   
   If accept_sock = INVALID_SOCKET Then
       MsgBox "Couldn't create socket(). Error:" & Err.LastDllError, "Accept"
       Exit Sub
   End If
   '
   ' Setup our local address to listen on
   '
   addr.sin_family = AF_INET
   addr.sin_port = htons(CLng(txtPort.Text))
   addr.sin_addr = INADDR_ANY
      
   'bind to socket
   If bind(accept_sock, addr, Len(addr)) = SOCKET_ERROR Then
       MsgBox "Couldn't bind() to socket. Error: " & Err.LastDllError
       closesocket (accept_sock)
       Exit Sub
   End If

   'start listening on socket
   If listen(accept_sock, 1) = SOCKET_ERROR Then
       MsgBox "Couldn't listen() to socket. Error: " & Err.LastDllError
       closesocket (accept_sock)
       Exit Sub
   End If
   
   client_addrlen = Len(client_addr)
   '
   ' Wait for a client to connect to us.
   '
   msg_sock = accept(accept_sock, client_addr, client_addrlen)
   If msg_sock = INVALID_SOCKET Then
       MsgBox "Couldn't accept an socket connection. Error: " & Err.LastDllError
       closesocket (accept_sock)
       Exit Sub
   End If
  
   Dim RetMsg As String
   Dim dwRc As Long
   Do 'loop recv and send until the other end close socket
    RetMsg = String(7000, 0)
    dwRc = recv(msg_sock, ByVal RetMsg, 7000, 0)
    If dwRc = SOCKET_ERROR Then
         MsgBox "Couldn't recieve data from remote Socket. Error: " & Err.LastDllError
         closesocket (accept_sock)
         closesocket (msg_sock)
         cmdStart.Enabled = True
         Exit Sub
     End If
     '
     ' Take the data we just read and write it back to the client
     '
     dwRc = send(msg_sock, ByVal RetMsg, Len(RetMsg), 0)
     If dwRc = SOCKET_ERROR Then
         MsgBox "Couldn't send data to remote Socket. Error: " & Err.LastDllError
         closesocket (accept_sock)
         closesocket (msg_sock)
         cmdStart.Enabled = True
         Exit Sub
     End If
    Loop
    closesocket (msg_sock)
    closesocket (accept_sock)
   
    cmdStart.Enabled = True
                 
End Sub





