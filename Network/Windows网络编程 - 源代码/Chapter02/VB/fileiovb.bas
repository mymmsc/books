Attribute VB_Name = "fileiovb"
'
' Project: fileio
'
' Description:
'    This is another simple app that uses the Win32 APIs CreateFile and
'    WriteFile to create a file on a remote computer. Once the file is
'    created we write some simple text to it. The most common error
'    encountered is insufficient permissiones (error 5) on the CreateFile
'    call.
'
Option Explicit
'
' Declare the Win32 file I/O functions as well as the contants used
'  by them
'
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

'
' Subroutine: Main
'
' Decription:
'    Make a call to CreatFile to create a file on the specified remote
'    share.
'
Sub Main()
    Dim FileHandle As Long, BytesWritten As Long
    Dim FileName As String
    Dim dwRet As Long
    
    MsgBox "Entering sub main..."
    '
    ' Prompt for the file name
    '
    FileName = InputBox("Please enter remote file path (e.g. \\server\share\file.txt): ")
        
    FileHandle = CreateFile(FileName, _
        GENERIC_WRITE Or GENERIC_READ, FILE_SHARE_READ Or FILE_SHARE_WRITE, _
        0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)
    
    If FileHandle = INVALID_HANDLE_VALUE Then
        MsgBox "CreateFile failed with error " & Err.LastDllError
        Exit Sub
    End If
    
    'Write 14 bytes to our new file
    dwRet = WriteFile(FileHandle, "This is a test", 14, BytesWritten, 0)
    
    If dwRet = 0 Then
        MsgBox "WriteFile failed with error " & Err.LastDllError
        Exit Sub
    End If
    
    dwRet = CloseHandle(FileHandle)
    
    If dwRet = 0 Then
        MsgBox "CloseHandle failed with error " & Err.LastDllError
        Exit Sub
    End If
    
    MsgBox "Exiting sub main..."
End Sub
