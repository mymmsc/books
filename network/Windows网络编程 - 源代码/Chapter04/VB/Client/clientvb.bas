Attribute VB_Name = "clientvb"
Option Explicit
Public Declare Function WaitNamedPipe Lib "kernel32" Alias "WaitNamedPipeA" (ByVal lpNamedPipeName As String, ByVal nTimeOut As Long) As Long
Public Declare Function CreateFile Lib "kernel32" Alias "CreateFileA" (ByVal lpFileName As String, ByVal dwDesiredAccess As Long, ByVal dwShareMode As Long, ByVal lpSecurityAttributes As Long, ByVal dwCreationDisposition As Long, ByVal dwFlagsAndAttributes As Long, ByVal hTemplateFile As Long) As Long
Public Declare Function ReadFile Lib "kernel32" (ByVal hFile As Long, ByVal lpBuffer As String, ByVal nNumberOfBytesToRead As Long, lpNumberOfBytesRead As Long, ByVal lpOverlapped As Long) As Long
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
Public Const NMPWAIT_WAIT_FOREVER = &HFFFF



