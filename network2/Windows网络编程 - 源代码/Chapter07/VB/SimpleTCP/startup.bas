Attribute VB_Name = "startup"
Option Explicit

'
' Subroutine: Main
'
' Description:
'    This is the routine which starts everything off. It first prompts
'    the user whether to run an instance of the echo client or server.
'    It then loads the correct form.
'
Sub Main()
    Dim dwRet As Long
    dwRet = MsgBox("Click Yes to start TCP echo server, or No to start TCP echo client.", vbYesNo)
    If dwRet = vbYes Then
        Load EchoSvr
        EchoSvr.Show
    Else
        Load EchoCli
        EchoCli.Show
    End If
End Sub

'
' Function: VBntoaVers
'
' Description:
'    This function takes a 32 bit value and takes the high word
'    and low word as a version number (seperated by a ".") and
'    returns the string version.
'
Function VBntoaVers(ByVal vers As Long) As String
    Dim szVers As String
    szVers = String(5, 0)
    szVers = (vers And &HFF) & "." & ((vers And &HFF00) / 256)
    VBntoaVers = szVers
End Function

'
' Function: hibyte
'
' Description:
'    This function returns the high byte of an integer.
'
Function hibyte(ByVal wParam As Integer)
   hibyte = wParam \ &H100 And &HFF&
End Function

'
' Function: lobyte
'
' Description:
'    This function returns the low byte of an integer.
'
Function lobyte(ByVal wParam As Integer)
   lobyte = wParam And &HFF&
End Function

