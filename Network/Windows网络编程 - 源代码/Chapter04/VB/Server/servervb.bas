Attribute VB_Name = "servervb"
'
' Project: server
'
' Description:
'    This app is a simple named pipe server. It creates a named pipe
'    and then waits for a single client to connect at which point it
'    reads a single message and writes it back. The connection is
'    then closed.
'
Option Explicit

Public Declare Function CreateNamedPipe Lib "kernel32" Alias "CreateNamedPipeA" (ByVal lpName As String, ByVal dwOpenMode As Long, ByVal dwPipeMode As Long, ByVal nMaxInstances As Long, ByVal nOutBufferSize As Long, ByVal nInBufferSize As Long, ByVal nDefaultTimeOut As Long, ByVal lpSecurityAttributes As Long) As Long
Public Declare Function ConnectNamedPipe Lib "kernel32" (ByVal hNamedPipe As Long, ByVal lpOverlapped As Long) As Long
Public Declare Function ReadFile Lib "kernel32" (ByVal hFile As Long, ByVal lpBuffer As String, ByVal nNumberOfBytesToRead As Long, lpNumberOfBytesRead As Long, ByVal lpOverlapped As Long) As Long
Public Declare Function WriteFile Lib "kernel32" (ByVal hFile As Long, ByVal lpBuffer As String, ByVal nNumberOfBytesToWrite As Long, lpNumberOfBytesWritten As Long, ByVal lpOverlapped As Long) As Long
Public Declare Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long
Public Const PIPE_READMODE_BYTE = &H0
Public Const PIPE_ACCESS_DUPLEX = &H3
Public Const PIPE_TYPE_BYTE = &H0
Public Const PIPE_NOWAIT = &H1
Public Const INVALID_HANDLE_VALUE = &HFFFFFFFF

'
' Subroutine: Main
'
' Description:
'    This app has no UI per se. It creates a named pipe, loads a simple
'    text box, and then waits to read data. Once data is read it is
'    written back to the pipe. Afterwards, the pipe is closed and the
'    application exits. Note that the UI will not refresh once this
'    Main routine blocks for incoming data. This is done for the sake
'    of simplicity.
'
Sub Main()
   Dim PipeHandle As Long, BytesWritten As Long, BytesRead As Long
   Dim buffer As String
   Dim dwRet As Long
   Dim StrPipe As String
         
   StrPipe = "\\.\PIPE\jim"
   '
   ' Create the named pipe
   '
   PipeHandle = CreateNamedPipe(StrPipe, _
      PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE Or PIPE_READMODE_BYTE, 1, _
      0, 0, 1000, 0)
      
   If PipeHandle = INVALID_HANDLE_VALUE Then
      MsgBox "CreateNamedPipe failed with error " & Err.LastDllError
      Exit Sub
   End If
   '
   ' Let the user know the name of the pipe
   '
   MsgBox "Created named pipe: " & StrPipe
   
   Load svrfrm
   svrfrm.Show
   DoEvents
   '
   ' Let clients connect to our pipe
   '
   dwRet = ConnectNamedPipe(PipeHandle, 0)
   If dwRet = 0 Then
      MsgBox "ConnectNamedPipe failed with error " & Err.LastDllError
      CloseHandle PipeHandle
      Exit Sub
   End If
      
   buffer = String(256, 0)
   '
   ' Read data from the pipe. This call blocks until data is present.
   '
   dwRet = ReadFile(PipeHandle, buffer, 256, BytesRead, 0)
   If dwRet <> 0 Then
      buffer = Left(buffer, BytesRead)
   Else
      MsgBox "Failed to read from pipe " & Err.LastDllError
   End If
   '
   ' Write the data back to the client
   '
   dwRet = WriteFile(PipeHandle, buffer, Len(buffer) + 1, BytesWritten, 0)
    
   If dwRet = 0 Then
        MsgBox "WriteFile failed with error " & Err.LastDllError
        CloseHandle PipeHandle
        Exit Sub
   End If
       
   CloseHandle PipeHandle
   
   Unload svrfrm
   MsgBox "Server has echoed a client message coming in to the pipe, and is ready to shut down..."

End Sub
