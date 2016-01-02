
#include <unistd.h>     // UNIX standard function definitions 
#include <sys/types.h>  // Type definitions
#include <netdb.h>      // Net DB structures
#include <arpa/inet.h>  // Inet functions
#include <netinet/in.h> // Structures defined by the internet system
#include <sys/socket.h> // Definitions related to sockets
#include <sys/time.h>   // Time value functions
/* File : mysocket.h */

/* Define constant */
const unsigned MysBUF_SIZE     =  1024; // Fixed string buffer length
const unsigned MysMAX_NAME_LEN =  256;  // Maximum string name length
const unsigned MysRX_BUF_SIZE  =  4096; // Default receive buffer size
const unsigned MysTX_BUF_SIZE  =  4096; // Default transmit buffer size
const unsigned MAXCONN  = 5;            // Max connection
const unsigned MySOCKET_DEFAULT_PORT = 1234;  // default port number

/* Define Error */
enum MySocketError {    // MySocket exception codes
  MySOCKET_NO_ERROR = 0,       // No socket errors reported
  MySOCKET_INVALID_ERROR_CODE, // Invalid socket error code

  // Socket error codes
  MySOCKET_ACCEPT_ERROR,         // Error accepting remote socket
  MySOCKET_BIND_ERROR,           // Could not bind socket
  MySOCKET_BUFOVER_ERROR,        // Buffer overflow
  MySOCKET_CONNECT_ERROR,        // Could not connect socket
  MySOCKET_FILESYSTEM_ERROR,     // A file system error occurred
  MySOCKET_GETOPTION_ERROR,      // Error getting socket option
  MySOCKET_HOSTNAME_ERROR,       // Could not resolve hostname
  MySOCKET_INIT_ERROR,           // Initialization error
  MySOCKET_LISTEN_ERROR,         // Listen error
  MySOCKET_PEERNAME_ERROR,       // Get peer name error
  MySOCKET_PROTOCOL_ERROR,       // Unknown protocol requested
  MySOCKET_RECEIVE_ERROR,        // Receive error
  MySOCKET_REQUEST_TIMEOUT,      // Request timed out
  MySOCKET_SERVICE_ERROR,        // Unknown service requested
  MySOCKET_SETOPTION_ERROR,      // Error setting socket option
  MySOCKET_SOCKNAME_ERROR,       // Get socket name error
  MySOCKET_SOCKETTYPE_ERROR,     // Unknown socket type requested
  MySOCKET_TRANSMIT_ERROR,       // Transmit error
};

 /****************************************************
 class name : MySocket
 Function: A wrapper class of basic socket function. 
 ******************************************************/
class MySocket
{
protected: // Socket variables 
  sa_family_t address_family;   // Object's address family
  int protocol_family;          // Object's protocol family
  int socket_type;              // Object's socket type
  int port_number;        // Object's port number
  int Mysocket;                   // Socket this object is bound to
  int conn_socket;              // Socket used for remote connections
  MySocketError socket_error;   // The last reported socket error

protected: // Process control variables
  int bytes_read;   // Number of bytes read following a read operation
  int bytes_moved;  // Number of bytes written following a write operation
  int is_connected; // True if the socket is connected
  int is_bound;     // True if the socket is bound
  
public: // Data structures used to set the internet domain and addresses
  sockaddr_in sin;        // Sock Internet address
  sockaddr_in remote_sin; // Remote socket Internet address

public:
  MySocket(); 
  MySocket(int st, int port, char *hostname = 0); 
  MySocket(sa_family_t af, int st, int pf,
	   int port, char *hostname = 0);
  virtual ~MySocket();
  
public: // Functions used to set the socket parameters
  void SetAddressFamily(sa_family_t af) { address_family = af; }
  void SetProtocolFamily(int pf) { protocol_family = pf; }
  int GetProtocolFamily() { return protocol_family; }
  void SetSocketType(int st) { socket_type = st; }
  int GetSocketType() { return socket_type; }
  void SetPortNumber(int p) { port_number = p; }
  int GetBoundSocket() { return Mysocket; } 
  int GetSocket() { return Mysocket; }  
  int GetRemoteSocket() { return conn_socket; }
  
public: // Socket functions
  int Socket();
  int InitSocket(int st, int port, char *hostname = 0);
  int InitSocket(sa_family_t af, int st, 
			 int pf, int port, char *hostname = 0);
  void Close();
  void Close(int &s);
  void CloseSocket();
  void CloseRemoteSocket();
  int Bind();
  int Connect();
  int Accept();
  int Listen(int max_connections = MAXCONN);
  int Recv(void *buf, int bytes, int flags = 0);
//just for client
  int Recv(int s, void *buf, int bytes, int flags = 0);
  int Recv(void *buf, int bytes, int seconds, int useconds, int flags = 0);
//just for client
  int Recv(int s, void *buf, int bytes, int seconds, int useconds, 
	   int flags = 0);
  int Send(const void *buf, int bytes, int flags = 0);
//just for client
  int Send(int s, const void *buf, int bytes, int flags = 0);
  int RemoteRecv(void *buf, int bytes, int seconds, int useconds, 
		 int flags = 0);
//just for server
  int RemoteRecv(void *buf, int bytes, int flags = 0);
//just for server
  int RemoteSend(const void *buf, int bytes, int flags = 0);
//just for server
  void ResetRead() { bytes_read = 0; }
  void ResetWrite() { bytes_moved = 0; } 
  void ShutDown(int how = 0);
  void ShutDown(int &s, int how = 0);
  void ShutDownSocket(int how = 0);
  void ShutDownRemoteSocket(int how = 0);
  int GetPeerName(int s, sockaddr_in *sa);
  int GetPeerName();
  int GetSockName(int s, sockaddr_in *sa);
  int GetSockName();
  int GetSockOpt(int s, int level, int optName, 
		 void *optVal, unsigned *optLen);
  int GetSockOpt(int level, int optName, void *optVal, unsigned *optLen);
  int SetSockOpt(int s, int level, int optName, 
		 const void *optVal, unsigned optLen);
  int SetSockOpt(int level, int optName, const void *optVal, unsigned optLen);
  int GetServByName(char *name, char *protocol = 0);
  int GetServByPort(int port, char *protocol = 0); 
  servent *GetServiceInformation(char *name, char *protocol = 0);
  servent *GetServiceInformation(int port, char *protocol = 0);
  int GetPortNumber(); 
  int GetRemotePortNumber();
  int GetHostName(char *sbuf);
  int GetIPAddress(char *sbuf);
  int GetDomainName(char *sbuf);
  int GetBoundIPAddress(char *sbuf);
  int GetRemoteHostName(char *sbuf);
  hostent *GetHostInformation(char *hostname);
  void GetClientInfo(char *client_name, int &r_port);
  sa_family_t GetAddressFamily();
  sa_family_t GetRemoteAddressFamily();
  
  // Process control functions
  int ReadSelect(int s, int seconds, int useconds); 
  int BytesRead() { return bytes_read; }
  int BytesMoved() { return bytes_moved; }
  int SetBytesRead(int bytes = 0) { return bytes_read = bytes; }
  int SetBytesMoved(int bytes = 0) { return bytes_moved = bytes; }
  int *GetBytesRead() { return &bytes_read; }
  int *GetBytesMoved() { return &bytes_moved; }
  int IsConnected() { return is_connected == 1; }
  int IsBound() { return is_bound == 1; }
  int SetSocket(int s) { return Mysocket = s; }
  int SetRemoteSocket(int s) { return conn_socket = s; }
  void ReleaseSocket() { Mysocket = (int)-1; }
  void ReleaseRemoteSocket() { conn_socket = (int)-1; }

  // Datagram functions
  int RecvFrom(int s, sockaddr_in *sa, void *buf,
	       int bytes, int seconds, int useconds, int flags = 0);
  int RecvFrom(int s, sockaddr_in *sa, void *buf,
	       int bytes, int flags = 0);
  int SendTo(int s, sockaddr_in *sa, void *buf,
	     int bytes, int flags = 0);
  int RecvFrom(void *buf, int bytes, int flags = 0);
  int RecvFrom(void *buf, int bytes, int seconds, int useconds, int flags = 0);
  int SendTo(void *buf, int bytes, int flags = 0);

  // Exception handling functions
  MySocketError GetSocketError() { return socket_error; }
  MySocketError GetSocketError() const { return socket_error; }
  MySocketError SetSocketError(MySocketError err) {
    return socket_error = err;
  }
  MySocketError ResetSocketError() {
    return socket_error = MySOCKET_NO_ERROR;
  }
  MySocketError ResetError() {
    return socket_error = MySOCKET_NO_ERROR;
  }

};

