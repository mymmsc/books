VERSION 5.00
Begin VB.Form frmrcvall 
   Caption         =   "SIO_RCVALL"
   ClientHeight    =   6570
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9600
   LinkTopic       =   "Form1"
   ScaleHeight     =   6570
   ScaleWidth      =   9600
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdClear 
      Caption         =   "Clear Capture Window"
      Height          =   615
      Left            =   240
      TabIndex        =   18
      Top             =   5400
      Width           =   2055
   End
   Begin VB.ListBox List2 
      BeginProperty Font 
         Name            =   "System"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4620
      Left            =   2520
      TabIndex        =   17
      Top             =   1680
      Width           =   6975
   End
   Begin VB.CommandButton cmdStop 
      Caption         =   "Stop Capturing"
      Height          =   615
      Left            =   240
      TabIndex        =   16
      Top             =   4680
      Width           =   2055
   End
   Begin VB.CommandButton cmdStart 
      Caption         =   "Start Capturing"
      Height          =   615
      Left            =   240
      TabIndex        =   15
      Top             =   3960
      Width           =   2055
   End
   Begin VB.Frame Frame2 
      Caption         =   "Filter On"
      Height          =   1335
      Left            =   3600
      TabIndex        =   6
      Top             =   120
      Width           =   5055
      Begin VB.TextBox txtusDestPort 
         Height          =   285
         Left            =   3960
         TabIndex        =   10
         Top             =   840
         Width           =   735
      End
      Begin VB.TextBox txtUiDestAddr 
         Height          =   285
         Left            =   1080
         TabIndex        =   9
         Top             =   840
         Width           =   1455
      End
      Begin VB.TextBox txtusSourcePort 
         Height          =   285
         Left            =   3960
         TabIndex        =   8
         Top             =   360
         Width           =   735
      End
      Begin VB.TextBox txtuiSourceAddr 
         Height          =   285
         Left            =   1080
         TabIndex        =   7
         Top             =   360
         Width           =   1455
      End
      Begin VB.Label Label11 
         Caption         =   "Dest Port"
         Height          =   255
         Left            =   3060
         TabIndex        =   14
         Top             =   840
         Width           =   795
      End
      Begin VB.Label Label10 
         Caption         =   "Dest IP"
         Height          =   375
         Left            =   120
         TabIndex        =   13
         Top             =   840
         Width           =   735
      End
      Begin VB.Label Label9 
         Caption         =   "Source Port"
         Height          =   255
         Left            =   2880
         TabIndex        =   12
         Top             =   375
         Width           =   975
      End
      Begin VB.Label Label8 
         Caption         =   "Source IP"
         Height          =   255
         Left            =   120
         TabIndex        =   11
         Top             =   360
         Width           =   735
      End
   End
   Begin VB.ListBox List1 
      Height          =   1620
      Left            =   120
      TabIndex        =   4
      Top             =   1800
      Width           =   2175
   End
   Begin VB.Frame Frame1 
      Caption         =   "Capture Option"
      Height          =   855
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   3255
      Begin VB.OptionButton optMC 
         Caption         =   "Multicast"
         Height          =   375
         Left            =   2150
         TabIndex        =   3
         Top             =   360
         Width           =   975
      End
      Begin VB.OptionButton optIGMP 
         Caption         =   "IGMP"
         Height          =   360
         Left            =   1080
         TabIndex        =   2
         Top             =   360
         Width           =   735
      End
      Begin VB.OptionButton optIP 
         Caption         =   "IP"
         Height          =   255
         Left            =   240
         TabIndex        =   1
         Top             =   360
         Width           =   615
      End
   End
   Begin VB.Timer Timer1 
      Enabled         =   0   'False
      Interval        =   100
      Left            =   1560
      Top             =   3480
   End
   Begin VB.Label Label7 
      Caption         =   "Select Local Inteface to Capture On:"
      Height          =   375
      Left            =   120
      TabIndex        =   5
      Top             =   1320
      Width           =   2655
   End
End
Attribute VB_Name = "frmrcvall"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: vbrcvall
'
' Description:
'    This application illustrates the use of the ioctl commands SIO_RCVALL,
'    SIO_RCVALL_MCAST, and SIO_RCVALL_IGMPMCAST. These are three new ioctl
'    commands available in Windows 2000 that may be set on a raw IP socket
'    which can then be used to read the raw IP packets off of the network.
'    This app provides a simple GUI for setting the various options
'    available with these ioctls.
'
Option Explicit
Option Base 0

Dim dwIoControlCode As Long
Dim dwProtocol As Long
Dim dwInterface  As Long
Dim LocalIPList(12) As sockaddr 'max 12 local interface

Dim uiSourceAddr As Long
Dim uiDestAddr As Long
Dim usSourcePort As Long
Dim usDestPort As Long

Dim optval As Long

Dim sockAll As Long
Dim hEvent As Long

Dim dwRet As Long, dwRc As Long, dwBytesRet As Long
    
Dim RetMsg(MAX_IP_SIZE) As Byte
    
'
' Subroutine: cmdClear_Click
'
' Description:
'    This routine is the callback for the "Clear Capture Window".
'    It simply clears the data window.
'
Private Sub cmdClear_Click()
    List2.Clear
End Sub

'
' Subroutine: cmdStart_Click
'
' Description:
'    This function initiates the IP datagram capture. This routine check
'    to see if a filter is set and then initiates the timer. When the
'    timer is triggered then we check to see if data is present.
'
Private Sub cmdStart_Click()
    List2.Clear
    cmdStart.Enabled = False
    cmdStop.Enabled = True
    '
    ' See if we're filtering on ports or addresses
    '
    If bFilter = True Then
        If txtUiDestAddr.Text <> "" Then
            uiDestAddr = GetHostByNameAlias(txtUiDestAddr.Text)
        Else
            uiDestAddr = 0
        End If
        If txtuiSourceAddr.Text <> "" Then
            uiSourceAddr = GetHostByNameAlias(txtuiSourceAddr.Text)
        Else
            uiSourceAddr = 0
        End If
        If txtusDestPort.Text <> "" Then
            usDestPort = CLng(txtusDestPort.Text)
        Else
            usDestPort = 0
        End If
        If txtusSourcePort.Text <> "" Then
            usSourcePort = CLng(txtusSourcePort.Text)
        Else
            usSourcePort = 0
        End If
    End If
    
    If uiDestAddr = 0 And uiSourceAddr = 0 And usDestPort = 0 And usSourcePort = 0 Then
        bFilter = False
    Else
        bFilter = True
    End If

    Timer1.Enabled = True
End Sub

'
' Subroutine: cmdStop_Click
'
' Description:
'    This routine stops the application from reading packets.
'
Private Sub cmdStop_Click()
    cmdStart.Enabled = True
    cmdStop.Enabled = False
    If sockAll <> INVALID_SOCKET Then
        closesocket sockAll
        sockAll = INVALID_SOCKET
        Timer1.Enabled = False
    End If

End Sub

'
' Subroutine: Form_Load
'
' Description:
'    This routine is called upon form load, and it sets up default
'    values.
'
Private Sub Form_Load()
    dwIoControlCode = SIO_RCVALL
    dwProtocol = IPPROTO_IP
    dwInterface = 0
    optIP.Value = True
    
    
    uiSourceAddr = 0
    uiDestAddr = 0
    usSourcePort = 0
    usDestPort = 0
    bFilter = False
    
    Timer1.Enabled = False
    cmdStart.Enabled = True
    cmdStop.Enabled = False
    
    sockAll = INVALID_SOCKET
    
    FillSzIgmpType
    FillSzProto
    
    If TCPIPStartup Then
    Else
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError
    End If
    '
    ' Create an event to use for detecting whether there is IP datagrams
    '  to be read.
    '
    hEvent = WSACreateEvent
    If hEvent = 0 Then
        MsgBox "Failed to create event. Error: " & Err.LastDllError
    End If
    PrintInterfaceList
End Sub

'
' Subroutine: PrintInterfaceList
'
' Description:
'    This routine gets all local IP interfaces. This is necessary because
'    to use the SIO_RCVALLxxx options, you must be bound to an explicit
'    local interface (that is, you cannot bind to INADDR_ANY).
'
Sub PrintInterfaceList()
    Dim s As Long
    Dim dwBytesRet As Long
    Dim ret As Long, i As Long
    Dim slist As SOCKET_ADDRESS_LIST
    Dim Buf As String
    Dim strResult As String
        
    Dim structAddr As sockaddr
    
    Buf = String(1024, 0)
    dwBytesRet = 0
      
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If s = INVALID_SOCKET Then
        MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
        Exit Sub
    End If
    '
    ' Use SIO_ADDRESS_LIST_QUERY to obtain the local IP interfaces.
    '
    ret = WSAIoctl(s, SIO_ADDRESS_LIST_QUERY, ByVal 0, 0, slist, 1024, dwBytesRet, ByVal 0, ByVal 0)
    
    If ret = SOCKET_ERROR Then
        MsgBox "SIO_ADDRESS_LIST_QUERY failed. Error: " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Parse through the returned structures
    '
    strResult = "Bytes Returned: " & dwBytesRet & " bytes" & vbCrLf
    strResult = strResult & "      Addr Count: " & slist.iAddressCount & vbCrLf
    Debug.Print strResult
    For i = 0 To slist.iAddressCount - 1
        Dim strSockaddr As String
        Dim ptrAddr As Long
        strSockaddr = String(2560, 0)
        ptrAddr = slist.Address(i).lpSockaddr
        CopyMemory structAddr, ByVal ptrAddr, 16
        CopyMemory LocalIPList(i), structAddr, LenB(structAddr)
        lstrcpy1 strSockaddr, inet_ntoa(structAddr.sin_addr)
        strSockaddr = Trim(strSockaddr)
        strResult = "Addr [" & i & "]: " & strSockaddr
        List1.AddItem strResult
    Next i
    
    closesocket s
    
    List1.ListIndex = 0
End Sub

'
' Subroutine: Form_Unload
'
' Description:
'    Unload Winsock.
'
Private Sub Form_Unload(Cancel As Integer)
    TCPIPShutDown
End Sub

'
' Subroutine: List1_Click
'
' Description:
'    This routine is called when the user selects an IP interface.
'
Private Sub List1_Click()
    dwInterface = List1.ListIndex
    Debug.Print dwInterface
End Sub

'
' Subroutine: List1_LostFocus
'
' Description:
'    This routine is called when the IP interface list box loses focus.
'
Private Sub List1_LostFocus()
    dwInterface = List1.ListIndex
    Debug.Print dwInterface
End Sub

'
' Subroutine: optIGMP_Click
'
' Description:
'    This routine is called if the user selects the IGMP capture option.
'    It is possible to filter on all IP traffic, IGMP traffic only, or
'    multicast traffic only.
'
Private Sub optIGMP_Click()
    dwIoControlCode = SIO_RCVALL_IGMPMCAST
    dwProtocol = IPPROTO_IGMP
End Sub

'
' Subroutine: optIP_Click
'
' Description:
'    This routine is called if the user selects the IP capture option.
'    It is possible to filter on all IP traffic, IGMP traffic only, or
'    multicast traffic only.
'

Private Sub optIP_Click()
    dwIoControlCode = SIO_RCVALL
    dwProtocol = IPPROTO_IP
End Sub

'
' Subroutine: optMC_Click
'
' Description:
'    This routine is called if the user selects the multicast capture option.
'    It is possible to filter on all IP traffic, IGMP traffic only, or
'    multicast traffic only.
'

Private Sub optMC_Click()
    dwIoControlCode = SIO_RCVALL_MCAST
    dwProtocol = IPPROTO_IGMP
End Sub

'
' Subroutine: Timer1_Timer
'
' Description:
'    This routine is the timer callback routine. This is triggered whenever
'    the timer expires. It uses WSAEventSelect to check for data to be read.
'    If this is the first call to the timer handler, the raw socket is first
'    created and the appropriate SIO_RCVALLxx ioctl is set.
'
Private Sub Timer1_Timer()
    'We could just create a socket and do blocking read in loop
    'but in VB we don't want to block, so here is the timer hack
    If sockAll = INVALID_SOCKET Then
        sockAll = WSASocket(AF_INET, SOCK_RAW, dwProtocol, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
        If sockAll = INVALID_SOCKET Then
            MsgBox "WSASocket failed. Error: " & Err.LastDllError
            Exit Sub
        End If
        LocalIPList(dwInterface).sin_family = AF_INET
        LocalIPList(dwInterface).sin_port = htons(0)
        '
        ' Have to bind to an explicit IP interface
        '
        dwRet = bind(sockAll, LocalIPList(dwInterface), LenB(LocalIPList(dwInterface)))
        If dwRet = SOCKET_ERROR Then
            MsgBox "bind failed. Error: " & Err.LastDllError
            Exit Sub
        End If
        
        optval = 1
        '
        ' Set the SIO_RCVALLxxx ioctl
        '
        dwRet = WSAIoctl(sockAll, dwIoControlCode, optval, LenB(optval), ByVal 0, 0, dwBytesRet, ByVal 0, 0)
        If dwRet = SOCKET_ERROR Then
            MsgBox "dwIoControlControl failed. Error: " & Err.LastDllError
            Exit Sub
        End If
    End If
    
    ' receive data
    dwRc = WSAEventSelect(sockAll, hEvent, FD_READ)
    If (dwRc = SOCKET_ERROR) Then
        MsgBox "Failed to select event. Error: " & Err.LastDllError
        Exit Sub
    End If
        
    dwRc = WSAWaitForMultipleEvents(1, hEvent, False, 0, False)
       
    Select Case dwRc
    Case WSA_WAIT_TIMEOUT
        Debug.Print "Recv timed out"
        
    Case WSA_WAIT_EVENT_0
        Dim NetworkEvents As WSANETWORKEVENTS
        NetworkEvents.lNetWorkEvents = 0
        dwRet = WSAEnumNetworkEvents(sockAll, hEvent, NetworkEvents)
        If (dwRet = SOCKET_ERROR) Then
            MsgBox "WSAEnumNetworkEvents failed to select event. Error: " & Err.LastDllError
        Else
                        
            'We are only interested in recv
            If (FD_READ And NetworkEvents.lNetWorkEvents) Then
                dwRc = recv(sockAll, RetMsg(0), 65535, 0)
                If dwRc = SOCKET_ERROR Then
                    MsgBox "Couldn't recieve data from remote Socket. Error: " & Err.LastDllError
                Else
                    Debug.Print "recv " & dwRc
                    DecodeIPHeader RetMsg, uiSourceAddr, usSourcePort, uiDestAddr, usDestPort
                End If
            End If
        End If
    
    Case WSA_WAIT_FAILED
        MsgBox "WSAWaitForMultipleEvents failed. Error: " & Err.LastDllError
    Case Else
        MsgBox "Unexpected WSAWaitForMultipleEvents return. Error: " & Err.LastDllError
    End Select
    
    dwRc = WSAEventSelect(sockAll, hEvent, 0)
    If (dwRc = SOCKET_ERROR) Then
        MsgBox "Failed to select event. Error: " & Err.LastDllError
        Exit Sub
    End If

End Sub

'
' Subroutine: txtUiDestAddr_Change
'
' Description:
'    This function sets a flag indicating that a filter has been set.
'
Private Sub txtUiDestAddr_Change()
    bFilter = True
End Sub

'
' Subroutine: txtUiSourceAddr_Change
'
' Description:
'    This function sets a flag indicating that a filter has been set.
'
Private Sub txtuiSourceAddr_Change()
    bFilter = True
End Sub

'
' Subroutine: txtusDestPort_Change
'
' Description:
'    This function sets a flag indicating that a filter has been set.
'
Private Sub txtusDestPort_Change()
    bFilter = True
End Sub

'
' Subroutine: txtusSourcePort_Change
'
' Description:
'    This function sets a flag indicating that a filter has been set.
'
Private Sub txtusSourcePort_Change()
    bFilter = True
End Sub
