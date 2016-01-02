VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "WSAEnumProtocols Output"
   ClientHeight    =   7725
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9795
   LinkTopic       =   "Form1"
   ScaleHeight     =   7725
   ScaleWidth      =   9795
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox List1 
      BeginProperty Font 
         Name            =   "Fixedsys"
         Size            =   9
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   7260
      Left            =   240
      TabIndex        =   0
      Top             =   120
      Width           =   9375
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: enumvb
'
' Description:
'    This project enumerates all of the Winsock catalog entries and
'    displays them in a list box. This is done using the WSAEnumProtocols
'    API.
'

Option Explicit

Dim EnumStrAll As String

'
' Subroutine: DoEnum
'
' Description:
'    This routine initializes Winsock and then calls WSAEnumProtocols.
'    This first call to this function is done to obtain the size of
'    the buffer required for the call to complete successfully. We
'    then allocate a buffer of that size and call the function again.
'
Private Sub DoEnum()
    Dim wsadStartupData As WSADataType
    Dim dwRet As Long, dwBufLen As Long, dwErr As Long
    Dim pProtoBuffer As Long
    Dim i As Long
    
    dwRet = WSAStartup(&H202, wsadStartupData)
    If dwRet <> 0 Then
        MsgBox "Error WSAStartup: " & Err.LastDllError
        Exit Sub
    End If
    '
    ' Call the function with NULL and 0 in order to obtain the
    '  necessary buffer size.
    '
    dwBufLen = 0
    dwRet = WSAEnumProtocols(0, 0, dwBufLen)
    If dwRet = SOCKET_ERROR Then 'it's going to happ as buffer is 0
        dwErr = Err.LastDllError
        If dwErr <> WSAENOBUFS And dwErr <> 0 Then
            MsgBox "Error WSAEnumProtocols: " & dwErr
            WSACleanup
            Exit Sub
        End If
        '
        ' Allocate the buffer and call it again.
        '
        pProtoBuffer = GlobalAlloc(GMEM_FIXED, dwBufLen)
        dwRet = WSAEnumProtocols(0, pProtoBuffer, dwBufLen)
        If dwRet = SOCKET_ERROR Then
            dwErr = Err.LastDllError
            MsgBox "Error WSAEnumPrtocols: " & dwErr
            GlobalFree pProtoBuffer
            WSACleanup
            Exit Sub
        End If
        
        List1.AddItem "Num Protocols found: " & dwRet
        
        Dim pWorkPointer As Long
        '
        ' Print the info
        '
        For i = 0 To dwRet - 1
            PrintProtocolInfo pProtoBuffer, i
        Next i
    End If
    GlobalFree pProtoBuffer
    WSACleanup
End Sub

'
' Subroutine: PrintProtocolInfo
'
' Description:
'    This function is responsible for taking each WSAPROTOCOL_INFO
'    structure returned from WSAEnumProtocols and printing the
'    contained information to a window in human readable form.
'
Sub PrintProtocolInfo(pProtoBuffer As Long, index As Long)
    Dim wsapi As WSAPROTOCOL_INFO
    CopyMemory2 wsapi, pProtoBuffer + index * LenB(wsapi), LenB(wsapi)
    Dim szProtocol As String
    szProtocol = String(256, 0)
    lstrcpy szProtocol, wsapi.szProtocol(0)
    szProtocol = Trim(szProtocol)
    List1.AddItem ""
    List1.AddItem "Protocol: " & szProtocol
    EnumStrAll = "           Address Family: "
    
    Select Case (wsapi.iAddressFamily)
        Case AF_INET:
            EnumStrAll = EnumStrAll & "AF_INET"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            Select Case (wsapi.iProtocol)
                Case IPPROTO_IP:
                    EnumStrAll = EnumStrAll & "IPROTO_IP"
                Case IPPROTO_ICMP:
                    EnumStrAll = EnumStrAll & "IPROTO_ICMP"
                Case IPPROTO_IGMP:
                    EnumStrAll = EnumStrAll & "IPROTO_IGMP"
                Case IPPROTO_GGP:
                    EnumStrAll = EnumStrAll & "IPROTO_GGP"
                Case IPPROTO_TCP:
                    EnumStrAll = EnumStrAll & "IPROTO_TCP"
                Case IPPROTO_PUP:
                    EnumStrAll = EnumStrAll & "IPROTO_PUP"
                Case IPPROTO_UDP:
                    EnumStrAll = EnumStrAll & "IPROTO_UDP"
                Case IPPROTO_IDP:
                    EnumStrAll = EnumStrAll & "IPROTO_IDP"
                Case IPPROTO_ND:
                    EnumStrAll = EnumStrAll & "IPROTO_ND"
                Case IPPROTO_RAW:
                    EnumStrAll = EnumStrAll & "IPROTO_RAW"
            End Select
            List1.AddItem EnumStrAll
        Case AF_INET6:
            EnumStrAll = EnumStrAll & "AF_INET6"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            Select Case (wsapi.iProtocol)
                Case IPPROTO_IP:
                    EnumStrAll = EnumStrAll & "IPROTO_IP"
               Case IPPROTO_ICMP:
                    EnumStrAll = EnumStrAll & "IPROTO_ICMP"
                Case IPPROTO_IGMP:
                    EnumStrAll = EnumStrAll & "IPROTO_IGMP"
                Case IPPROTO_GGP:
                    EnumStrAll = EnumStrAll & "IPROTO_GGP"
                Case IPPROTO_TCP:
                    EnumStrAll = EnumStrAll & "IPROTO_TCP"
                Case IPPROTO_PUP:
                    EnumStrAll = EnumStrAll & "IPROTO_PUP"
                Case IPPROTO_UDP:
                    EnumStrAll = EnumStrAll & "IPROTO_UDP"
                Case IPPROTO_IDP:
                    EnumStrAll = EnumStrAll & "IPROTO_IDP"
                Case IPPROTO_ND:
                    EnumStrAll = EnumStrAll & "IPROTO_ND"
                Case IPPROTO_RAW:
                    EnumStrAll = EnumStrAll & "IPROTO_RAW"
            End Select
            List1.AddItem EnumStrAll
        Case AF_UNSPEC:
            EnumStrAll = EnumStrAll & "AF_UNSPEC"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_UNIX:
            EnumStrAll = EnumStrAll & "AF_UNIX"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_IMPLINK:
            EnumStrAll = EnumStrAll & "AF_IMPLINK"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_PUP:
            EnumStrAll = EnumStrAll & "AF_PUP"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_CHAOS:
            EnumStrAll = EnumStrAll & "AF_CHAOS"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_NS:
            EnumStrAll = EnumStrAll & "AF_NS or AF_IPX"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            Select Case wsapi.iProtocol
                Case NSPROTO_IPX:
                    EnumStrAll = EnumStrAll & "NSPROTO_IPX"
                Case NSPROTO_SPX:
                    EnumStrAll = EnumStrAll & "NSPROTO_SPX"
               Case NSPROTO_SPXII:
                    EnumStrAll = EnumStrAll & "NSPROTO_SPXII"
           End Select
           List1.AddItem EnumStrAll
        Case AF_ISO:
            EnumStrAll = EnumStrAll & "AF_ISO or AF_OSI"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_ECMA:
            EnumStrAll = EnumStrAll & "AF_ECMA"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_DATAKIT:
            EnumStrAll = EnumStrAll & "AF_DATAKIT"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_CCITT:
            EnumStrAll = EnumStrAll & "AF_CCITT"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_SNA:
            EnumStrAll = EnumStrAll & "AF_SNA"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_DECnet:
            EnumStrAll = EnumStrAll & "AF_DECnet"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_DLI:
            EnumStrAll = EnumStrAll & "AF_DLI"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_LAT:
            EnumStrAll = EnumStrAll & "AF_LAT"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_HYLINK:
            EnumStrAll = EnumStrAll & "AF_HYLINK"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_APPLETALK:
            EnumStrAll = EnumStrAll & "AF_APPLETALK"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            Select Case (wsapi.iProtocol)
                Case DDPPROTO_RTMP:
                    EnumStrAll = EnumStrAll & "DDPPROTO_RTMP"
                Case DDPPROTO_NBP:
                    EnumStrAll = EnumStrAll & "DDPPROTO_NBP"
                Case DDPPROTO_ATP:
                    EnumStrAll = EnumStrAll & "DDPROTO_ATP"
                Case DDPPROTO_AEP:
                    EnumStrAll = EnumStrAll & "DDPPROTO_AEP"
                Case DDPPROTO_RTMPRQ:
                    EnumStrAll = EnumStrAll & "DDPPROTO_RTMPRQ"
                Case DDPPROTO_ZIP:
                    EnumStrAll = EnumStrAll & "DDPPROTO_ZIP"
                Case DDPPROTO_ADSP:
                    EnumStrAll = EnumStrAll & "DDPPROTO_ADSP"
                Case ATPROTO_ADSP:
                    EnumStrAll = EnumStrAll & "ATPROTO_ADSP"
                Case ATPROTO_ATP:
                    EnumStrAll = EnumStrAll & "ATPROTO_ATP"
                Case ATPROTO_ASP:
                    EnumStrAll = EnumStrAll & "ATPROTO_ASP"
                Case ATPROTO_PAP:
                    EnumStrAll = EnumStrAll & "ATPROTO_PAP"
            End Select
            List1.AddItem EnumStrAll
        Case AF_NETBIOS:
            EnumStrAll = EnumStrAll & "AF_NETBIOS"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            If wsapi.iProtocol = &H80000000 Then
                EnumStrAll = EnumStrAll & "NetBIOS LANA " & 0
                List1.AddItem EnumStrAll
            Else
                EnumStrAll = EnumStrAll & "NetBIOS LANA " & Abs(wsapi.iProtocol)
                List1.AddItem EnumStrAll
            End If
        Case AF_VOICEVIEW:
            EnumStrAll = EnumStrAll & "AF_VOICEVIEW"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_FIREFOX:
            EnumStrAll = EnumStrAll & "AF_FIREFOX"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_UNKNOWN1:
            EnumStrAll = EnumStrAll & "AF_UNKNOWN1"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_BAN:
            EnumStrAll = EnumStrAll & "AF_BAN"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_ATM:
            EnumStrAll = EnumStrAll & "AF_ATM"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            Select Case (wsapi.iProtocol)
                Case ATMPROTO_AALUSER:
                    EnumStrAll = EnumStrAll & "ATMPROTO_AALUSER"
                    List1.AddItem EnumStrAll
                Case ATMPROTO_AAL1:
                    EnumStrAll = EnumStrAll & "ATMPROTO_AAL1"
                    List1.AddItem EnumStrAll
                Case ATMPROTO_AAL2:
                    EnumStrAll = EnumStrAll & "ATMPROTO_AAL2"
                    List1.AddItem EnumStrAll
                Case ATMPROTO_AAL34:
                    EnumStrAll = EnumStrAll & "ATMPROTO_AAL34"
                    List1.AddItem EnumStrAll
                Case ATMPROTO_AAL5:
                    EnumStrAll = EnumStrAll & "ATMPROTO_AAL5"
                    List1.AddItem EnumStrAll
            End Select
        Case AF_CLUSTER:
            EnumStrAll = EnumStrAll & "AF_CLUSTER"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_12844:
            EnumStrAll = EnumStrAll & "AF_12844"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: UNKNOWN: " & wsapi.iProtocol
        Case AF_IRDA:
            EnumStrAll = EnumStrAll & "AF_IRDA"
            List1.AddItem EnumStrAll
            EnumStrAll = "                 Protocol: "
            Select Case (wsapi.iProtocol)
               ' Case IRDA_PROTO_SOCK_STREAM:
                '    EnumStrAll = EnumStrAll & "IRDA_PROTO_SOCK_STREAM" & vbCrLf
            End Select
        Case Else:
            EnumStrAll = EnumStrAll & "Unknown: " & wsapi.iAddressFamily
            List1.AddItem EnumStrAll
    End Select
    
    
    EnumStrAll = "              Socket Type: "
    Select Case (wsapi.iSocketType)
        Case SOCK_STREAM:
            EnumStrAll = EnumStrAll & "SOCK_STREAM"
        Case SOCK_DGRAM:
            EnumStrAll = EnumStrAll & "SOCK_DGRAM"
        Case SOCK_RAW:
            EnumStrAll = EnumStrAll & "SOCK_RAW"
        Case SOCK_RDM:
            EnumStrAll = EnumStrAll & "SOCK_RDM"
        Case SOCK_SEQPACKET:
            EnumStrAll = EnumStrAll & "SOCK_SEQPACKET"
    End Select
    List1.AddItem EnumStrAll
    EnumStrAll = "           Connectionless: "
    If (wsapi.dwServiceFlags1 And XP1_CONNECTIONLESS) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "      Guaranteed Delivery: "
    If (wsapi.dwServiceFlags1 And XP1_GUARANTEED_DELIVERY) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "         Guaranteed Order: "
    If (wsapi.dwServiceFlags1 And XP1_GUARANTEED_ORDER) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "         Message Oriented: "
    If (wsapi.dwServiceFlags1 And XP1_MESSAGE_ORIENTED) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "            Pseudo Stream: "
    If (wsapi.dwServiceFlags1 And XP1_PSEUDO_STREAM) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "           Graceful Close: "
    If (wsapi.dwServiceFlags1 And XP1_GRACEFUL_CLOSE) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "          Expedited Close: "
    If (wsapi.dwServiceFlags1 And XP1_EXPEDITED_DATA) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "             Connect Data: "
    If (wsapi.dwServiceFlags1 And XP1_CONNECT_DATA) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "          Disconnect Data: "
    If (wsapi.dwServiceFlags1 And XP1_DISCONNECT_DATA) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "       Supports Broadcast: "
    If (wsapi.dwServiceFlags1 And XP1_SUPPORT_BROADCAST) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "      Supports Multipoint: "
    If (wsapi.dwServiceFlags1 And XP1_SUPPORT_MULTIPOINT) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = " Multipoint Control Plane: "
    If (wsapi.dwServiceFlags1 And XP1_MULTIPOINT_CONTROL_PLANE) Then
        EnumStrAll = EnumStrAll & "ROOTED"
    Else
        EnumStrAll = EnumStrAll & "NON-ROOTED"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "    Multipoint Data Plane: "
    If (wsapi.dwServiceFlags1 And XP1_MULTIPOINT_DATA_PLANE) Then
        EnumStrAll = EnumStrAll & "ROOTED"
    Else
        EnumStrAll = EnumStrAll & "NON-ROOTED"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "            QoS Supported: "
    If (wsapi.dwServiceFlags1 And XP1_QOS_SUPPORTED) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "     Unidirectional Sends: "
    If (wsapi.dwServiceFlags1 And XP1_UNI_SEND) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "    Unidirection Receives: "
    If (wsapi.dwServiceFlags1 And XP1_UNI_RECV) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "              IFS Handles: "
    If (wsapi.dwServiceFlags1 And XP1_IFS_HANDLES) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "         Partial Messages: "
    If (wsapi.dwServiceFlags1 And XP1_PARTIAL_MESSAGE) Then
        EnumStrAll = EnumStrAll & "YES"
    Else
        EnumStrAll = EnumStrAll & "NO"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "           Provider Flags: "
    Select Case (wsapi.dwProviderFlags)
        Case PFL_MULTIPLE_PROTO_ENTRIES:
            EnumStrAll = EnumStrAll & "This is one or more entries for a protocol"
            List1.AddItem EnumStrAll
            EnumStrAll = vbTab & vbTab & vbTab & "   which is capable of implementing multiple"
            List1.AddItem EnumStrAll
            EnumStrAll = vbTab & vbTab & vbTab & "   behaviors"
            List1.AddItem EnumStrAll
        Case PFL_RECOMMENDED_PROTO_ENTRY:
            EnumStrAll = EnumStrAll & "This entry is recommended for a protocol"
            List1.AddItem EnumStrAll
            EnumStrAll = vbTab & vbTab & vbTab & "   that is capable of multple behaviors"
            List1.AddItem EnumStrAll
        Case PFL_HIDDEN:
            List1.AddItem EnumStrAll
            EnumStrAll = "You shouldn't be seeing this"
            List1.AddItem EnumStrAll
        Case PFL_MATCHES_PROTOCOL_ZERO:
            EnumStrAll = EnumStrAll & "A value of zero in the protocol parameter"
            List1.AddItem EnumStrAll
            EnumStrAll = vbTab & vbTab & vbTab & "   of socket or WSASocket matches this entry"
            List1.AddItem EnumStrAll
        Case Default:
            EnumStrAll = EnumStrAll & "NONE"
            List1.AddItem EnumStrAll
    End Select
    Dim szGuidString As String
    szGuidString = String(256, 0)
    StringFromGUID2 wsapi.ProviderId, szGuidString, 256
    szGuidString = StrConv(szGuidString, vbFromUnicode)
    EnumStrAll = "              Provider Id: " & szGuidString
    List1.AddItem EnumStrAll
    EnumStrAll = "         Catalog Entry Id: " & wsapi.dwCatalogEntryId
    List1.AddItem EnumStrAll
    EnumStrAll = "  Number of Chain Entries: " & wsapi.ProtocolChain.ChainLen & "   {"
    Dim i As Long
    For i = 0 To wsapi.ProtocolChain.ChainLen - 1
        EnumStrAll = EnumStrAll & wsapi.ProtocolChain.ChainEntries(i) & " "
    Next
    EnumStrAll = EnumStrAll & "}"
    List1.AddItem EnumStrAll
    EnumStrAll = "                  Version: " & wsapi.iVersion
    List1.AddItem EnumStrAll
    EnumStrAll = "Max Socket Address Length: " & wsapi.iMaxSockAddr
    List1.AddItem EnumStrAll
    EnumStrAll = "Min Socket Address Length: " & wsapi.iMinSockAddr
    List1.AddItem EnumStrAll
    EnumStrAll = "      Protocol Max Offset: " & wsapi.iProtocolMaxOffset
    List1.AddItem EnumStrAll
    EnumStrAll = "       Network Byte Order: "
    If (wsapi.iNetworkByteOrder = 0) Then
        EnumStrAll = EnumStrAll & "BIG-ENDIAN"
    Else
        EnumStrAll = EnumStrAll & "LITLE-ENDIAN"
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "          Security Scheme: "
    
    If (wsapi.iSecurityScheme = SECURITY_PROTOCOL_NONE) Then
        EnumStrAll = EnumStrAll & "NONE"
    Else
        EnumStrAll = EnumStrAll & wsapi.iSecurityScheme
    End If
    List1.AddItem EnumStrAll
    EnumStrAll = "             Message Size: "
    If (wsapi.dwMessageSize = 0) Then
        EnumStrAll = EnumStrAll & "N/A (Stream Oriented)"
    ElseIf (wsapi.dwMessageSize = 1) Then
        EnumStrAll = EnumStrAll & "Depended on underlying MTU"
    ElseIf (wsapi.dwMessageSize = &HFFFFFFFF) Then
        EnumStrAll = EnumStrAll & "No limit"
    Else
        EnumStrAll = EnumStrAll & wsapi.dwMessageSize
    End If
    List1.AddItem EnumStrAll
End Sub

'
' Subroutine: Form_Load
'
' Description:
'    Initalize a global variable and kick things off by calling
'    the DoEnum routine.
'
Private Sub Form_Load()
    EnumStrAll = ""
    DoEnum
End Sub

