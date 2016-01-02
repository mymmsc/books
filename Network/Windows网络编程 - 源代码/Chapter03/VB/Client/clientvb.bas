Attribute VB_Name = "clientvb"
'
' Project: client
'
' Description:
'    This is a simple mailslot client. It opens a handle to an existing mailslot
'    and sends a simple message to it. Since mailslots are one way, this is the
'    only thing we can do.
'
Option Explicit

Public Declare Function CreateFile Lib "kernel32" Alias "CreateFileA" (ByVal lpFileName As String, ByVal dwDesiredAccess As Long, ByVal dwShareMode As Long, ByVal lpSecurityAttributes As Long, ByVal dwCreationDisposition As Long, ByVal dwFlagsAndAttributes As Long, ByVal hTemplateFile As Long) As Long
Public Declare Function WriteFile Lib "kernel32" (ByVal hFile As Long, ByVal lpBuffer As String, ByVal nNumberOfBytesToWrite As Long, lpNumberOfBytesWritten As Long, ByVal lpOverlapped As Long) As Long
Public Declare Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long
Public Const GENERIC_WRITE = &H40000000
Public Const GENERIC_READ = &H80000000
Public Const FILE_SHARE_READ = &H1
Public Const FILE_SHARE_WRITE = &H2
Public Const CREATE_ALWAYS = 2
Public Const FILE_ATTRIBUTE_NORMAL = &H80
Public Const INVALID_HANDLE_VALUE = &HFFFFFFFF
Public Const OPEN_EXISTING = 3

'
' Subroutine: Main
'
' Description:
'    This is a GUI-less application that opens a mailslot and writes some
'    simple data to it. Once completed the mailslot is closed and the
'    program exits.
'
Sub Main()
    Dim Mailslot As Long, BytesWritten As Long
    Dim ServerName As String
    Dim dwRet As Long
    
    MsgBox "Entering Sub Main..."
    '
    ' Prompt for the server name where the mailslot resides
    '
    ServerName = InputBox("Please enter the server name: ")
    ServerName = "\\" & ServerName & "\mailslot\myslot"
    '
    ' Open the mailslot
    '
    Mailslot = CreateFile(ServerName, GENERIC_WRITE, _
        FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, _
        0)
    
    If Mailslot = INVALID_HANDLE_VALUE Then
        MsgBox "CreateFile failed with error " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Do a quick write
    '
    dwRet = WriteFile(Mailslot, "This is a test", 14, BytesWritten, 0)
    
    If dwRet = 0 Then
        MsgBox "WriteFile failed with error " & Err.LastDllError
        Exit Sub
    End If
    
    MsgBox "Wrote " & BytesWritten & " bytes"

    CloseHandle Mailslot
    
    MsgBox "Exiting Sub Main..."
End Sub

