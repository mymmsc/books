Attribute VB_Name = "servervb"
'
' Project: server
'
' Description:
'    This is a simple mailslot server application. A mailslot is created
'    and we block on a read call. Once a message is read we display it
'    and exit.
'

Option Explicit

Public Declare Function CreateMailslot Lib "kernel32" Alias "CreateMailslotA" (ByVal lpName As String, ByVal nMaxMessageSize As Long, ByVal lReadTimeout As Long, ByVal lpSecurityAttributes As Long) As Long
Public Declare Function ReadFile Lib "kernel32" (ByVal hFile As Long, ByVal lpBuffer As String, ByVal nNumberOfBytesToRead As Long, lpNumberOfBytesRead As Long, ByVal lpOverlapped As Long) As Long
Public Declare Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long
Public Const INVALID_HANDLE_VALUE = &HFFFFFFFF
Public Const MAILSLOT_WAIT_FOREVER = (-1)

'
' Subroutine: Main
'
' Description:
'    This simple app has no UI. It creates a mailslot (which therefor means
'    it can only run on NT). It then attempts a ReadFile from the mailslot
'    which will block until data is received. This means the app will
'    remain blocked until data is read or until it is killed. Once data is
'    read the program exits.
'
Sub Main()
   Dim Mailslot As Long, NumberOfBytesRead As Long
   Dim buffer As String
   Dim dwRet As Long
   Dim StrMailSlot As String

   StrMailSlot = "\\.\mailslot\myslot"
   
   MsgBox "Entering Sub Main..."
   
   MsgBox "This server doesn't have a UI. It does a ReadFile, display a message when it comes in, and exits"
   ' Create the mailslot
   Mailslot = CreateMailslot(StrMailSlot, 0, _
      MAILSLOT_WAIT_FOREVER, 0)
     
   If Mailslot = INVALID_HANDLE_VALUE Then
      MsgBox "Failed to create a MailSlot " & Err.LastDllError
      Exit Sub
   End If
   '
   ' Let the user know what the mailslot is named
   '
   MsgBox "Mailslot created on: " & StrMailSlot
   
   buffer = String(256, 0)
   '
   ' Read some data from the mailslot
   '
   dwRet = ReadFile(Mailslot, buffer, 256, NumberOfBytesRead, 0)
   If dwRet <> 0 Then
      buffer = Left(buffer, NumberOfBytesRead)
      MsgBox buffer
   Else
      MsgBox "Failed to read a MailSlot " & Err.LastDllError
   End If
   
   CloseHandle Mailslot
   
   MsgBox "Exiting Sub Main..."
   
End Sub


