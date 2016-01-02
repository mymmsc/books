VERSION 5.00
Begin VB.Form pipec 
   Caption         =   "Pipe Client"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox List1 
      Height          =   1425
      Left            =   480
      TabIndex        =   3
      Top             =   1560
      Width           =   3375
   End
   Begin VB.CommandButton cmdSend 
      Caption         =   "Send ""This a test"" to the pipe server"
      Height          =   495
      Left            =   480
      TabIndex        =   2
      Top             =   720
      Width           =   3255
   End
   Begin VB.TextBox txtPipeName 
      Height          =   285
      Left            =   1800
      TabIndex        =   1
      Text            =   "\\.\PIPE\jim"
      Top             =   240
      Width           =   1455
   End
   Begin VB.Label Label1 
      Caption         =   "Pipe Name:"
      Height          =   375
      Left            =   480
      TabIndex        =   0
      Top             =   240
      Width           =   1095
   End
End
Attribute VB_Name = "pipec"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: client
'
' Description:
'    This is a simple named pipe client application. It opens a handle to
'    the given named pipe and then sends a message and reads a message back.
'

Option Explicit

'
' Subroutine: cmdSend_Click
'
' Description:
'    This routine is invoked when the user hits the Send button. This
'    procedure will wait until the given named pipe becomes available
'    and then will open an instance to it. It will then write data to
'    the pipe and read it back.
'
Private Sub cmdSend_Click()
    Dim PipeHandle As Long, BytesWritten As Long, BytesRead As Long
    Dim PIPE_NAME As String
    Dim buffer As String
    Dim dwRet As Long
    
    PIPE_NAME = txtPipeName.Text
    '
    ' Wait until the named pipe becomes available
    '
    dwRet = WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)
    
    If dwRet = 0 Then
        MsgBox "WaitNamedPipe failed with error " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Open an instance to the pipe.
    '
    PipeHandle = CreateFile(PIPE_NAME, _
        GENERIC_READ Or GENERIC_WRITE, _
        0, _
        0, _
        OPEN_EXISTING, _
        FILE_ATTRIBUTE_NORMAL, _
        0)
    
    If PipeHandle = INVALID_HANDLE_VALUE Then
       MsgBox "CreateFile failed with error " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Write data to the pipe
    '
    dwRet = WriteFile(PipeHandle, "This is a test", 14, BytesWritten, 0)
    
    If dwRet = 0 Then
        MsgBox "WriteFile failed with error " & Err.LastDllError
        CloseHandle PipeHandle
        Exit Sub
    End If
    '
    ' Read the data back from the pipe server
    '
    buffer = String(256, 0)
    dwRet = ReadFile(PipeHandle, buffer, 256, BytesRead, 0)
    If dwRet <> 0 Then
      buffer = Left(buffer, BytesRead)
      List1.AddItem "Echo from server: " & buffer
    Else
      MsgBox "Failed to read from pipe " & Err.LastDllError
    End If
    
    CloseHandle PipeHandle
    
    MsgBox "The client is ready to shut down"
    Unload Me
    
End Sub

