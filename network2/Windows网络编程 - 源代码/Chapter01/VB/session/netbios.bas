Attribute VB_Name = "Module1"
Option Explicit

Global Const NCBNAMSZ = 16
Global Const HEAP_ZERO_MEMORY = &H8
Global Const HEAP_GENERATE_EXCEPTIONS = &H4

Global Const INFINITE = &HFFFFFFFF
Global Const WAIT_FAILED = &HFFFFFFFF


Global Const MAX_SESSIONS = 254
Global Const MAX_NAMES = 254
Global Const MAX_DATAGRAM_SIZE = 512
Global Const MAX_LANA = 254

Global Const ASYNCH = &H80
Global Const NRC_GOODRET = &H0        ' good return
                                      ' also returned when ASYNCH request accepted
Global Const NRC_BUFLEN = &H1         ' illegal buffer length
Global Const NRC_ILLCMD = &H3         ' illegal command
Global Const NRC_CMDTMO = &H5         ' command timed out
Global Const NRC_INCOMP = &H6         ' message incomplete, issue another command
Global Const NRC_BADDR = &H7          ' illegal buffer address
Global Const NRC_SNUMOUT = &H8        ' session number out of range
Global Const NRC_NORES = &H9          ' no resource available
Global Const NRC_SCLOSED = &HA        ' session closed
Global Const NRC_CMDCAN = &HB         ' command cancelled
Global Const NRC_DUPNAME = &HD        ' duplicate name
Global Const NRC_NAMTFUL = &HE        ' name table full
Global Const NRC_ACTSES = &HF         ' no deletions, name has active sessions
Global Const NRC_LOCTFUL = &H11       ' local session table full
Global Const NRC_REMTFUL = &H12       ' remote session table full
Global Const NRC_ILLNN = &H13         ' illegal name number
Global Const NRC_NOCALL = &H14        ' no callname
Global Const NRC_NOWILD = &H15        ' cannot put * in NCB_NAME
Global Const NRC_INUSE = &H16         ' name in use on remote adapter
Global Const NRC_NAMERR = &H17        ' name deleted
Global Const NRC_SABORT = &H18        ' session ended abnormally
Global Const NRC_NAMCONF = &H19       ' name conflict detected
Global Const NRC_IFBUSY = &H21        ' interface busy, IRET before retrying
Global Const NRC_TOOMANY = &H22       ' too many commands outstanding, retry later
Global Const NRC_BRIDGE = &H23        ' ncb_lana_num field invalid
Global Const NRC_CANOCCR = &H24       ' command completed while cancel occurring
Global Const NRC_CANCEL = &H26        ' command not valid to cancel
Global Const NRC_DUPENV = &H30        ' name defined by anther local process
Global Const NRC_ENVNOTDEF = &H34     ' environment undefined. RESET required
Global Const NRC_OSRESNOTAV = &H35    ' required OS resources exhausted
Global Const NRC_MAXAPPS = &H36       ' max number of applications exceeded
Global Const NRC_NOSAPS = &H37        ' no saps available for netbios
Global Const NRC_NORESOURCES = &H38   ' requested resources are not available
Global Const NRC_INVADDRESS = &H39    ' invalid ncb address or length > segment
Global Const NRC_INVDDID = &H3B       ' invalid NCB DDID
Global Const NRC_LOCKFAIL = &H3C      ' lock of user area failed
Global Const NRC_OPENERR = &H3F       ' NETBIOS not loaded
Global Const NRC_SYSTEM = &H40        ' system error

Global Const NRC_PENDING = &HFF       ' asynchronous command is not yet finished

Global Const NCBCALL = &H10                   ' NCB CALL
Global Const NCBLISTEN = &H11                 ' NCB LISTEN
Global Const NCBHANGUP = &H12                 ' NCB HANG UP
Global Const NCBSEND = &H14                   ' NCB SEND
Global Const ncbRecv = &H15                   ' NCB RECEIVE
Global Const NCBRECVANY = &H16                ' NCB RECEIVE ANY
Global Const NCBCHAINSEND = &H17              ' NCB CHAIN SEND
Global Const NCBDGSEND = &H20                 ' NCB SEND DATAGRAM
Global Const NCBDGRECV = &H21                 ' NCB RECEIVE DATAGRAM
Global Const NCBDGSENDBC = &H22               ' NCB SEND BROADCAST DATAGRAM
Global Const NCBDGRECVBC = &H23               ' NCB RECEIVE BROADCAST DATAGRAM
Global Const NCBADDNAME = &H30                ' NCB ADD NAME
Global Const NCBDELNAME = &H31                ' NCB DELETE NAME
Global Const NCBRESET = &H32                  ' NCB RESET
Global Const NCBASTAT = &H33                  ' NCB ADAPTER STATUS
Global Const NCBSSTAT = &H34                  ' NCB SESSION STATUS
Global Const NCBCANCEL = &H35                 ' NCB CANCEL
Global Const NCBADDGRNAME = &H36              ' NCB ADD GROUP NAME
Global Const NCBENUM = &H37                   ' NCB ENUMERATE LANA NUMBERS
Global Const NCBUNLINK = &H70                 ' NCB UNLINK
Global Const NCBSENDNA = &H71                 ' NCB SEND NO ACK
Global Const NCBCHAINSENDNA = &H72            ' NCB CHAIN SEND NO ACK
Global Const NCBLANSTALERT = &H73             ' NCB LAN STATUS ALERT
Global Const NCBACTION = &H77                 ' NCB ACTION
Global Const NCBFINDNAME = &H78               ' NCB FIND NAME
Global Const NCBTRACE = &H79                  ' NCB TRACE

Type LANA_ENUM
    length As Byte
    lana(MAX_LANA) As Byte
End Type


Type NCB
    ncb_command As Byte 'Integer
    ncb_retcode As Byte 'Integer
    ncb_lsn As Byte 'Integer
    ncb_num As Byte ' Integer
    ncb_buffer As Long 'String
    ncb_length As Integer
    ncb_callname(NCBNAMSZ - 1) As Byte
    ncb_name(NCBNAMSZ - 1) As Byte
    ncb_rto As Byte 'Integer
    ncb_sto As Byte ' Integer
    ncb_post As Long
    ncb_lana_num As Byte 'Integer
    ncb_cmd_cplt As Byte  'Integer
    ncb_reserve(9) As Byte ' Reserved, must be 0
    ncb_event As Long
End Type

Type UserBuffer
    userByteArray(511) As Byte
 End Type
 
Type ADAPTER_STATUS
    adapter_address(5) As Byte 'As String * 6
    rev_major As Byte 'Integer
    reserved0 As Byte 'Integer
    adapter_type As Byte 'Integer
    rev_minor As Byte 'Integer
    duration As Integer
    frmr_recv As Integer
    frmr_xmit As Integer
    iframe_recv_err As Integer
    xmit_aborts As Integer
    xmit_success As Long
    recv_success As Long
    iframe_xmit_err As Integer
    recv_buff_unavail As Integer
    t1_timeouts As Integer
    ti_timeouts As Integer
    Reserved1 As Long
    free_ncbs As Integer
    max_cfg_ncbs As Integer
    max_ncbs As Integer
    xmit_buf_unavail As Integer
    max_dgram_size As Integer
    pending_sess As Integer
    max_cfg_sess As Integer
    max_sess As Integer
    max_sess_pkt_size As Integer
    name_count As Integer
End Type

Type NAME_BUFFER
    name  As String * NCBNAMSZ
    name_num As Integer
    name_flags As Integer
End Type
   
Type ASTAT
    adapt As ADAPTER_STATUS
    NameBuff(30) As NAME_BUFFER
End Type

Public Declare Sub ZeroMemory Lib "kernel32" Alias "RtlZeroMemory" (dest As Any, ByVal numBytes As Long)
Public Declare Function Netbios Lib "netapi32.dll" _
           (pncb As NCB) As Byte
Public Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" ( _
           hpvDest As Any, ByVal hpvSource As Long, ByVal cbCopy As Long)
Public Declare Sub CopyMemory2 Lib "kernel32" Alias "RtlMoveMemory" ( _
           ByVal hpvDest As Long, hpvSource As Any, ByVal cbCopy As Long)
Public Declare Function GetProcessHeap Lib "kernel32" () As Long
Public Declare Function HeapAlloc Lib "kernel32" _
           (ByVal hHeap As Long, ByVal dwFlags As Long, _
           ByVal dwBytes As Long) As Long
Public Declare Function HeapFree Lib "kernel32" (ByVal hHeap As Long, _
           ByVal dwFlags As Long, lpMem As Any) As Long
Public Declare Function lstrcpyn Lib "kernel32" Alias "lstrcpynA" (ByVal lpString1 As Long, ByVal lpString2 As String, ByVal iMaxLength As Long) As Long
Public Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
Public Declare Function CreateEvent Lib "kernel32" Alias "CreateEventA" (ByVal lpEventAttributes As Long, ByVal bManualReset As Long, ByVal bInitialState As Long, ByVal lpName As String) As Long
Public Declare Function ResetEvent Lib "kernel32" (ByVal hEvent As Long) As Long
Public Declare Function WaitForMultipleObjects Lib "kernel32" (ByVal nCount As Long, lpHandles As Long, ByVal bWaitAll As Long, ByVal dwMilliseconds As Long) As Long
Public Declare Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long
Public Declare Function lstrcpy Lib "kernel32" Alias "lstrcpyA" (ByVal lpString1 As String, ByVal lpString2 As Long) As Long
Public Declare Function lstrcpy2 Lib "kernel32" Alias "lstrcpyA" (ByVal lpString1 As Long, ByVal lpString2 As String) As Long
Public Declare Function lstrcpy3 Lib "kernel32" Alias "lstrcpyA" (ByVal lpString1 As String, lpString2 As Any) As Long


Public Const GMEM_FIXED = &H0
Public Declare Function GlobalAlloc Lib "kernel32" (ByVal wFlags As Long, ByVal dwBytes As Long) As Long
Public Declare Function GlobalFree Lib "kernel32" (ByVal hMem As Long) As Long

'
' Function: LanaEnum
'
' Description:
'    Enumerate all LANA numbers on the machine. Pass a pointer
'    to a valid LANA_ENUM struct into the function and it will
'    be filled in.
'
Function LanaEnum(lenum As LANA_ENUM) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBENUM
    localNcb.ncb_buffer = VarPtr(lenum)
    localNcb.ncb_length = Len(lenum)
    
    nRet = Netbios(localNcb)
    
    If nRet <> NRC_GOODRET Then
        Debug.Print "ERROR: Netbios: NCBENUM: " & localNcb.ncb_retcode
        LanaEnum = localNcb.ncb_retcode
    Else
        LanaEnum = NRC_GOODRET
    End If
End Function

'
' Function: ResetAll
'
' Description:
'    Reset each LANA listed in the LANA_ENUM structure.  Also set
'    the NetBIOS environment (max sessions, max name table size,
'    and use the first NetBIOS name).
'
Function ResetAll(lenum As LANA_ENUM, ByVal ucMaxSession As Byte, ByVal ucMaxName As Byte, ByVal bFirstName As Boolean) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBRESET
    localNcb.ncb_callname(0) = ucMaxSession
    localNcb.ncb_callname(2) = ucMaxName
    localNcb.ncb_callname(3) = CByte(bFirstName)
       
    Dim i As Long
    For i = 0 To lenum.length - 1
        localNcb.ncb_lana_num = lenum.lana(i)
        nRet = Netbios(localNcb)
        If nRet <> NRC_GOODRET Then
            Debug.Print "ERROR: Netbios: NCBRESET: " & localNcb.ncb_retcode
            ResetAll = localNcb.ncb_retcode
            Exit Function
        End If
    Next
    ResetAll = NRC_GOODRET
End Function

'
' Function: AddName
'
' Description:
'    Add the given name to the given LANA number. Return the name
'    number for the registered name. This name number is essential
'    for datagram operations.
'
Function AddName(ByVal lana As Long, ByVal name As String, ByRef num As Long) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBADDNAME
    localNcb.ncb_lana_num = lana
    Dim i As Long, j As Long
    
    For i = 0 To NCBNAMSZ - 1
        localNcb.ncb_name(i) = Asc(" ")
    Next
    
    If Len(name) < NCBNAMSZ - 1 Then
        j = Len(name)
    Else
        j = NCBNAMSZ - 1
    End If
    
    For i = 0 To j - 1
        localNcb.ncb_name(i) = Asc(Mid(name, i + 1, 1))
    Next
    
    nRet = Netbios(localNcb)
       
    If nRet <> NRC_GOODRET Then
        MsgBox "ERROR: Netbios: NCBADDNAME: " & localNcb.ncb_retcode
        AddName = localNcb.ncb_retcode
    Else
        num = localNcb.ncb_num
        AddName = NRC_GOODRET
    End If

End Function

'
' Function: DelName
'
' Description:
'    Delete the given NetBIOS name from the name table associated
'    with the LANA number.
'
Function DelName(ByVal lana As Long, ByVal name As String) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBDELNAME
    localNcb.ncb_lana_num = lana
    
    Dim i As Long, j As Long
    For i = 0 To NCBNAMSZ - 1
        localNcb.ncb_name(i) = Asc(" ")
    Next
    
    If Len(name) < NCBNAMSZ - 1 Then
        j = Len(name)
    Else
        j = NCBNAMSZ - 1
    End If
    
    For i = 0 To j - 1
        localNcb.ncb_name(i) = Asc(Mid(name, i + 1, 1))
    Next
       
    nRet = Netbios(localNcb)
       
    If nRet <> NRC_GOODRET Then
        MsgBox "ERROR: Netbios: NCBDELNAME: " & localNcb.ncb_retcode
        DelName = localNcb.ncb_retcode
    Else
        DelName = NRC_GOODRET
    End If

End Function

'
' Function: AddGroupName
'
' Description:
'    Add the given NetBIOS group name to the given LANA
'    number. Return the name number for the added name.
'
Function AddGroupName(ByVal lana As Long, ByVal name As String, ByRef num As Long) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBADDGRNAME
    localNcb.ncb_lana_num = lana
    
    Dim i As Long, j As Long
    For i = 0 To NCBNAMSZ - 1
        localNcb.ncb_name(i) = Asc(" ")
    Next
    If Len(name) < NCBNAMSZ - 1 Then
        j = Len(name)
    Else
        j = NCBNAMSZ - 1
    End If
    
    For i = 0 To j - 1
        localNcb.ncb_name(i) = Asc(Mid(name, i + 1, 1))
    Next
       
    nRet = Netbios(localNcb)
       
    If nRet <> NRC_GOODRET Then
        MsgBox "ERROR: Netbios: AddGroupName: " & localNcb.ncb_retcode
        AddGroupName = localNcb.ncb_retcode
    Else
        num = localNcb.ncb_num
        AddGroupName = NRC_GOODRET
    End If

End Function

'
' Function: Send
'
' Description:
'    Send len bytes from the data buffer on the given session (lsn)
'    and lana number. This function performs a synchronous send.
'
Function Send(ByVal lana As Long, ByVal lsn As Long, ByVal dataPtr As Long, ByVal dlen As Long) As Long
    Dim localNcb As NCB
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBSEND
    localNcb.ncb_buffer = dataPtr
    localNcb.ncb_length = dlen
    localNcb.ncb_lana_num = lana
    localNcb.ncb_lsn = lsn
    Send = Netbios(localNcb)
End Function

'
' Function: Recv
'
' Description:
'    Receive up to len bytes into the data buffer on the given session
'    (lsn) and lana number.
'
Function Recv(ByVal lana As Long, ByVal lsn As Long, ByVal dataPtr As Long, ByRef dlen As Long) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = ncbRecv
    localNcb.ncb_buffer = dataPtr
    localNcb.ncb_length = dlen
    localNcb.ncb_lana_num = lana
    localNcb.ncb_lsn = lsn
    nRet = Netbios(localNcb)
    If nRet <> NRC_GOODRET Then
        dlen = -1
        Recv = localNcb.ncb_retcode
    Else
        dlen = localNcb.ncb_length
        Recv = NRC_GOODRET
    End If
End Function

'
' Function: Hangup
'
' Description:
'    Disconnect the given session on the given lana number.
'
Function Hangup(ByVal lana As Long, ByVal lsn As Long) As Long
    Dim localNcb As NCB
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBHANGUP
    localNcb.ncb_lsn = lsn
    localNcb.ncb_lana_num = lana
    Hangup = Netbios(localNcb)

End Function

'
' Function: Cancel
'
' Description:
'    Cancel the given asynchronous command denoted in the NCB
'    structure parameter.
'
Function Cancel(pncb As NCB) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBCANCEL
    localNcb.ncb_buffer = VarPtr(pncb)
    localNcb.ncb_lana_num = pncb.ncb_lana_num
    
    nRet = Netbios(localNcb)
    
    If nRet <> NRC_GOODRET Then
        MsgBox "ERROR: Netbios: NCBCANCEL: " & localNcb.ncb_retcode
        Cancel = localNcb.ncb_retcode
    Else
        Cancel = NRC_GOODRET
    End If
End Function

'
' Function: Connect
'
' Description:
'    Post an asyncrhonous connect on the given LANA number to server.
'    The NCB structure passed in already has the ncb_event field set
'    to a valid Windows event handle. Just fill in the blanks and make
'    the call.
'
Function Connect(pncb As NCB, ByVal lana As Long, ByVal server As String, ByVal client As String) As Long
    Dim nRet As Long
    
    pncb.ncb_command = NCBCALL Or ASYNCH
    pncb.ncb_lana_num = lana
    
    
    Dim i As Long, j As Long
    For i = 0 To NCBNAMSZ - 1
        pncb.ncb_callname(i) = Asc(" ")
        pncb.ncb_name(i) = Asc(" ")
    Next
    If Len(client) < NCBNAMSZ - 1 Then
        j = Len(client)
    Else
        j = NCBNAMSZ - 1
    End If
    For i = 0 To j - 1
        pncb.ncb_name(i) = Asc(Mid(client, i + 1, 1))
    Next
    If Len(server) < NCBNAMSZ - 1 Then
        j = Len(server)
    Else
        j = NCBNAMSZ - 1
    End If
    For i = 0 To j - 1
        pncb.ncb_callname(i) = Asc(Mid(server, i + 1, 1))
    Next
    
    nRet = Netbios(pncb)
    If nRet <> NRC_GOODRET Then
        Debug.Print "Netbios: NCBCONNECT failed: " & pncb.ncb_retcode
        Connect = pncb.ncb_retcode
    Else
        Connect = NRC_GOODRET
    End If

End Function

'
' Function: Listen
'
' Description:
'    Post an asynchronous listen. The NCB structure passed into this function
'    should either have a callback or an event set within the structure.
'
Function Listen(pncb As NCB, ByVal lana As Long, ByVal name As String) As Long
    Dim nRet As Long
    
    pncb.ncb_command = NCBLISTEN Or ASYNCH
    pncb.ncb_lana_num = lana
    
    
    Dim i As Long, j As Long
    For i = 0 To NCBNAMSZ - 1
        pncb.ncb_callname(i) = Asc(" ")
        pncb.ncb_name(i) = Asc(" ")
    Next
    If Len(name) < NCBNAMSZ - 1 Then
        j = Len(name)
    Else
        j = NCBNAMSZ - 1
    End If
    For i = 0 To j - 1
        pncb.ncb_name(i) = Asc(Mid(name, i + 1, 1))
    Next
    
    pncb.ncb_callname(0) = Asc("*")
    
    nRet = Netbios(pncb)
    If nRet <> NRC_GOODRET Then
        Debug.Print "Netbios: NCBLISTEN failed: " & pncb.ncb_retcode
        Listen = pncb.ncb_retcode
    Else
        Listen = NRC_GOODRET
    End If

End Function

'
' Function: DatagramSend
'
' Description:
'    Send a directed datagram to the specified recipient on the
'    specified LANA number from the given name number to the
'    specified recipient. Also specified is the data buffer and
'    the number of bytes to send.
'
Function DatagramSend(ByVal lana As Long, ByVal num As Long, ByVal recipient As String, ByVal buffer As Long, ByRef buflen As Long) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBDGSEND
    localNcb.ncb_lana_num = lana
    localNcb.ncb_num = num
    localNcb.ncb_buffer = buffer
    localNcb.ncb_length = buflen
    
    Dim i As Long, j As Long
    For i = 0 To NCBNAMSZ - 1
        localNcb.ncb_callname(i) = Asc(" ")
    Next
    If Len(recipient) < NCBNAMSZ - 1 Then
        j = Len(recipient)
    Else
        j = NCBNAMSZ - 1
    End If
    For i = 0 To j - 1
        localNcb.ncb_callname(i) = Asc(Mid(recipient, i + 1, 1))
    Next
    
    nRet = Netbios(localNcb)
    If nRet <> NRC_GOODRET Then
        Debug.Print "Netbios: NCBDGSEND failed: " & localNcb.ncb_retcode
        DatagramSend = localNcb.ncb_retcode
    Else
        DatagramSend = NRC_GOODRET
    End If
End Function

'
' Function: DatagramSendBC
'
' Description:
'    Send a broadcast datagram on the specified LANA number from the
'    given name number.  Also specified is the data buffer and number
'    of bytes to send.
'
Function DatagramSendBC(ByVal lana As Long, ByVal num As Long, ByVal buffer As Long, ByRef buflen As Long) As Long
    Dim localNcb As NCB
    Dim nRet As Long
    ZeroMemory localNcb, Len(localNcb)
    localNcb.ncb_command = NCBDGSENDBC
    localNcb.ncb_lana_num = lana
    localNcb.ncb_num = num
    localNcb.ncb_buffer = buffer
    localNcb.ncb_length = buflen
    
    
    nRet = Netbios(localNcb)
    If nRet <> NRC_GOODRET Then
        Debug.Print "Netbios: NCBDGSENDBC failed: " & localNcb.ncb_retcode
        DatagramSendBC = localNcb.ncb_retcode
    Else
        DatagramSendBC = NRC_GOODRET
    End If
End Function

'
' Function: DatagramRecv
'
' Description:
'    Receive a datagram on the given LANA number directed towards the
'    name represented by num.  Data is copied into the supplied buffer.
'    If hEvent is not zero then the receive call is made asynchronously
'    with the supplied event handle. If num is 0xFF then listen for a
'    datagram destined for any NetBIOS name registered by the process.
'
Function DatagramRecv(pncb As NCB, ByVal lana As Long, ByVal num As Long, ByVal buffer As Long, ByRef buflen As Long, ByVal hEvent As Long) As Long
    Dim nRet As Long
    ZeroMemory pncb, Len(pncb)
    
    If hEvent <> 0 Then
        pncb.ncb_command = NCBDGRECV Or ASYNCH
        pncb.ncb_event = hEvent
    Else
        pncb.ncb_command = NCBDGRECV
    End If
    
    pncb.ncb_lana_num = lana
    pncb.ncb_num = num
    pncb.ncb_buffer = buffer
    pncb.ncb_length = buflen
    
    
    nRet = Netbios(pncb)
    If nRet <> NRC_GOODRET Then
        Debug.Print "Netbios: NCBDGRECV failed: " & pncb.ncb_retcode
        DatagramRecv = pncb.ncb_retcode
    Else
        DatagramRecv = NRC_GOODRET
    End If
End Function

'
' Function: DatagramRecvBC
'
' Description:
'    Receive a broadcast datagram on the given LANA number.
'    Data is copied into the supplied buffer.  If hEvent is not zero
'    then the receive call is made asynchronously with the supplied
'    event handle.
'
Function DatagramRecvBC(pncb As NCB, ByVal lana As Long, ByVal num As Long, ByVal buffer As Long, ByRef buflen As Long, ByVal hEvent As Long) As Long
    Dim nRet As Long
    ZeroMemory pncb, Len(pncb)
    
    If hEvent <> 0 Then
        pncb.ncb_command = NCBDGRECVBC Or ASYNCH
        pncb.ncb_event = hEvent
    Else
        pncb.ncb_command = NCBDGRECV
    End If
    
    pncb.ncb_lana_num = lana
    pncb.ncb_num = num
    pncb.ncb_buffer = buffer
    pncb.ncb_length = buflen
    
    
    nRet = Netbios(pncb)
    If nRet <> NRC_GOODRET Then
        Debug.Print "Netbios: NCBDGRECVBC failed: " & pncb.ncb_retcode
        DatagramRecvBC = pncb.ncb_retcode
    Else
        DatagramRecvBC = NRC_GOODRET
    End If
End Function

'
' Function: FormatNetbiosName
'
' Description:
'    Format the given NetBIOS name so it is printable.  Any unprintable
'    characters are replaced by a period.  The outname buffer is
'    the returned string which is assumed to be at least NCBNAMSZ+1
'    characters in length.
'
Function FormatNetbiosName(nbname() As Byte, ByRef outname As String) As Long
    Dim i As Long
    i = 0
    outname = ""
    Do While (nbname(i) <> 0)
        If nbname(i) = Asc(" ") Then
            Exit Do
        End If
        outname = outname & Chr(nbname(i))
        i = i + 1
        If i = 16 Then
            Exit Do
        End If
         
    Loop
    FormatNetbiosName = NRC_GOODRET
    
End Function
