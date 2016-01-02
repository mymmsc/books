VERSION 5.00
Begin VB.Form pingfrm 
   Caption         =   "VB Ping"
   ClientHeight    =   5265
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   8085
   LinkTopic       =   "Form1"
   ScaleHeight     =   5265
   ScaleWidth      =   8085
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox chkRoute 
      Caption         =   "Record Route"
      Height          =   255
      Left            =   4800
      TabIndex        =   6
      Top             =   360
      Width           =   1575
   End
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
      Height          =   3660
      Left            =   120
      TabIndex        =   5
      Top             =   1440
      Width           =   7575
   End
   Begin VB.TextBox txtRemoteHost 
      Height          =   375
      Left            =   1200
      TabIndex        =   4
      Text            =   "products1"
      Top             =   360
      Width           =   1215
   End
   Begin VB.TextBox txtPktSize 
      Height          =   375
      Left            =   3720
      TabIndex        =   2
      Text            =   "32"
      Top             =   360
      Width           =   615
   End
   Begin VB.CommandButton cmdPing 
      Caption         =   "Ping"
      Height          =   375
      Left            =   6600
      TabIndex        =   0
      Top             =   360
      Width           =   1215
   End
   Begin VB.Label Label2 
      Caption         =   "Remote Host:"
      Height          =   375
      Left            =   120
      TabIndex        =   3
      Top             =   360
      Width           =   1095
   End
   Begin VB.Label Label1 
      Caption         =   "Packet Size:"
      Height          =   375
      Left            =   2640
      TabIndex        =   1
      Top             =   360
      Width           =   1095
   End
End
Attribute VB_Name = "pingfrm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Project: vbping
'
' Description:
'    This app implments ICMP echo requests (otherwise known as ping). It creates
'    a raw socket of the ICMP protocol and sends echo requests to the specified
'    remote host. Once the remote host receives these, it will respond with an
'    ICMP echo reply message. This tells you that the remote host is running and
'    accessible from the network. The user may specify the data size to send with
'    the echo request as well as request that the record route IP option should
'    be turned on as well. Note that when the record route option is specified,
'    some routers will discard these packets which will generate a time out
'    error. If this occurs turn off the record route option to see if the ICMP
'    requests then succeed.
'
Option Explicit

Const DEF_PACKET_SIZE = 32
Const MAX_PACKET = 1024
Const MAX_IP_HDR_SIZE = 60

Dim nCount As Integer        ' Counter for ICMP echoes sent
Dim seq_no As Integer        ' Keep track of the sequence numbers
'
' Subroutine: cmdPing_Click
'
' Description:
'    This is the event handler for the Ping buttong. When the
'    user clicks this a socket is created, the appropriate socket
'    options are set, and ICMP echo packets are sent to the
'    specified destination.
'
Private Sub cmdPing_Click()
    Dim sockRaw As Long
    Dim dest As sockaddr, from As sockaddr
    Dim bread As Long
    Dim bwrote As Long
    Dim datasize As Long
    Dim fromlen As Long
    Dim timeout As Long
    Dim ret As Long
    Dim icmp_data() As Byte
    Dim recvbuf() As Byte
    Dim addr As Integer
    Dim ipopt As IpOptionHeader
    Dim ICMPhdr As IcmpHeader
    Dim sockErr As Long
   
    cmdPing.Enabled = False
    List1.Clear
    
    nCount = 0
    fromlen = LenB(from)
    sockRaw = INVALID_SOCKET
    addr = 0
    seq_no = 0
    
    If txtRemoteHost.Text = "" Then
        MsgBox "Please specify a remote host to ping to"
        Exit Sub
    End If
    
    sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, ByVal 0, 0, WSA_FLAG_OVERLAPPED)
    
    If sockRaw = INVALID_SOCKET Then
        If Err.LastDllError = WSAEACCES Then
            MsgBox "WSASocket failed. You must be an Administrator to create a raw socket"
            Exit Sub
        Else
            MsgBox "WSASocket failed. Error: " & Err.LastDllError & ". App shuts down."
            Exit Sub
        End If
    End If
    ZeroMemory ipopt, LenB(ipopt)
    '
    ' If the Record Route box is checked setup the IP options header
    '
    If chkRoute.Value = 1 Then
        ipopt.code = IP_RECORD_ROUTE
        ipopt.ptr = 4
        ipopt.len = 39 ' Length of option headers
        
        ret = setsockopt2(sockRaw, IPPROTO_IP, IP_OPTIONS, ipopt, LenB(ipopt))
        If ret = SOCKET_ERROR Then
           MsgBox "setsockopt IP_OPTIONS failed. Error: " & Err.LastDllError
        End If
    End If
    '
    ' Set the receive timeout value for the socket
    '
    timeout = 1000
    bread = setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, timeout, LenB(timeout))
    If bread = SOCKET_ERROR Then
        MsgBox "setsockopt SO_RCVTIMEO failed. Error: " & Err.LastDllError
        closesocket sockRaw
        cmdPing.Enabled = True
        Exit Sub
    End If
    '
    ' Resolve the given name and setup the ICMP header
    '
    ZeroMemory dest, LenB(dest)
    dest.sin_family = AF_INET
    dest.sin_addr = GetHostByNameAlias(txtRemoteHost.Text)

    datasize = CLng(txtPktSize.Text)
    If datasize = 0 Then
        datasize = DEF_PACKET_SIZE
    End If
    
    datasize = datasize + LenB(ICMPhdr)
    ReDim icmp_data(datasize)
    ReDim recvbuf(MAX_PACKET)
        
    ZeroMemory icmp_data(0), datasize
    ZeroMemory recvbuf(0), MAX_PACKET
    
    ICMPhdr.i_type = ICMP_ECHO
    ICMPhdr.i_code = 0
    ICMPhdr.i_id = GetCurrentProcessId Mod 65535
    '
    ' Place some junk in the buffer. You could put anything here.
    '
    Dim i As Long
    For i = LenB(ICMPhdr) To datasize - 1
        icmp_data(i) = Asc("E")
    Next
    '
    ' Now send the ICMP echo request packets and wait for the reply
    '
    Do While True
        If (nCount = 4) Then Exit Do
        nCount = nCount + 1
        ICMPhdr.i_cksum = 0
        ICMPhdr.timestamp = GetTickCount
        ICMPhdr.i_seq = seq_no
        seq_no = seq_no + 1
        CopyMemory icmp_data(0), ICMPhdr, LenB(ICMPhdr)
        
        CheckSum icmp_data, datasize
        '
        ' Send the ICMP echo request
        '
        bwrote = sendto(sockRaw, icmp_data(0), datasize, 0, dest, LenB(dest))
        If bwrote = SOCKET_ERROR Then
            sockErr = Err.LastDllError
            If sockErr = WSAETIMEDOUT Then
                List1.AddItem "timed out"
                GoTo NextLoop
            Else
                MsgBox "sendto failed: " & sockErr
                closesocket sockRaw
                cmdPing.Enabled = True
                Exit Sub
            End If
        End If
        
        If bwrote < datasize Then
            List1.AddItem "Worte " & bwrote & " bytes"
        End If
        Dim formlen As Long
        formlen = LenB(from)
        '
        ' Read the reply (or timeout if no reply is received)
        '
        bread = recvfrom(sockRaw, recvbuf(0), MAX_PACKET, 0, from, fromlen)
                
        If bread = SOCKET_ERROR Then
            sockErr = Err.LastDllError
            If sockErr = WSAETIMEDOUT Then
                List1.AddItem "timed out"
                GoTo NextLoop
            Else
                MsgBox "recvfrom failed: " & sockErr
                closesocket sockRaw
                cmdPing.Enabled = True
                Exit Sub
            End If
        End If
        '
        ' Decode the message to see if its ours and whether its
        ' an echo reply
        '
        DecodeICMPHeader recvbuf, bread, from
        Sleep (1000)
        
    
NextLoop:
    Loop
    
    If sockRaw <> INVALID_SOCKET Then closesocket sockRaw
    cmdPing.Enabled = True
   
    
End Sub


Private Sub Form_Load()
    If TCPIPStartup Then
    Else
        MsgBox "Windows Sockets not initialized. Error: " & Err.LastDllError
    End If
    nCount = 0
End Sub

Private Sub Form_Unload(Cancel As Integer)
    TCPIPShutDown
End Sub

'
' Subroutine: DecodeIPOptions
'
' Description:
'    If the Record Route option was checked, then this routine
'    decodes the option header and prints each IP address
'    contained therein
'
Private Sub DecodeIPOptions(buff() As Byte, ByVal bytes As Long)
    Dim ipopt As IpOptionHeader
    Dim host As HostEnt, hostent_addr As Long
    Dim hostip_addr As Long
    Dim temp_ip_address() As Byte
    Dim i As Integer, j As Integer
    Dim ip_address As String
    Dim tmpStr As String
    Dim tmpStr2 As String
    tmpStr = String(512, 0)
    tmpStr2 = String(512, 0)
    
    '
    ' Step through each 4 byte entry to get an IP address
    '
    List1.AddItem "RR:   "
    CopyMemory ipopt, buff(20), 3
    CopyMemory ipopt.addr(0), buff(23), 36
    For i = 0 To ipopt.ptr \ 4 - 2
        tmpStr = "      "
        
        hostent_addr = gethostbyaddr(ipopt.addr(i), LenB(ipopt.addr(i)), AF_INET)
    
        If hostent_addr = 0 Then
            lstrcpy1 tmpStr2, inet_ntoa(ipopt.addr(i))
            tmpStr2 = Left(tmpStr2, InStr(tmpStr2, Chr(0)) - 1)
            List1.AddItem tmpStr & tmpStr2
        Else
    
            CopyMemory host, ByVal hostent_addr, LenB(host)
            CopyMemory hostip_addr, ByVal host.h_addr_list, 4
        
            tmpStr2 = String(255, 0)
            lstrcpy1 tmpStr2, host.h_name
            tmpStr2 = Left(tmpStr2, InStr(tmpStr2, Chr(0)) - 1)
            
            ReDim temp_ip_address(1 To host.h_length)
            CopyMemory temp_ip_address(1), ByVal hostip_addr, host.h_length
           
            For j = 1 To host.h_length
               ip_address = ip_address & temp_ip_address(j) & "."
            Next
            ip_address = Mid(ip_address, 1, Len(ip_address) - 1)
           
            List1.AddItem tmpStr & ip_address & "  " & tmpStr2
        End If
               
        tmpStr = ""
        ip_address = ""
    Next

End Sub

'
' Subroutine: DecodeICMPHeader
'
' Description:
'    This routine decodes the ICMP responses received. It makes
'    sure that the packet read originated from us as well as
'    determine whether it is an echo response or some other
'    ICMP message.
'
Private Sub DecodeICMPHeader(buff() As Byte, ByVal bytes As Long, from As sockaddr)
    Dim IPhdr As IpHeader, ICMPhdr As IcmpHeader, IPHdrLen As Integer, tick As Long, addrs As String
    
    
    CopyMemory IPhdr, buff(0), LenB(IPhdr)
      
    IPHdrLen = (IPhdr.h_len And &HF) * 4
    tick = GetTickCount
    
    If (IPHdrLen = MAX_IP_HDR_SIZE) And (nCount = 1) Then
        DecodeIPOptions buff, bytes
    End If
        
    If bytes < IPHdrLen + ICMP_MIN Then
        List1.AddItem "Too few bytes from " & inet_ntoa(from.sin_addr)
    End If
    
    CopyMemory ICMPhdr, buff(IPHdrLen), LenB(ICMPhdr)
      
    If ICMPhdr.i_type <> ICMP_ECHOREPLY Then
         List1.AddItem "Non-Echo type " & ICMPhdr.i_type & " recvd"
         Exit Sub
      End If
      
      If ICMPhdr.i_id <> (GetCurrentProcessId() Mod 65535) Then
         List1.AddItem "Somehow we got someone else's packet!"
         Exit Sub
      End If
      
      addrs = inet_ntoa(from.sin_addr)
      Dim tmpStr As String
      Dim tmpLen As Long
      tmpStr = String(256, 0)
      lstrcpy1 tmpStr, addrs
      tmpStr = Left(tmpStr, InStr(tmpStr, Chr(0)) - 1)
      List1.AddItem bytes & " bytes from " & tmpStr & ": icmp_seq = " & ICMPhdr.i_seq & ". time: " & tick - ICMPhdr.timestamp & " ms "
      DoEvents
End Sub
'
' The following three functions were provided by GaryY:
' CheckSum
' LongAdd
' Flip
'
Function CheckSum(buff() As Byte, ByVal datasize As Long) As Double
   ' The CheckSum formula (the hardest part of this code to figure out)
   ' requires you to do long unsigned arithmetic, adding each of the octets
   ' in the ICMP frame as short unsigned integers (2 bytes).
   ' (1) Total the octets as if you were adding them up in a 4 byte
   '     unsigned long.  When you overrun the max value of the unsigned
   '     long, start over at zero plus whatever portion is left over.
   ' (2) Once you have the total, add the high two bytes to the low
   '     two bytes to make an unsigned integer (2 bytes) value.
   ' (3) Now invert, (ones-complement) the value and place the the two byte
   '     value in the 16-Bit checksum field of the ICMP frame (bytes 2 & 3).
   Dim cksum As Double, j As Double, acc As Double
   Dim i As Long

   cksum = 0   ' (1) add the shorts
   For i = 0 To datasize - 2 Step 2
      cksum = LongAdd(cksum, CLng(buff(i + 1) * 256#))
      cksum = LongAdd(cksum, CLng(buff(i)))
   Next i
   cksum = LongAdd(cksum, CLng(buff(i)))
   
   j = cksum Mod 65536#  ' (2) add the two high bytes to the two low bytes
   acc = j + (cksum \ (2 ^ 16))
   
   buff(2) = Flip(acc Mod (256)) ' (3) Perform a Ones-Complement on each byte
   acc = acc - acc Mod 256       '     individually and place the bytes in the
   buff(3) = Flip(acc \ 256)     '     ICMP Message
End Function

Function LongAdd(d As Double, s As Long) As Double
   ' Emulate unsigned long integer arithmetic by storing
   ' the value in a double and MOD on the value whenever
   ' it is greater than MAXLONG
   Const MAXLONG = 4294967295#
   d = d + s
   
   If d > MAXLONG Then  ' If you went over
      d = d Mod MAXLONG ' wrap back around to zero plus
   End If               ' the amount you went over
   LongAdd = d
End Function

Function Flip(b As Byte) As Byte
   ' Ones-complement is simply inverting every bit in a value
   ' 11100100  (0xE4)  becomes
   ' 00011011  (0x1B)
   ' This accomplished by accumulating the sum of two raised to
   ' the x power for each mod 2 of the original value, and then
   ' dividing the value by 2, eight times.
   Dim i As Byte, accum As Byte, x As String
   accum = 0
   For i = 0 To 7
      If b Mod 2 Then
         x = "1" & x
      Else
         x = "0" & x
         accum = accum + 2 ^ i
      End If
      b = b \ 2
   Next i
   Flip = accum
End Function


