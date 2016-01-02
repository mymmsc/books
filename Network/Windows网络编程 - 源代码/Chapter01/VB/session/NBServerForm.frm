VERSION 5.00
Begin VB.Form NBServerForm 
   Caption         =   "NBServerForm"
   ClientHeight    =   4275
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6660
   LinkTopic       =   "Form1"
   ScaleHeight     =   4275
   ScaleWidth      =   6660
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtNBServerName 
      Height          =   285
      Left            =   3480
      TabIndex        =   4
      Text            =   "nbServer"
      Top             =   240
      Width           =   1335
   End
   Begin VB.ListBox recvList 
      Height          =   3180
      Left            =   1560
      TabIndex        =   3
      Top             =   840
      Width           =   4815
   End
   Begin VB.ListBox ListLana 
      Height          =   3375
      Left            =   120
      TabIndex        =   2
      Top             =   600
      Width           =   1215
   End
   Begin VB.CommandButton cmdListen 
      Caption         =   "Listen"
      Height          =   495
      Left            =   4920
      TabIndex        =   0
      Top             =   120
      Width           =   1455
   End
   Begin VB.Label Label2 
      Caption         =   "NB Name to listen on:"
      Height          =   255
      Left            =   1800
      TabIndex        =   5
      Top             =   240
      Width           =   1575
   End
   Begin VB.Label Label1 
      Caption         =   "Available Lana(s):"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   240
      Width           =   1455
   End
End
Attribute VB_Name = "NBServerForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: VBNBSvr
'
' Description:
'    This is a NetBIOS session oriented server. It listens for client connections
'    to a given NetBIOS name. The user enters the server name on which we will
'    wait for client connections. Once a connection is established, we receive
'    some data from the client and echo it back.
'

Option Explicit

Dim dwErr As Long
Dim dwNum As Long, dwRet As Long, dwRetVal As Long, dwBufferLen As Long
Dim lenum As LANA_ENUM
Dim byteMessage As UserBuffer
Dim ncbArray(254) As NCB
Dim eventArray(254) As Long
Dim i As Long, j As Long
Dim szClientName As String, szClientMsg As String

'
' Subroutine: cmdListen_Click
'
' Description:
'    This routine issues an asynchronous NCBLISTEN command for
'    client connections. This is done using a Win32 event object,
'    and once the command is issued the routine blocks on the
'    event until it becomes signaled. This will result in the
'    UI "hanging" until an incoming connection occurs. This
'    sample does this for simplicity.
'
Private Sub cmdListen_Click()
    cmdListen.Enabled = False
    '
    ' Add a name to each LANA and post a listen
    '
    For i = 0 To lenum.length - 1
        eventArray(i) = CreateEvent(0, 1, 0, vbNullString)
        ZeroMemory ncbArray(i), Len(ncbArray(i))
        ncbArray(i).ncb_event = eventArray(i)
        AddName lenum.lana(i), txtNBServerName.Text, dwNum
        Listen ncbArray(i), lenum.lana(i), txtNBServerName
    Next
    '
    ' Wait until a connection comes in
    '
    dwRet = WaitForMultipleObjects(lenum.length, eventArray(0), 0, INFINITE)
        
    If dwRet = WAIT_FAILED Then
        MsgBox "WaitForMultipleObjects failed"
        Exit Sub
    End If
    '
    ' Cancel or hangup any of the other posted NCBLISTEN commands
    '  That is, we only want to accent one connection from the
    '  given endpoint (in the case the the client posted NCBCALLs
    '  on multiple LANAs).
    '
    For i = 0 To lenum.length - 1
        If i <> dwRet Then
            If ncbArray(i).ncb_cmd_cplt = NRC_PENDING Then
                Cancel ncbArray(i)
            Else
                Hangup ncbArray(i).ncb_lana_num, ncbArray(i).ncb_lsn
            End If
        End If
    Next
    '
    ' Format the clients name and then go into a receive/send loop to
    '  read and echo the data.
    '
    FormatNetbiosName ncbArray(dwRet).ncb_callname, szClientName
    ncbArray(dwRet).ncb_event = 0
    Do While (True)
        dwBufferLen = Len(byteMessage)
        dwRetVal = Recv(ncbArray(dwRet).ncb_lana_num, ncbArray(dwRet).ncb_lsn, VarPtr(byteMessage), dwBufferLen)
        If dwRetVal <> NRC_GOODRET Then
            Exit Do
        End If
        
        szClientMsg = String(512, 0)
        lstrcpy szClientMsg, VarPtr(byteMessage)
        recvList.AddItem "Read LANA " & ncbArray(dwRet).ncb_lana_num & ": " & szClientMsg
        dwRetVal = Send(ncbArray(dwRet).ncb_lana_num, ncbArray(dwRet).ncb_lsn, VarPtr(byteMessage), dwBufferLen)
         If dwRetVal <> NRC_GOODRET Then
            Exit Do
        End If
    Loop
    recvList.AddItem "Client " & szClientName & " on Lana " & ncbArray(dwRet).ncb_lana_num & " disconnected"
    '
    ' Hangup the connection and clean things up
    '
    Hangup ncbArray(dwRet).ncb_lana_num, ncbArray(dwRet).ncb_lsn
    For i = 0 To lenum.length - 1
        DelName lenum.lana(i), txtNBServerName.Text
        CloseHandle eventArray(i)
    Next
    
    dwErr = ResetAll(lenum, MAX_SESSIONS, MAX_NAMES, False)
    If dwErr <> NRC_GOODRET Then
        MsgBox "ResetAll failed: " & dwErr
        Exit Sub
    End If
    
    cmdListen.Enabled = True
    MsgBox "Click Listen button to accept another client connection"

End Sub

'
' Subroutine: Form_Load
'
' Description:
'    Upon form load, enumerate and reset all LANAs
'
Private Sub Form_Load()
    dwErr = LanaEnum(lenum)
    If dwErr <> NRC_GOODRET Then
        MsgBox "LanaEnum failed: " & dwErr
        Exit Sub
    End If
    
    For i = 0 To lenum.length - 1
        ListLana.AddItem lenum.lana(i)
    Next i
    
    dwErr = ResetAll(lenum, MAX_SESSIONS, MAX_NAMES, False)
    If dwErr <> NRC_GOODRET Then
        MsgBox "ResetAll failed: " & dwErr
        Exit Sub
    End If

End Sub

