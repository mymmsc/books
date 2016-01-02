VERSION 5.00
Begin VB.Form NBClient 
   Caption         =   "NBClient"
   ClientHeight    =   5355
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6090
   LinkTopic       =   "Form1"
   ScaleHeight     =   5355
   ScaleMode       =   0  'User
   ScaleWidth      =   609
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox recvList 
      Height          =   3960
      Left            =   1800
      TabIndex        =   7
      Top             =   1200
      Width           =   4095
   End
   Begin VB.TextBox txtServerName 
      Height          =   285
      Left            =   4080
      TabIndex        =   6
      Text            =   "nbServer"
      Top             =   720
      Width           =   1815
   End
   Begin VB.TextBox txtClientName 
      Height          =   285
      Left            =   4080
      TabIndex        =   4
      Text            =   "nbClient"
      Top             =   240
      Width           =   1815
   End
   Begin VB.CommandButton cmdSendRecv 
      Caption         =   "Connect"
      Height          =   495
      Left            =   120
      TabIndex        =   2
      Top             =   4680
      Width           =   1455
   End
   Begin VB.ListBox ListLana 
      Height          =   3960
      Left            =   120
      TabIndex        =   1
      Top             =   480
      Width           =   1095
   End
   Begin VB.Label Label3 
      Caption         =   "Specify the server NB name:"
      Height          =   255
      Left            =   1800
      TabIndex        =   5
      Top             =   720
      Width           =   2295
   End
   Begin VB.Label Label2 
      Caption         =   "Specify the client NB name: "
      Height          =   255
      Left            =   1800
      TabIndex        =   3
      Top             =   240
      Width           =   2175
   End
   Begin VB.Label Label1 
      Caption         =   "Available Lana"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   1215
   End
End
Attribute VB_Name = "NBClient"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: VBNBClient
'
' Description:
'    This app is a NetBIOS client application for session oriented communication.
'    In order to establish a session the user must specify the client's NetBIOS
'    name as well as the server's NetBIOS name which we will connect to. Once
'    we have connected some data is exchanged and displayed in the window.
'
Option Explicit

Dim dwErr As Long
Dim dwNum As Long, dwRet As Long, dwBufferLen As Long
Dim dwIndex As Long
Dim lenum As LANA_ENUM
Dim byteMessage(254) As UserBuffer
Dim ncbArray(254) As NCB
Dim eventArray(254) As Long
Dim i As Long, j As Long

'
' Subroutine: Form_Load
'
' Description:
'    Upon form load, enumerate and reset all LANAs.
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

'
' Subroutine: cmdSendRecv_Click
'
' Description:
'    When this button is clicked, post an NCBCALL (connect) on each
'    available LANA. Then wait until at least one succeeds. At this
'    point take the first successfull connection and either cancel
'    or hangup the remaining connect calls. Once this occurs start
'    sending and receiving data.
'
Private Sub cmdSendRecv_Click()
    cmdSendRecv.Enabled = False
    '
    ' Post a connect on each LANA
    '
    For i = 0 To lenum.length - 1
        eventArray(i) = CreateEvent(0, 1, 0, vbNullString)
        ZeroMemory ncbArray(i), Len(ncbArray(i))
        ncbArray(i).ncb_event = eventArray(i)
        AddName lenum.lana(i), txtClientName.Text, dwNum
        Connect ncbArray(i), lenum.lana(i), txtServerName.Text, txtClientName.Text
    Next
    '
    ' Wait for at least one to complete
    '
    dwIndex = WaitForMultipleObjects(lenum.length, eventArray(0), 0, INFINITE)
                    
    If dwIndex = WAIT_FAILED Then
        MsgBox "WaitForMultipleObjects failed"
        Exit Sub
    Else
        ' Hangup or cancel the remaining attempts
        '
        For i = 0 To lenum.length - 1
            If i <> dwIndex Then
                If ncbArray(i).ncb_cmd_cplt = NRC_PENDING Then
                    Cancel ncbArray(i)
                Else
                    Hangup ncbArray(i).ncb_lana_num, ncbArray(i).ncb_lsn
                End If
            End If
        Next
        
        recvList.AddItem "Connected on LANA: " & ncbArray(i).ncb_lana_num
        
        Dim tempMessageStr As String
        Dim tempSenderStr As String
        '
        ' Send some messages to the server then read them back.
        '
        For i = 0 To 9
            tempMessageStr = "Test message " & i
            lstrcpy2 VarPtr(byteMessage(0)), tempMessageStr
            dwRet = Send(ncbArray(dwIndex).ncb_lana_num, ncbArray(dwIndex).ncb_lsn, VarPtr(byteMessage(0)), Len(tempMessageStr) + 1)
            If dwRet <> NRC_GOODRET Then Exit For
            
            dwBufferLen = 512
            dwRet = Recv(ncbArray(dwIndex).ncb_lana_num, ncbArray(dwIndex).ncb_lsn, VarPtr(byteMessage(0)), dwBufferLen)
            If dwRet <> NRC_GOODRET Then Exit For
            byteMessage(0).userByteArray(dwBufferLen) = 0
            lstrcpy tempMessageStr, VarPtr(byteMessage(0))
            recvList.AddItem tempMessageStr
        Next
        
        Hangup ncbArray(dwIndex).ncb_lana_num, ncbArray(dwIndex).ncb_lsn
        
    End If
    '
    ' Clean things up
    '
    For i = 0 To lenum.length - 1
        DelName lenum.lana(i), txtClientName.Text
        CloseHandle eventArray(i)
    Next
    
    cmdSendRecv.Enabled = True
    
End Sub
