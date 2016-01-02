VERSION 5.00
Begin VB.Form vbrnrfrm 
   Caption         =   "Name Space Enumeration"
   ClientHeight    =   4200
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   8205
   LinkTopic       =   "Form1"
   ScaleHeight     =   4200
   ScaleWidth      =   8205
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox List1 
      BeginProperty Font 
         Name            =   "System"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3420
      Left            =   360
      TabIndex        =   0
      Top             =   480
      Width           =   7455
   End
End
Attribute VB_Name = "vbrnrfrm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: vbenumnp
'
' Description:
'    This simple app enumerates the installed name space providers available
'    on the machine. This is a simple call to the WSAEnumNameSpaceProviders API.
'    The results are displayed in a listbox.
'

Option Explicit

'
' Subroutine: DoEnum
'
' Description:
'    This routine enumerates all name space providers installed
'    on the system via the WSAEnumNameSpaceProvders function.
'
Private Sub DoEnum()
    Dim wsadStartupData As WSADataType
    Dim dwRet As Long, dwBufLen As Long, dwErr As Long
    Dim pNSBuffer As Long
    Dim i As Long
    
    dwBufLen = 0
    dwRet = WSAEnumNameSpaceProviders(dwBufLen, 0)
    If dwRet = SOCKET_ERROR Then 'it's going to happ as buffer is 0
        dwErr = Err.LastDllError
        If dwErr <> WSAENOBUFS And dwErr <> 0 And dwErr <> WSAEFAULT Then
            MsgBox "Error WSAEnumProtocols: " & dwErr
            WSACleanup
            Exit Sub
        End If
        pNSBuffer = GlobalAlloc(GMEM_FIXED, dwBufLen)
        dwRet = WSAEnumNameSpaceProviders(dwBufLen, pNSBuffer)
        If dwRet = SOCKET_ERROR Then
            dwErr = Err.LastDllError
            MsgBox "Error WSAEnumPrtocols: " & dwErr
            GlobalFree pNSBuffer
            Exit Sub
        End If
        
        List1.AddItem "Num Name Space found: " & dwRet
        
        Dim pWorkPointer As Long
        
        For i = 0 To dwRet - 1
            DisplayNSInfoToList pNSBuffer, i
        Next i
    End If
    GlobalFree pNSBuffer
End Sub

'
' Subroutine: DisplayNSInfoToList
'
' Description:
'    This routine converts the name space identifier into a friendlier
'    string description.
'
Sub DisplayNSInfoToList(pNSBuffer As Long, index As Long)
    Dim EnumStrAll As String
    Dim nsinfo As WSANAMESPACE_INFO
    CopyMemory2 nsinfo, pNSBuffer + index * LenB(nsinfo), LenB(nsinfo)
    Dim szIdentifier As String
    szIdentifier = String(256, 0)
    lstrcpy1 szIdentifier, nsinfo.lpszIdentifier
    szIdentifier = Trim(szIdentifier)
    List1.AddItem ""
    List1.AddItem "Name Space: " & szIdentifier
    EnumStrAll = "        ID: "
        
    Select Case nsinfo.dwNameSpace
        
    Case NS_ALL
        EnumStrAll = EnumStrAll & "NS_ALL"
    Case NS_SAP
        EnumStrAll = EnumStrAll & "NS_SAP"
    Case NS_NDS
        EnumStrAll = EnumStrAll & "NS_NDS"
    Case NS_PEER_BROWSE:
        EnumStrAll = EnumStrAll & "NS_PEER_BROWSE"
    Case NS_TCPIP_LOCAL:
        EnumStrAll = EnumStrAll & "NS_TCPIP_LOCAL"
    Case NS_TCPIP_HOSTS:
        EnumStrAll = EnumStrAll & "NS_TCPIP_HOSTS"
    Case NS_DNS:
        EnumStrAll = EnumStrAll & "NS_DNS"
    Case NS_NETBT:
        EnumStrAll = EnumStrAll & "NS_NETBT"
    Case NS_WINS:
        EnumStrAll = EnumStrAll & "NS_WINS"
    Case NS_NBP:
        EnumStrAll = EnumStrAll & "NS_NBP"
    Case NS_MS:
        EnumStrAll = EnumStrAll & "NS_MS"
    Case NS_STDA:
        EnumStrAll = EnumStrAll & "NS_STDA"
    Case NS_NTDS:
        EnumStrAll = EnumStrAll & "NS_NTDS"
    Case NS_X500:
        EnumStrAll = EnumStrAll & "NS_X500"
    Case NS_NIS:
        EnumStrAll = EnumStrAll & "NS_NIS"
    Case NS_NISPLUS:
        EnumStrAll = EnumStrAll & "NS_NISPLUS"
    Case NS_WRQ:
        EnumStrAll = EnumStrAll & "NS_WRQ"
    Case Else
        EnumStrAll = EnumStrAll & nsinfo.dwNameSpace
    End Select
    List1.AddItem EnumStrAll
    
    Dim szGuidString As String
    szGuidString = String(256, 0)
    StringFromGUID2 nsinfo.NSProviderId, szGuidString, 256
    szGuidString = StrConv(szGuidString, vbFromUnicode)
    EnumStrAll = "       GUID: " & szGuidString
    List1.AddItem EnumStrAll
    List1.AddItem "    Active: " & nsinfo.fActive
    List1.AddItem "   Version: " & nsinfo.dwVersion

End Sub

'
' Subroutine: Form_Load
'
' Description:
'    Load the Winsock DLL.
'
Private Sub Form_Load()
    If Not TCPIPStartup Then
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    DoEnum
End Sub

'
' Subroutine: Form_Unload
'
' Description:
'    Unload the Winsock DLL.
'
Private Sub Form_Unload(Cancel As Integer)
    TCPIPShutDown
End Sub
