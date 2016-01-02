Attribute VB_Name = "parser"
'
' This file contains routines for parsing IP packets and generating
'  human readable text. Note that a parser for every protocol is not
'  provided. We parse only those protocols were interested in: TCP,
'  UDP, IGMP, etc.
'

Option Explicit
Option Base 0


Global Const MAX_IP_SIZE = 65535
Global Const MIN_IP_HDR_SIZE = 20
Global szProto() As String
Global szIgmpType() As String
Global bFilter As Boolean

Sub FillSzProto()
                 ReDim szProto(100)
                 szProto(0) = "Reserved"     '//  0
                 szProto(1) = "ICMP"         '//  1
                 szProto(2) = "IGMP"         '//  2
                 szProto(3) = "GGP"          '//  3
                 szProto(4) = "IP"           '//  4
                 szProto(5) = "ST"           '//  5
                 szProto(6) = "TCP"          '//  6
                 szProto(7) = "UCL"          '//  7
                 szProto(8) = "EGP"          '//  8
                 szProto(9) = "IGP"          '//  9
                 szProto(10) = "BBN-RCC-MON" '// 10
                 szProto(11) = "NVP-II"      '// 11
                 szProto(12) = "PUP"         '// 12
                 szProto(13) = "ARGUS"       '// 13
                 szProto(14) = "EMCON"       '// 14
                 szProto(15) = "XNET"        '// 15
                 szProto(16) = "CHAOS"       '// 16
                 szProto(17) = "UDP"         '// 17
                 szProto(18) = "MUX"         '// 18
                 szProto(19) = "DCN-MEAS"    '// 19
                 szProto(20) = "HMP"         '// 20
                 szProto(21) = "PRM"         '// 21
                 szProto(22) = "XNS-IDP"     '// 22
                 szProto(23) = "TRUNK-1"     '// 23
                 szProto(24) = "TRUNK-2"     '// 24
                 szProto(25) = "LEAF-1"      '// 25
                 szProto(26) = "LEAF-2"      '// 26
                 szProto(27) = "RDP"         '// 27
                 szProto(28) = "IRTP"        '// 28
                 szProto(29) = "ISO-TP4"     '// 29
                 szProto(30) = "NETBLT"      '// 30
                 szProto(31) = "MFE-NSP"     '// 31
                 szProto(32) = "MERIT-INP"   '// 32
                 szProto(33) = "SEP"         '// 33
                 szProto(34) = "3PC"         '// 34
                 szProto(35) = "IDPR"        '// 35
                 szProto(36) = "XTP"         '// 36
                 szProto(37) = "DDP"         '// 37
                 szProto(38) = "IDPR-CMTP"   '// 38
                 szProto(39) = "TP++"        '// 39
                 szProto(40) = "IL"          '// 40
                 szProto(41) = "SIP"         '// 41
                 szProto(42) = "SDRP"        '// 42
                 szProto(43) = "SIP-SR"      '// 43
                 szProto(44) = "SIP-FRAG"    '// 44
                 szProto(45) = "IDRP"        '// 45
                 szProto(46) = "RSVP"        '// 46
                 szProto(47) = "GRE"         '// 47
                 szProto(48) = "MHRP"        '// 48
                 szProto(49) = "BNA"         '// 49
                 szProto(50) = "SIPP-ESP"    '// 50
                 szProto(51) = "SIPP-AH"     '// 51
                 szProto(52) = "I-NLSP"      '// 52
                 szProto(53) = "SWIPE"       '// 53
                 szProto(54) = "NHRP"        '// 54
                 szProto(55) = "unassigned"  '// 55
                 szProto(56) = "unassigned"  '// 56
                 szProto(57) = "unassigned"  '// 57
                 szProto(58) = "unassigned"  '// 58
                 szProto(59) = "unassigned"  '// 59
                 szProto(60) = "unassigned"  '// 60
                 szProto(61) = "any host internal protocol" '// 61
                 szProto(62) = "CFTP"        '// 62
                 szProto(63) = "any local network"          '// 63
                 szProto(64) = "SAT-EXPAK"   '// 64
                 szProto(65) = "KRYPTOLAN"   '// 65
                 szProto(66) = "RVD"         '// 66
                 szProto(67) = "IPPC"        '// 67
                 szProto(68) = "any distributed file system"  '// 68
                 szProto(69) = "SAT-MON"   '// 69
                 szProto(60) = "VISA"      '// 70
                 szProto(71) = "IPCV"      '// 71
                 szProto(72) = "CPNX"      '// 72
                 szProto(73) = "CPHB"      '// 73
                 szProto(74) = "WSN"       '// 74
                 szProto(75) = "PVP"       '// 75
                 szProto(76) = "BR-SAT-MON"  '// 76
                 szProto(77) = "SUN-ND"    '// 77
                 szProto(78) = "WB-MON"    '// 78
                 szProto(79) = "WB-EXPAK"  '// 79
                 szProto(80) = "ISO-IP"    '// 80
                 szProto(81) = "VMTP"      '// 81
                 szProto(82) = "SECURE-VMTP" '// 82
                 szProto(83) = "VINES"     '// 83
                 szProto(84) = "TTP"       '// 84
                 szProto(85) = "NSFNET-IGP"  '// 85
                 szProto(86) = "DGP"       '// 86
                 szProto(87) = "TCF"       '// 87
                 szProto(88) = "IGRP"      '// 88
                 szProto(89) = "OSPFIGP"   '// 89
                 szProto(90) = "Sprite-RPC"  '// 90
                 szProto(91) = "LARP"      '// 91
                 szProto(92) = "MTP"       '// 92
                 szProto(93) = "AX.25"     '// 93
                 szProto(94) = "IPIP"      '// 94
                 szProto(95) = "MICP"      '// 95
                 szProto(96) = "SCC-SP"    '// 96
                 szProto(97) = "ETHERIP"   '// 97
                 szProto(98) = "ENCAP"     '// 98
                 szProto(98) = "any private encryption scheme"   '// 98
                 szProto(99) = "GMTP"        '// 99
End Sub


'//
'// The types of IGMP messages
'//
Sub FillSzIgmpType()
    ReDim szIgmpType(8)
    szIgmpType(0) = ""
    szIgmpType(1) = "Host Membership Query"
    szIgmpType(2) = "Host Membership Report"
    szIgmpType(3) = ""
    szIgmpType(4) = ""
    szIgmpType(5) = ""
    szIgmpType(6) = "Version 2 Membership Report"
    szIgmpType(7) = "Leave Group"
End Sub

'
' Function: HI_WORD
'
' Description:
'    This function returns the high 4 bits of a byte
'
Function HI_WORD(bt As Byte) As Long
    HI_WORD = (CLng(bt) \ 16) And &HF&
End Function

'
' Function: LO_WORD
'
' Description:
'    This function returns the low 4 bits of a byte
'
Function LO_WORD(bt As Byte) As Long
    LO_WORD = CLng(bt) And &HF&
End Function


'//
'// Function: PrintRawBytes
'//
'// Description:
'//    This function simply prints out a series of bytes
'//    as hexadecimal digits.
'//
Function PrintRawBytes(ptr() As Byte, ByVal dwlen As Long) As String
    Dim i As Long
    Dim ptrpos As Long
    Dim strHex As String
    
    ptrpos = 0
    Do While dwlen > 0
        strHex = ""
        For i = 0 To 19
            strHex = strHex & Hex(HI_WORD(ptr(ptrpos))) & Hex(LO_WORD(ptr(ptrpos))) & " "
            dwlen = dwlen - 1
            ptrpos = ptrpos + 1
            If dwlen = 0 Then Exit Do
        Next i
        frmrcvall.List2.AddItem strHex
    Loop
    PrintRawBytes = strHex
    
End Function


'//
'// Function: DecodeIGMPHeader
'//
'// Description:
'//    This function takes a pointer to a buffer containing
'//    an IGMP packet and prints it out in a readable form.
'//
Sub DecodeIGMPHeader(ptr() As Byte, ByVal iphdrlen As Long)
    Dim chksum As Long, version As Long, ntype As Long, maxresptime As Long
    Dim addr As sockaddr
    Dim hdr As Byte
    Dim pos As Long
    Dim strIGMPHeader As String
    
    pos = iphdrlen
    hdr = ptr(pos)
    strIGMPHeader = ""
    
    version = HI_WORD(hdr)
    ntype = LO_WORD(hdr)
    
    pos = pos + 1
    
    maxresptime = ptr(pos)
    pos = pos + 1
    
    CopyMemory chksum, ptr(pos), 2
    chksum = ntohs(chksum) And &HFFFF&
    pos = pos + 2
    
    CopyMemory addr.sin_addr, ptr(pos), 4
    frmrcvall.List2.AddItem "   IGMP HEADER:"
    
    If ntype = 1 Or ntype = 2 Then
        version = 1
    Else
        version = 2
    End If
    
    frmrcvall.List2.AddItem "   IGMP Version = " & version
    frmrcvall.List2.AddItem "   IGMP Type = " & szIgmpType(ntype)
    
    If version = 2 Then
        Dim strIP As String
        strIP = String(256, 0)
        lstrcpy1 strIP, inet_ntoa(addr.sin_addr)
        strIP = Trim(strIP)
        frmrcvall.List2.AddItem "   IGMP Grp Addr = " & strIP
    End If

End Sub

'//
'// Function: DecodeUDPHeader
'//
'// Description:
'//    This function takes a buffer which points to a UDP
'//    header and prints it out in a readable form.
'//
Sub DecodeUDPHeader(ptr() As Byte, ByVal iphdrlen As Long)

    Dim shortval As Long, udp_src_port As Long, udp_dest_port As Long, udp_len As Long, udp_chksum As Long
    Dim addr As sockaddr
    Dim hdr As Byte
    Dim pos As Long
    Dim strUDPHeader As String
    
    pos = iphdrlen
    hdr = ptr(pos)
    
    CopyMemory shortval, ptr(pos), 2
    udp_src_port = ntohs(shortval) And &HFFFF&
    pos = pos + 2

    CopyMemory shortval, ptr(pos), 2
    udp_dest_port = ntohs(shortval) And &HFFFF&
    pos = pos + 2

    CopyMemory shortval, ptr(pos), 2
    udp_len = ntohs(shortval) And &HFFFF&
    pos = pos + 2
    
    CopyMemory shortval, ptr(pos), 2
    udp_chksum = ntohs(shortval) And &HFFFF&
    
    frmrcvall.List2.AddItem "   UDP HEADER"
    frmrcvall.List2.AddItem "   Source Port: " & udp_src_port & "       | Dest Port: " & udp_dest_port
    frmrcvall.List2.AddItem "       UDP Len: " & udp_len & "       |    ChkSum: " & udp_chksum
End Sub


'//
'// Function: DecodeTCPHeader
'//
'// Description:
'//    This function takes a buffer pointing to a TCP header
'//    and prints it out in a readable form.
'//
Sub DecodeTCPHeader(ptr() As Byte, ByVal iphdrlen As Long)
    Dim shortval As Long, longval As Long
    Dim hdr As Byte
    Dim pos As Long
    Dim strTCPHeader As String
    
    pos = iphdrlen
    hdr = ptr(pos)
    
    frmrcvall.List2.AddItem "   TCP HEADER"
    
    CopyMemory shortval, ptr(pos), 2
    shortval = ntohs(shortval) And &HFFFF&
    frmrcvall.List2.AddItem "   Src Port   : " & shortval
    pos = pos + 2
    
    CopyMemory shortval, ptr(pos), 2
    shortval = ntohs(shortval) And &HFFFF&
    frmrcvall.List2.AddItem "   Dest Port  : " & shortval
    pos = pos + 2

    CopyMemory longval, ptr(pos), 4
    longval = ntohl(longval)
    frmrcvall.List2.AddItem "   Seq Num    : " & longval
    pos = pos + 4
    
    CopyMemory longval, ptr(pos), 4
    longval = ntohl(longval)
    frmrcvall.List2.AddItem "   ACK Num    : " & longval
    pos = pos + 4

    frmrcvall.List2.AddItem "   Header Len : " & HI_WORD(ptr(pos)) & " (bytes " & HI_WORD(ptr(pos)) * 4 & ")"

    CopyMemory shortval, ptr(pos), 2
    shortval = ntohs(shortval) And &H3F&
    strTCPHeader = "   Flags      : "
    If shortval And &H20& Then strTCPHeader = strTCPHeader & "URG "
    If shortval And &H10& Then strTCPHeader = strTCPHeader & "ACK "
    If shortval And &H8& Then strTCPHeader = strTCPHeader & "PSH "
    If shortval And &H4& Then strTCPHeader = strTCPHeader & "RST "
    If shortval And &H2& Then strTCPHeader = strTCPHeader & "SYN "
    If shortval And &H1& Then strTCPHeader = strTCPHeader & "FIN "
    frmrcvall.List2.AddItem strTCPHeader
    pos = pos + 2
    
    CopyMemory shortval, ptr(pos), 2
    shortval = ntohs(shortval) And &HFFFF&
    frmrcvall.List2.AddItem "   Window size: " & shortval
    pos = pos + 2
    
    CopyMemory shortval, ptr(pos), 2
    shortval = ntohs(shortval) And &HFFFF&
    frmrcvall.List2.AddItem "   TCP Chksum : 0x" & Hex(shortval)
    pos = pos + 2

    CopyMemory shortval, ptr(pos), 2
    shortval = ntohs(shortval) And &HFFFF&
    frmrcvall.List2.AddItem "   Urgent ptr : " & shortval

End Sub

'//
'// Function: DecodeIPHeader
'//
'// Description:
'//    This function takes a pointer to an IP header and prints
'//    it out in a readable form.
'//
Sub DecodeIPHeader(buf() As Byte, srcip As Long, ByVal srcport As Long, _
          destip As Long, ByVal destport As Long)
    
    Dim shortval As Long
    Dim hdr As Byte
    Dim nexthdr As Long
    Dim pos As Long
    Dim srcaddr As sockaddr, destaddr As sockaddr
    Dim ip_version As Long, ip_hdr_len As Long, ip_tos As Long, ip_total_len As Long
    Dim ip_id As Long, ip_flags As Long, ip_ttl As Long, ip_frag_offset As Long
    Dim ip_proto As Long, ip_hdr_chksum As Long, ip_src_port As Long, ip_dest_port As Long
    
    Dim ip_src As Long, ip_dest As Long
    Dim bPrint As Boolean
    bPrint = True
    
    pos = 0
    hdr = buf(pos)
    
    ip_version = HI_WORD(hdr)
    ip_hdr_len = LO_WORD(hdr) * 4
    
    nexthdr = pos + ip_hdr_len
    pos = pos + 1
    
    ip_tos = buf(pos)
    pos = pos + 1
    
    CopyMemory shortval, buf(pos), 2
    ip_total_len = ntohs(shortval) And &HFFFF&
    pos = pos + 2
    
    CopyMemory shortval, buf(pos), 2
    ip_id = ntohs(shortval) And &HFFFF&
    pos = pos + 2
   
    hdr = buf(pos)
    ip_flags = hdr \ 32
    
    CopyMemory shortval, buf(pos), 2
    ip_frag_offset = ntohs(shortval) And &H1FFF&
    pos = pos + 2
    
    ip_ttl = buf(pos)
    pos = pos + 1

    ip_proto = buf(pos)
    pos = pos + 1
    
    CopyMemory shortval, buf(pos), 2
    ip_hdr_chksum = ntohs(shortval) And &HFFFF&
    pos = pos + 2
    
    CopyMemory srcaddr.sin_addr, buf(pos), 4
    ip_src = srcaddr.sin_addr
    pos = pos + 4
    
    CopyMemory destaddr.sin_addr, buf(pos), 4
    ip_dest = destaddr.sin_addr
    pos = pos + 4
    
    '//
    '// If packet is UDP, TCP, or IGMP read ahead and
    '//  get the port values.
    '//
    If ((ip_proto = 2) Or (ip_proto = 6) Or (ip_proto = 17)) And (bFilter = True) Then
        
        CopyMemory ip_src_port, buf(nexthdr), 2
        ip_src_port = ntohs(ip_src_port) And &HFFFF&
        nexthdr = nexthdr + 2
        CopyMemory ip_dest_port, buf(nexthdr), 2
        ip_dest_port = ntohs(ip_dest_port) And &HFFFF&

        If (srcip = ip_src) Or (srcport = ip_src_port) Or (destip = ip_dest) Or (destport = ip_dest_port) Then
            bPrint = True
        Else
            bPrint = False
        End If
        
    ElseIf bFilter = True Then
        bPrint = False
    End If
    
    '// Print IP Hdr
    '//
    If bPrint Then
        frmrcvall.List2.AddItem "IP HEADER"
        frmrcvall.List2.AddItem "   IP Version: " & ip_version & "  |  IP Header Len: " & ip_hdr_len & " bytes   |   IP TOS: 0x" & Hex(HI_WORD(CByte(ip_tos))) & Hex(LO_WORD(CByte(ip_tos)))
        frmrcvall.List2.AddItem "   IP Total Len: " & ip_total_len & " bytes | Identification: 0x" & Hex(ip_id) & " | IP Flags: 0x" & Hex(ip_flags)
        
        frmrcvall.List2.AddItem "   Frag Offset: 0x" & Hex(ip_frag_offset) & "  |            TTL: " & ip_ttl & "  | Protocol: " & szProto(ip_proto)
        frmrcvall.List2.AddItem "   Hdr Checksum: 0x" & Hex(ip_hdr_chksum)
        Dim strIP As String
        strIP = String(256, 0)
        lstrcpy1 strIP, inet_ntoa(srcaddr.sin_addr)
        strIP = Trim(strIP)
        frmrcvall.List2.AddItem "   Src Addr:  " & strIP
        strIP = String(256, 0)
        lstrcpy1 strIP, inet_ntoa(destaddr.sin_addr)
        strIP = Trim(strIP)
        frmrcvall.List2.AddItem "   Dest Addr: " & strIP
    Else
        Exit Sub
    End If
    
    
    Select Case ip_proto
        Case 2:        '// IGMP
            DecodeIGMPHeader buf, ip_hdr_len
        Case 6:        '// TCP
            DecodeTCPHeader buf, ip_hdr_len
        Case 17:       '// UDP
            DecodeUDPHeader buf, ip_hdr_len
        Case Else:
            frmrcvall.List2.AddItem "   No decoder installed for protocol"
    End Select
    
    frmrcvall.List2.AddItem ""
End Sub


