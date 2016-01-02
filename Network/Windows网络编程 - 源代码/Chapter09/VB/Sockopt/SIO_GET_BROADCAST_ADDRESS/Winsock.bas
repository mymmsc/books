Attribute VB_Name = "Winsock"
Option Explicit


'/*
' * Option flags per-socket.
' */
Global Const SO_DEBUG = &H1                   '/* turn on debugging info recording */
Global Const SO_ACCEPTCONN = &H2              '/* socket has had listen() */
Global Const SO_REUSEADDR = &H4               '/* allow local address reuse */
Global Const SO_KEEPALIVE = &H8               '/* keep connections alive */
Global Const SO_DONTROUTE = &H10              '/* just use interface addresses */
Global Const SO_BROADCAST = &H20              '/* permit sending of broadcast msgs */
Global Const SO_USELOOPBACK = &H40            '/* bypass hardware when possible */
Global Const SO_LINGER = &H80                 '/* linger on close if data present */
Global Const SO_OOBINLINE = &H100             '/* leave received OOB data in line */

Global Const SO_DONTLINGER = SO_LINGER Xor &HFFFFFFFF
Global Const SO_EXCLUSIVEADDRUSE = SO_REUSEADDR Xor &HFFFFFFFF '/* disallow local address reuse */

'/*
' * Additional options.
' */
Global Const SO_SNDBUF = &H1001               '/* send buffer size */
Global Const SO_RCVBUF = &H1002               '/* receive buffer size */
Global Const SO_SNDLOWAT = &H1003             '/* send low-water mark */
Global Const SO_RCVLOWAT = &H1004             '/* receive low-water mark */
Global Const SO_SNDTIMEO = &H1005             '/* send timeout */
Global Const SO_RCVTIMEO = &H1006             '/* receive timeout */
Global Const SO_ERROR = &H1007                '/* get error status and clear */
Global Const SO_TYPE = &H1008                 '/* get socket type */

'/*
' * WinSock 2 extension -- new options
' */
Global Const SO_GROUP_ID = &H2001           '/* ID of a socket group */
Global Const SO_GROUP_PRIORITY = &H2002     '/* the relative priority within a group*/
Global Const SO_MAX_MSG_SIZE = &H2003       '/* maximum message size */
Global Const SO_PROTOCOL_INFOA = &H2004     '/* WSAPROTOCOL_INFOA structure */
Global Const SO_PROTOCOL_INFOW = &H2005     '/* WSAPROTOCOL_INFOW structure */
Global Const PVD_CONFIG = &H3001                '/* configuration info for service provider */

'/*
' * TCP options.
' */
Global Const TCP_NODELAY = &H1
Global Const IP_OPTIONS = 1        '/* set/get IP options */
Global Const IP_HDRINCL = 2        '/* header is included with data */
Global Const IP_TOS = 3            '/* IP type of service and preced*/
Global Const IP_TTL = 4            '/* IP time to live */
Global Const IP_MULTICAST_IF = 9       '/* set/get IP multicast i/f  */
Global Const IP_MULTICAST_TTL = 10         '/* set/get IP multicast ttl */
Global Const IP_MULTICAST_LOOP = 11        '/*set/get IP multicast loopback */
Global Const IP_ADD_MEMBERSHIP = 12        '/* add an IP group membership */
Global Const IP_DROP_MEMBERSHIP = 13      '/* drop an IP group membership */
Global Const IP_DONTFRAGMENT = 14    '/* don't fragment IP datagrams */

Type ip_mreq
    imr_multiaddr As Long   '/* IP multicast address of group */
    imr_interface As Long   '/* local IP address of interface */
End Type

Global Const AF_UNSPEC = 0                    '/* unspecified */
'/*
' * Although  AF_UNSPEC  is  defined for backwards compatibility, using
' * AF_UNSPEC for the "af" parameter when creating a socket is STRONGLY
' * DISCOURAGED.    The  interpretation  of  the  "protocol"  parameter
' * depends  on the actual address family chosen.  As environments grow
' * to  include  more  and  more  address families that use overlapping
' * protocol  values  there  is  more  and  more  chance of choosing an
' * undesired address family when AF_UNSPEC is used.
' */
Global Const AF_UNIX = 1                      '/* local to host (pipes, portals) */
Global Const AF_INET = 2                      '/* internetwork: UDP, TCP, etc. */
Global Const AF_IMPLINK = 3                   '/* arpanet imp addresses */
Global Const AF_PUP = 4                       '/* pup protocols: e.g. BSP */
Global Const AF_CHAOS = 5                     '/* mit CHAOS protocols */
Global Const AF_NS = 6                        '/* XEROX NS protocols */
Global Const AF_IPX = AF_NS                   '/* IPX protocols: IPX, SPX, etc. */
Global Const AF_ISO = 7                       '/* ISO protocols */
Global Const AF_OSI = AF_ISO                  '/* OSI is ISO */
Global Const AF_ECMA = 8                      '/* european computer manufacturers */
Global Const AF_DATAKIT = 9                   '/* datakit protocols */
Global Const AF_CCITT = 10                    '/* CCITT protocols, X.25 etc */
Global Const AF_SNA = 11                      '/* IBM SNA */
Global Const AF_DECnet = 12                   '/* DECnet */
Global Const AF_DLI = 13                      '/* Direct data link interface */
Global Const AF_LAT = 14                      '/* LAT */
Global Const AF_HYLINK = 15                   '/* NSC Hyperchannel */
Global Const AF_APPLETALK = 16                '/* AppleTalk */
Global Const AF_NETBIOS = 17                  '/* NetBios-style addresses */
Global Const AF_VOICEVIEW = 18                '/* VoiceView */
Global Const AF_FIREFOX = 19                  '/* Protocols from Firefox */
Global Const AF_UNKNOWN1 = 20                 '/* Somebody is using this! */
Global Const AF_BAN = 21                      '/* Banyan */
Global Const AF_ATM = 22                      '/* Native ATM Services */
Global Const AF_INET6 = 23                    '/* Internetwork Version 6 */
Global Const AF_CLUSTER = 24                  '/* Microsoft Wolfpack */
Global Const AF_12844 = 25                    '/* IEEE 1284.4 WG AF */
Global Const AF_IRDA = 26                     '/* IrDA */


Global Const AF_MAX = 27

Global Const NSPROTO_IPX = 1000
Global Const NSPROTO_SPX = 1256
Global Const NSPROTO_SPXII = 1257

Global Const ATPROTO_BASE = (1000 * AF_APPLETALK)
Global Const SOL_APPLETALK = (ATPROTO_BASE)

Global Const DDPPROTO_RTMP = (ATPROTO_BASE + 1)
Global Const DDPPROTO_NBP = (ATPROTO_BASE + 2)
Global Const DDPPROTO_ATP = (ATPROTO_BASE + 3)
Global Const DDPPROTO_AEP = (ATPROTO_BASE + 4)
Global Const DDPPROTO_RTMPRQ = (ATPROTO_BASE + 5)
Global Const DDPPROTO_ZIP = (ATPROTO_BASE + 6)
Global Const DDPPROTO_ADSP = (ATPROTO_BASE + 7)

Global Const DDPPROTO_MAX = (ATPROTO_BASE + 255)

'//
'//  Define the higher layer appletalk protocol types
'//

Global Const ATPROTO_ADSP = (DDPPROTO_MAX + 1)
Global Const ATPROTO_ATP = (DDPPROTO_MAX + 2)
Global Const ATPROTO_ASP = (DDPPROTO_MAX + 3)
Global Const ATPROTO_PAP = (DDPPROTO_MAX + 4)

Global Const ATMPROTO_AALUSER = 0             '/* User-defined AAL */
Global Const ATMPROTO_AAL1 = 1                '/* AAL 1 */
Global Const ATMPROTO_AAL2 = 2                '/* AAL 2 */
Global Const ATMPROTO_AAL34 = 3               '/* AAL 3/4 */
Global Const ATMPROTO_AAL5 = 5                '/* AAL 5 */

'/*
' * Protocols
' */
Global Const IPPROTO_IP = 0                            '/* dummy for IP */
Global Const IPPROTO_ICMP = 1                          '/* control message protocol */
Global Const IPPROTO_IGMP = 2                          '/* internet group management protocol */
Global Const IPPROTO_GGP = 3                           '/* gateway^2 (deprecated) */
Global Const IPPROTO_TCP = 6                           '/* tcp */
Global Const IPPROTO_PUP = 12                          '/* pup */
Global Const IPPROTO_UDP = 17                          '/* user datagram protocol */
Global Const IPPROTO_IDP = 22                          '/* xns idp */
Global Const IPPROTO_ND = 77                           '/* UNOFFICIAL net disk proto */

Global Const IPPROTO_RAW = 255                         '/* raw IP packet */
Global Const IPPROTO_MAX = 256

'/*
' * Port/socket numbers: network standard functions
' */
Global Const IPPORT_ECHO = 7
Global Const IPPORT_DISCARD = 9
Global Const IPPORT_SYSTAT = 11
Global Const IPPORT_DAYTIME = 13
Global Const IPPORT_NETSTAT = 15
Global Const IPPORT_FTP = 21
Global Const IPPORT_TELNET = 23
Global Const IPPORT_SMTP = 25
Global Const IPPORT_TIMESERVER = 37
Global Const IPPORT_NAMESERVER = 42
Global Const IPPORT_WHOIS = 43
Global Const IPPORT_MTP = 57

'/*
' * Port/socket numbers: host specific functions
' */
Global Const IPPORT_TFTP = 69
Global Const IPPORT_RJE = 77
Global Const IPPORT_FINGER = 79
Global Const IPPORT_TTYLINK = 87
Global Const IPPORT_SUPDUP = 95

'/*
' * UNIX TCP sockets
' */
Global Const IPPORT_EXECSERVER = 512
Global Const IPPORT_LOGINSERVER = 513
Global Const IPPORT_CMDSERVER = 514
Global Const IPPORT_EFSSERVER = 520

'/*
' * UNIX UDP sockets
' */
Global Const IPPORT_BIFFUDP = 512
Global Const IPPORT_WHOSERVER = 513
Global Const IPPORT_ROUTESERVER = 520
                                   '     /* 520+1 also used */

'/*
' * Ports < IPPORT_RESERVED are reserved for
' * privileged processes (e.g. root).
' */
Global Const IPPORT_RESERVED = 1024

'/* Flag bit definitions for dwProviderFlags */
Global Const PFL_MULTIPLE_PROTO_ENTRIES = &H1
Global Const PFL_RECOMMENDED_PROTO_ENTRY = &H2
Global Const PFL_HIDDEN = &H4
Global Const PFL_MATCHES_PROTOCOL_ZERO = &H8

Global Const WSABASEERR = 10000

'/*
' * Windows Sockets definitions of regular Microsoft C error constants
' */
Global Const WSAEINTR = (WSABASEERR + 4)
Global Const WSAEBADF = (WSABASEERR + 9)
Global Const WSAEACCES = (WSABASEERR + 13)
Global Const WSAEFAULT = (WSABASEERR + 14)
Global Const WSAEINVAL = (WSABASEERR + 22)
Global Const WSAEMFILE = (WSABASEERR + 24)

'/*
' * Windows Sockets definitions of regular Berkeley error constants
' */
Global Const WSAEWOULDBLOCK = (WSABASEERR + 35)
Global Const WSAEINPROGRESS = (WSABASEERR + 36)
Global Const WSAEALREADY = (WSABASEERR + 37)
Global Const WSAENOTSOCK = (WSABASEERR + 38)
Global Const WSAEDESTADDRREQ = (WSABASEERR + 39)
Global Const WSAEMSGSIZE = (WSABASEERR + 40)
Global Const WSAEPROTOTYPE = (WSABASEERR + 41)
Global Const WSAENOPROTOOPT = (WSABASEERR + 42)
Global Const WSAEPROTONOSUPPORT = (WSABASEERR + 43)
Global Const WSAESOCKTNOSUPPORT = (WSABASEERR + 44)
Global Const WSAEOPNOTSUPP = (WSABASEERR + 45)
Global Const WSAEPFNOSUPPORT = (WSABASEERR + 46)
Global Const WSAEAFNOSUPPORT = (WSABASEERR + 47)
Global Const WSAEADDRINUSE = (WSABASEERR + 48)
Global Const WSAEADDRNOTAVAIL = (WSABASEERR + 49)
Global Const WSAENETDOWN = (WSABASEERR + 50)
Global Const WSAENETUNREACH = (WSABASEERR + 51)
Global Const WSAENETRESET = (WSABASEERR + 52)
Global Const WSAECONNABORTED = (WSABASEERR + 53)
Global Const WSAECONNRESET = (WSABASEERR + 54)
Global Const WSAENOBUFS = (WSABASEERR + 55)
Global Const WSAEISCONN = (WSABASEERR + 56)
Global Const WSAENOTCONN = (WSABASEERR + 57)
Global Const WSAESHUTDOWN = (WSABASEERR + 58)
Global Const WSAETOOMANYREFS = (WSABASEERR + 59)
Global Const WSAETIMEDOUT = (WSABASEERR + 60)
Global Const WSAECONNREFUSED = (WSABASEERR + 61)
Global Const WSAELOOP = (WSABASEERR + 62)
Global Const WSAENAMETOOLONG = (WSABASEERR + 63)
Global Const WSAEHOSTDOWN = (WSABASEERR + 64)
Global Const WSAEHOSTUNREACH = (WSABASEERR + 65)
Global Const WSAENOTEMPTY = (WSABASEERR + 66)
Global Const WSAEPROCLIM = (WSABASEERR + 67)
Global Const WSAEUSERS = (WSABASEERR + 68)
Global Const WSAEDQUOT = (WSABASEERR + 69)
Global Const WSAESTALE = (WSABASEERR + 70)
Global Const WSAEREMOTE = (WSABASEERR + 71)

'/*
' * Extended Windows Sockets error constant definitions
' */
Global Const WSASYSNOTREADY = (WSABASEERR + 91)
Global Const WSAVERNOTSUPPORTED = (WSABASEERR + 92)
Global Const WSANOTINITIALISED = (WSABASEERR + 93)
Global Const WSAEDISCON = (WSABASEERR + 101)
Global Const WSAENOMORE = (WSABASEERR + 102)
Global Const WSAECANCELLED = (WSABASEERR + 103)
Global Const WSAEINVALIDPROCTABLE = (WSABASEERR + 104)
Global Const WSAEINVALIDPROVIDER = (WSABASEERR + 105)
Global Const WSAEPROVIDERFAILEDINIT = (WSABASEERR + 106)
Global Const WSASYSCALLFAILURE = (WSABASEERR + 107)
Global Const WSASERVICE_NOT_FOUND = (WSABASEERR + 108)
Global Const WSATYPE_NOT_FOUND = (WSABASEERR + 109)
Global Const WSA_E_NO_MORE = (WSABASEERR + 110)
Global Const WSA_E_CANCELLED = (WSABASEERR + 111)
Global Const WSAEREFUSED = (WSABASEERR + 112)

'/*
' * Error return codes from gethostbyname() and gethostbyaddr()
' * (when using the resolver). Note that these errors are
' * retrieved via WSAGetLastError() and must therefore follow
' * the rules for avoiding clashes with error numbers from
' * specific implementations or language run-time systems.
' * For this reason the codes are based at WSABASEERR+1001.
' * Note also that [WSA]NO_ADDRESS is defined only for
' * compatibility purposes.
' */

'/* Authoritative Answer: Host not found */
Global Const WSAHOST_NOT_FOUND = (WSABASEERR + 1001)

'/* Non-Authoritative: Host not found, or SERVERFAIL */
Global Const WSATRY_AGAIN = (WSABASEERR + 1002)

'/* Non-recoverable errors, FORMERR, REFUSED, NOTIMP */
Global Const WSANO_RECOVERY = (WSABASEERR + 1003)

'/* Valid name, no data record of requested type */
Global Const WSANO_DATA = (WSABASEERR + 1004)

'/*
' * Define QOS related error return codes
' *
' */
Global Const WSA_QOS_RECEIVERS = (WSABASEERR + 1005)
         '/* at least one Reserve has arrived */
Global Const WSA_QOS_SENDERS = (WSABASEERR + 1006)
         '/* at least one Path has arrived */
Global Const WSA_QOS_NO_SENDERS = (WSABASEERR + 1007)
         '/* there are no senders */
Global Const WSA_QOS_NO_RECEIVERS = (WSABASEERR + 1008)
         '/* there are no receivers */
Global Const WSA_QOS_REQUEST_CONFIRMED = (WSABASEERR + 1009)
         '/* Reserve has been confirmed */
Global Const WSA_QOS_ADMISSION_FAILURE = (WSABASEERR + 1010)
         '/* error due to lack of resources */
Global Const WSA_QOS_POLICY_FAILURE = (WSABASEERR + 1011)
         '/* rejected for administrative reasons - bad credentials */
Global Const WSA_QOS_BAD_STYLE = (WSABASEERR + 1012)
         '/* unknown or conflicting style */
Global Const WSA_QOS_BAD_OBJECT = (WSABASEERR + 1013)
         '/* problem with some part of the filterspec or providerspecific
         ' * buffer in general */
Global Const WSA_QOS_TRAFFIC_CTRL_ERROR = (WSABASEERR + 1014)
         '/* problem with some part of the flowspec */
Global Const WSA_QOS_GENERIC_ERROR = (WSABASEERR + 1015)
         '/* general error */

' connection types

Global Const SOCK_STREAM = 1               '/* stream socket */
Global Const SOCK_DGRAM = 2               '/* datagram socket */
Global Const SOCK_RAW = 3               '/* raw-protocol interface */
Global Const SOCK_RDM = 4               '/* reliably-delivered message */
Global Const SOCK_SEQPACKET = 5               '/* sequenced packet stream */

'/* Flag bit definitions for dwServiceFlags1 */
Global Const XP1_CONNECTIONLESS = &H1
Global Const XP1_GUARANTEED_DELIVERY = &H2
Global Const XP1_GUARANTEED_ORDER = &H4
Global Const XP1_MESSAGE_ORIENTED = &H8
Global Const XP1_PSEUDO_STREAM = &H10
Global Const XP1_GRACEFUL_CLOSE = &H20
Global Const XP1_EXPEDITED_DATA = &H40
Global Const XP1_CONNECT_DATA = &H80
Global Const XP1_DISCONNECT_DATA = &H100
Global Const XP1_SUPPORT_BROADCAST = &H200
Global Const XP1_SUPPORT_MULTIPOINT = &H400
Global Const XP1_MULTIPOINT_CONTROL_PLANE = &H800
Global Const XP1_MULTIPOINT_DATA_PLANE = &H1000
Global Const XP1_QOS_SUPPORTED = &H2000
Global Const XP1_INTERRUPT = &H4000
Global Const XP1_UNI_SEND = &H8000
Global Const XP1_UNI_RECV = &H10000
Global Const XP1_IFS_HANDLES = &H20000
Global Const XP1_PARTIAL_MESSAGE = &H40000

Global Const BIGENDIAN = &H0
Global Const LITTLEENDIAN = &H1

Global Const SECURITY_PROTOCOL_NONE = &H0

'/*
' * WinSock 2 extension -- manifest constants for WSAJoinLeaf()
' */
Global Const JL_SENDER_ONLY = &H1
Global Const JL_RECEIVER_ONLY = &H2
Global Const JL_BOTH = &H4

'/*
' * WinSock 2 extension -- manifest constants for WSASocket()
' */
Global Const WSA_FLAG_OVERLAPPED = &H1
Global Const WSA_FLAG_MULTIPOINT_C_ROOT = &H2
Global Const WSA_FLAG_MULTIPOINT_C_LEAF = &H4
Global Const WSA_FLAG_MULTIPOINT_D_ROOT = &H8
Global Const WSA_FLAG_MULTIPOINT_D_LEAF = &H10

'/*
' * WinSock 2 extension -- manifest constants for WSAIoctl()
' */
Global Const IOC_UNIX = &H0
Global Const IOC_WS2 = &H8000000
Global Const IOC_PROTOCOL = &H10000000
Global Const IOC_VENDOR = &H18000000

Global Const IOCPARM_MASK = &H7F              '/* parameters must be < 128 bytes */
Global Const IOC_VOID = &H20000000            '/* no parameters */
Global Const IOC_OUT = &H40000000             '/* copy out parameters */
Global Const IOC_IN = &H80000000              '/* copy in parameters */
Global Const IOC_INOUT = IOC_IN Or IOC_OUT

Global Const SIO_ASSOCIATE_HANDLE = IOC_IN Or IOC_WS2 Or 1
Global Const SIO_ENABLE_CIRCULAR_QUEUEING = IOC_VOID Or IOC_WS2 Or 2
Global Const SIO_FIND_ROUTE = IOC_OUT Or IOC_WS2 Or 3
Global Const SIO_FLUSH = IOC_VOID Or IOC_WS2 Or 4
Global Const SIO_GET_BROADCAST_ADDRESS = IOC_OUT Or IOC_WS2 Or 5
Global Const SIO_GET_EXTENSION_FUNCTION_POINTER = IOC_INOUT Or IOC_WS2 Or 6
Global Const SIO_GET_QOS = IOC_INOUT Or IOC_WS2 Or 7
Global Const SIO_GET_GROUP_QOS = IOC_INOUT Or IOC_WS2 Or 8
Global Const SIO_MULTIPOINT_LOOPBACK = IOC_IN Or IOC_WS2 Or 9
Global Const SIO_MULTICAST_SCOPE = IOC_IN Or IOC_WS2 Or 10
Global Const SIO_SET_QOS = IOC_IN Or IOC_WS2 Or 11
Global Const SIO_SET_GROUP_QOS = IOC_IN Or IOC_WS2 Or 12
Global Const SIO_TRANSLATE_HANDLE = IOC_INOUT Or IOC_WS2 Or 13
Global Const SIO_ROUTING_INTERFACE_QUERY = IOC_INOUT Or IOC_WS2 Or 20
Global Const SIO_ROUTING_INTERFACE_CHANGE = IOC_IN Or IOC_WS2 Or 21
Global Const SIO_ADDRESS_LIST_QUERY = IOC_OUT Or IOC_WS2 Or 22
Global Const SIO_ADDRESS_LIST_CHANGE = IOC_VOID Or IOC_WS2 Or 23
Global Const SIO_QUERY_TARGET_PNP_HANDLE = IOC_OUT Or IOC_WS2 Or 24

Type tcp_keepalive
    onoff As Long
    keepalivetime As Long
    keepaliveinterval As Long
End Type

Global Const SIO_RCVALL = IOC_IN Or IOC_VENDOR Or 1
Global Const SIO_RCVALL_MCAST = IOC_IN Or IOC_VENDOR Or 2
Global Const SIO_RCVALL_IGMPMCAST = IOC_IN Or IOC_VENDOR Or 3
Global Const SIO_KEEPALIVE_VALS = IOC_IN Or IOC_VENDOR Or 4
Global Const SIO_ABSORB_RTRALERT = IOC_IN Or IOC_VENDOR Or 5
Global Const SIO_UCAST_IF = IOC_IN Or IOC_VENDOR Or 6
Global Const SIO_LIMIT_BROADCASTS = IOC_IN Or IOC_VENDOR Or 7
Global Const SIO_INDEX_BIND = IOC_IN Or IOC_VENDOR Or 8
Global Const SIO_INDEX_MCASTIF = IOC_IN Or IOC_VENDOR Or 9
Global Const SIO_INDEX_ADD_MCAST = IOC_IN Or IOC_VENDOR Or 10
Global Const SIO_INDEX_DEL_MCAST = IOC_IN Or IOC_VENDOR Or 11

'/*
' * SockAddr Information
' */
Type SOCKET_ADDRESS
    lpSockaddr As Long
    iSockaddrLength As Long
End Type
'/*
' * CSAddr Information
' */
Type CSADDR_INFO
    LocalAddr As Long
    RemoteAddr As Long
    iSocketType As Long
    iProtocol As Long
End Type
'/*
' * Address list returned via SIO_ADDRESS_LIST_QUERY
' */
Type SOCKET_ADDRESS_LIST
    iAddressCount As Long
    Address(20) As SOCKET_ADDRESS 'here we hack a little bit
End Type
'/*
' *  Address Family/Protocol Tuples
' */
Type AFPROTOCOLS
    iAddressFamily As Long
    iProtocol As Long
End Type

'address constants
Global Const INADDR_NONE = &HFFFF
Global Const INADDR_ANY = &H0

Global Const SOL_SOCKET = &HFFFF&

Global Const INVALID_SOCKET = -1
Global Const SOCKET_ERROR = -1

Global Const MAXGETHOSTSTRUCT = 1024

Type GUID
    Data1 As Long
    Data2 As Integer
    Data3 As Integer
    Data4(7) As Byte
End Type
 
Global Const MAX_PROTOCOL_CHAIN = 7

Global Const BASE_PROTOCOL = 1
Global Const LAYERED_PROTOCOL = 0

Type WSAPROTOCOLCHAIN
     ChainLen As Long                                 '/* the length of the chain,     */
                                                  '/* length = 0 means layered protocol, */
                                                  '/* length = 1 means base protocol, */
                                                  '/* length > 1 means protocol chain */
    ChainEntries(MAX_PROTOCOL_CHAIN - 1) As Long   '/* a list of dwCatalogEntryIds */
End Type

Global Const WSAPROTOCOL_LEN = 255

Type WSAPROTOCOL_INFO
    dwServiceFlags1 As Long
    dwServiceFlags2 As Long
    dwServiceFlags3 As Long
    dwServiceFlags4 As Long
    dwProviderFlags As Long
    ProviderId As GUID
    dwCatalogEntryId As Long
    ProtocolChain As WSAPROTOCOLCHAIN
    iVersion As Long
    iAddressFamily As Long
    iMaxSockAddr As Long
    iMinSockAddr As Long
    iSocketType As Long
    iProtocol As Long
    iProtocolMaxOffset As Long
    iNetworkByteOrder As Long
    iSecurityScheme As Long
    dwMessageSize As Long
    dwProviderReserved As Long
    szProtocol(WSAPROTOCOL_LEN) As Byte
End Type

Global Const NS_ALL = 0

Global Const NS_SAP = 1
Global Const NS_NDS = 2
Global Const NS_PEER_BROWSE = 3

Global Const NS_TCPIP_LOCAL = 10
Global Const NS_TCPIP_HOSTS = 11
Global Const NS_DNS = 12
Global Const NS_NETBT = 13
Global Const NS_WINS = 14

Global Const NS_NBP = 20

Global Const NS_MS = 30
Global Const NS_STDA = 31
Global Const NS_NTDS = 32

Global Const NS_X500 = 40
Global Const NS_NIS = 41
Global Const NS_NISPLUS = 42

Global Const NS_WRQ = 50

Type WSANAMESPACE_INFO
    NSProviderId As GUID
    dwNameSpace As Long
    fActive As Long
    dwVersion As Long
    lpszIdentifier As Long
End Type

Type sockaddr
    sin_family As Integer
    sin_port As Integer
    sin_addr As Long
    sin_zero As String * 8
End Type
Global Const sockaddr_size = 16

Type HostEnt
    h_name As Long
    h_aliases As Long
    h_addrtype As Integer
    h_length As Integer
    h_addr_list As Long
End Type
Global Const hostent_size = 16

Global Const WSA_DESCRIPTIONLEN = 256
Global Const WSA_DescriptionSize = WSA_DESCRIPTIONLEN + 1
Global Const WSA_SYS_STATUS_LEN = 128
Global Const WSA_SysStatusSize = WSA_SYS_STATUS_LEN + 1

Type WSADataType
    wVersion As Integer
    wHighVersion As Integer
    szDescription As String * WSA_DescriptionSize
    szSystemStatus As String * WSA_SysStatusSize
    iMaxSockets As Integer
    iMaxUdpDg As Integer
    lpVendorInfo As Long
End Type

Type LingerType
    l_onoff As Integer
    l_linger As Integer
End Type


Global Const FD_READ_BIT = 0
Global Const FD_READ = 1         ''(1 << FD_READ_BIT)

Global Const FD_WRITE_BIT = 1
Global Const FD_WRITE = &H2        ''(1 << FD_WRITE_BIT)

Global Const FD_OOB_BIT = 2
Global Const FD_OOB = &H4          '(1 << FD_OOB_BIT)

Global Const FD_ACCEPT_BIT = 3
Global Const FD_ACCEPT = &H8       '(1 << FD_ACCEPT_BIT)

Global Const FD_CONNECT_BIT = 4
Global Const FD_CONNECT = &H10      '(1 << FD_CONNECT_BIT)

Global Const FD_CLOSE_BIT = 5
Global Const FD_CLOSE = &H20       '(1 << FD_CLOSE_BIT)

Global Const FD_QOS_BIT = 6
Global Const FD_QOS = &H40          '(1 << FD_QOS_BIT)

Global Const FD_GROUP_QOS_BIT = 7
Global Const FD_GROUP_QOS = &H80   '(1 << FD_GROUP_QOS_BIT)

Global Const FD_ROUTING_INTERFACE_CHANGE_BIT = 8
Global Const FD_ROUTING_INTERFACE_CHANGE = &H100     '(1 << FD_ROUTING_INTERFACE_CHANGE_BIT)

Global Const FD_ADDRESS_LIST_CHANGE_BIT = 9
Global Const FD_ADDRESS_LIST_CHANGE = &H200   '(1 << FD_ADDRESS_LIST_CHANGE_BIT)

Global Const FD_MAX_EVENTS = 10
Global Const FD_ALL_EVENTS = &H3FF     '(1 << FD_MAX_EVENTS) - 1)


Type WSANETWORKEVENTS
    lNetWorkEvents As Long
    iErrorCode(FD_MAX_EVENTS - 1) As Long
End Type

Global Const WSA_WAIT_FAILED = &HFFFFFFFF
Global Const WSA_WAIT_EVENT_0 = 0
Global Const WSA_WAIT_TIMEOUT = &H102

Public Const GMEM_FIXED = &H0
Public Declare Function GlobalAlloc Lib "Kernel32" (ByVal wFlags As Long, ByVal dwBytes As Long) As Long
Public Declare Function GlobalFree Lib "Kernel32" (ByVal hMem As Long) As Long
Public Declare Sub CopyMemory Lib "Kernel32" Alias "RtlMoveMemory" (dest As Any, src As Any, ByVal cb As Long)
Public Declare Sub CopyMemory2 Lib "Kernel32" Alias "RtlMoveMemory" (dest As Any, ByVal src As Long, ByVal cb As Long)
Public Declare Sub CopyMemory3 Lib "Kernel32" Alias "RtlMoveMemory" (dest As Long, ByVal src As Long, ByVal cb As Long)
Public Declare Sub ZeroMemory Lib "Kernel32" Alias "RtlZeroMemory" (dest As Any, ByVal numBytes As Long)
Public Declare Function StringFromGUID2 Lib "ole32.dll" (pGUID As GUID, ByVal _
PointerToString As String, ByVal MaxLength As Long) As Long
Public Declare Function lstrcpy Lib "Kernel32" Alias "lstrcpyA" (ByVal lpString1 As String, lpString2 As Byte) As Long
Public Declare Function lstrlen Lib "Kernel32" Alias "lstrlenA" (ByVal lpString As String) As Long
Public Declare Function lstrcpy1 Lib "Kernel32" Alias "lstrcpyA" (ByVal lpString1 As String, ByVal lpString2 As Long) As Long
Public Declare Function GetCurrentProcessId Lib "Kernel32" () As Long
Public Declare Function GetTickCount Lib "Kernel32" () As Long
Public Declare Function Sleep Lib "Kernel32" (ByVal dwMilliseconds As Long) As Long

Declare Function getsockname Lib "ws2_32.DLL" (ByVal s As Long, sname As sockaddr, namelen As Long) As Long
Declare Function WSAStartup Lib "ws2_32.DLL" (ByVal wVR As Long, lpWSAD As WSADataType) As Long
Declare Function WSACleanup Lib "ws2_32.DLL" () As Long
Declare Function bind Lib "ws2_32.DLL" (ByVal s As Long, addr As sockaddr, ByVal namelen As Long) As Long
Declare Function accept Lib "ws2_32.DLL" (ByVal s As Long, addr As sockaddr, namelen As Long) As Long
Declare Function socket Lib "ws2_32.DLL" (ByVal af As Long, ByVal s_type As Long, ByVal protocol As Long) As Long
Declare Function WSASocket Lib "ws2_32.DLL" Alias "WSASocketA" (ByVal af As Long, ByVal s_type As Long, ByVal protocol As Long, lpProtocolInfo As Any, ByVal g As Long, ByVal dwFlags As Long) As Long
Declare Function closesocket Lib "ws2_32.DLL" (ByVal s As Long) As Long
Declare Function connect Lib "ws2_32.DLL" (ByVal s As Long, addr As sockaddr, ByVal namelen As Long) As Long
Declare Function gethostbyname Lib "ws2_32.DLL" (ByVal host_name As String) As Long
Declare Function gethostbyaddr Lib "ws2_32.DLL" (addr As Any, ByVal nlen As Long, ByVal ntype As Long) As Long
Declare Function gethostname Lib "ws2_32.DLL" (ByVal host_name As String, ByVal namelen As Long) As Long
Declare Function recv Lib "ws2_32.DLL" (ByVal s As Long, Buf As Any, ByVal buflen As Long, ByVal flags As Long) As Long
Declare Function recvfrom Lib "ws2_32.DLL" (ByVal s As Long, Buf As Any, ByVal buflen As Long, ByVal flags As Long, from As sockaddr, fromlen As Long) As Long
Declare Function send Lib "ws2_32.DLL" (ByVal s As Long, Buf As Any, ByVal buflen As Long, ByVal flags As Long) As Long
Declare Function sendto Lib "ws2_32.DLL" (ByVal s As Long, Buf As Any, ByVal buflen As Long, ByVal flags As Long, to_addr As Any, ByVal tolen As Long) As Long
Declare Function htonl Lib "ws2_32.DLL" (ByVal hostlong As Long) As Long
Declare Function htons Lib "ws2_32.DLL" (ByVal hostshort As Long) As Integer
Declare Function ntohs Lib "ws2_32.DLL" (ByVal netshort As Long) As Integer
Declare Function ntohl Lib "ws2_32.DLL" (ByVal netlong As Long) As Long
Declare Function inet_addr Lib "ws2_32.DLL" (ByVal cp As String) As Long
Declare Function inet_ntoa Lib "ws2_32.DLL" (ByVal in_n As Long) As Long
Declare Function setsockopt Lib "ws2_32.DLL" (ByVal s As Long, ByVal level As Long, ByVal optname As Long, optval As Long, ByVal optlen As Long) As Long
Declare Function setsockopt2 Lib "ws2_32.DLL" Alias "setsockopt" (ByVal s As Long, ByVal level As Long, ByVal optname As Long, optval As Any, ByVal optlen As Long) As Long
Declare Function getsockopt Lib "ws2_32.DLL" (ByVal s As Long, ByVal level As Long, ByVal optname As Long, optval As Long, optlen As Long) As Long
Declare Function getsockopt2 Lib "ws2_32.DLL" Alias "getsockopt" (ByVal s As Long, ByVal level As Long, ByVal optname As Long, optval As Any, optlen As Long) As Long
Declare Function listen Lib "ws2_32.DLL" (ByVal s As Long, ByVal backlog As Long) As Long
Declare Function WSAIoctl Lib "ws2_32.DLL" (ByVal s As Long, ByVal dwIoControlCode As Long, lpvInBuffer As Any, ByVal cbInBuffer As Long, _
     lpvOutBuffer As Any, ByVal cbOutBuffer As Long, lpcbBytesReturned As Long, lpOverlapped As Any, lpCompletionRoutine As Any) As Long
Declare Function WSAEnumProtocols Lib "ws2_32.DLL" Alias "WSAEnumProtocolsA" (ByVal lpiProtocols As Long, ByVal lpProtocolBuffer As Long, lpdwBufferLength As Long) As Long
'in VB, application should use Err.LastDllError to get last error of any API call
'Declare Function WSAGetLastError Lib "ws2_32.DLL" () As Long
Declare Function WSACreateEvent Lib "ws2_32.DLL" () As Long
Declare Function WSACloseEvent Lib "ws2_32.DLL" (ByVal hEvent As Long) As Boolean
Declare Function WSAEventSelect Lib "ws2_32.DLL" (ByVal s As Long, ByVal hEventOjbect As Long, ByVal lNetWorkEvents As Long) As Long
Declare Function WSAEnumNetworkEvents Lib "ws2_32.DLL" (ByVal s As Long, ByVal hEventOjbect As Long, lpNetWorkEvents As WSANETWORKEVENTS) As Long
Declare Function WSAWaitForMultipleEvents Lib "ws2_32.DLL" (ByVal cEvents As Long, _
    lphEvents As Long, ByVal fWaitAll As Boolean, _
    ByVal dwTimeOUT As Long, ByVal fAlertable As Boolean) As Long
Declare Function WSAResetEvent Lib "ws2_32.DLL" (ByVal hEvent As Long) As Boolean

Declare Function WSAEnumNameSpaceProviders Lib "ws2_32.DLL" Alias "WSAEnumNameSpaceProvidersA" (lpdwBufferLength As Long, ByVal lpnspBuffer As Long) As Boolean

Public Const ICMP_ECHO = 8
Public Const ICMP_ECHOREPLY = 0
Public Const ICMP_MIN = 8  'minimum 8 byte icmp packet (header)
Public Const IP_RECORD_ROUTE = 7

' The IP header
Type IpHeader
   h_len As Byte              ' length of the header and  Version of IP
   tos As Byte                ' Type of service
   total_len As Integer       ' total length of the packet
   ident As Integer           ' unique identifier
   frag_and_flags As Integer  ' flags
   ttl As Byte
   proto As Byte              ' protocol (TCP, UDP etc)
   CheckSum As Integer        ' IP checksum
   sourceIP As Long
   destIP As Long
End Type

' ICMP header
Type IcmpHeader
  i_type As Byte
  i_code As Byte
  i_cksum As Integer
  i_id As Integer
  i_seq As Integer
  timestamp As Long   '/* This is not the std header, but we reserve space for time */
End Type

' IP option header - user with socket option IP_OPTIONS
Type IpOptionHeader
    code As Byte        'option type
    len As Byte         'length of option hdr
    ptr As Byte         'offset into options
    addr(8) As Long     'list of IP addrs
End Type

Function GetHostByNameAlias(ByVal hostname As String) As Long
    On Error Resume Next
    'Return IP address as a long, in network byte order

    Dim phe As Long    ' pointer to host information entry
    Dim heDestHost As HostEnt 'hostent structure
    Dim addrList As Long
    Dim retIP As Long
    'first check to see if what we have been passed is a valid IP
    retIP = inet_addr(hostname)
    If retIP = INADDR_NONE Then
        'it wasn't an IP, so do a DNS lookup
        phe = gethostbyname(hostname)
        If phe <> 0 Then
            'Pointer is non-null, so copy in hostent structure
            CopyMemory heDestHost, ByVal phe, hostent_size
            'Now get first pointer in address list
            CopyMemory addrList, ByVal heDestHost.h_addr_list, 4
            CopyMemory retIP, ByVal addrList, heDestHost.h_length
        Else
            'its not a valid address
            retIP = INADDR_NONE
        End If
    End If
    GetHostByNameAlias = retIP
    If Err Then GetHostByNameAlias = INADDR_NONE
End Function

Public Function TCPIPStartup() As Boolean
  Dim rc As Integer   'Return code
  Dim wVersionRequested As Long   'Version requested for winsocks
  Dim WSAData As WSADataType          'Detais os winsock implementation
  
  wVersionRequested = &H202
  TCPIPStartup = True
  rc = WSAStartup(wVersionRequested, WSAData)
  If rc <> 0 Then
    MsgBox ("RC: " & rc & " Unable to start winsocks" & ", Error " & Err.LastDllError)
    Call TCPIPShutDown
    TCPIPStartup = False
    Exit Function
  End If

End Function

Public Function TCPIPShutDown() As Boolean
    WSACleanup
End Function

