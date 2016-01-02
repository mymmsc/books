/* File : mysocket.cpp  */

#include<string.h>
#include <fcntl.h>
#include "mysocket.h"

MySocket::MySocket()
// Socket constructor that performs no initialization other then
// setting default values for the socket data members.
{
  address_family = AF_INET;      // Default address family
  socket_type = SOCK_STREAM;     // Default socket type
  protocol_family = IPPROTO_TCP; // Default protocol family
  port_number = MySOCKET_DEFAULT_PORT; // Default port number
  Mysocket = -1;
  conn_socket = -1;
  bytes_read = bytes_moved = 0;
  is_connected = 0;
  is_bound = 0;
  socket_error = MySOCKET_NO_ERROR;
}

MySocket::MySocket(sa_family_t af, int st, int pf,
		   int port, char *hostname)
// Socket constructor used to initialize the socket according to the
// address family, socket type, and protocol family. A hostname name should
// only be specified for client sockets.
{
  Mysocket = -1;
  conn_socket = -1;
  bytes_read = bytes_moved = 0;
  is_connected = 0;
  is_bound = 0;
  socket_error = MySOCKET_NO_ERROR;

  // Initialize the socket. NOTE: Any errors detected during initialization
  // will be recorded in the socket_error member.
  InitSocket(af, st, pf, port, hostname);
}

MySocket::MySocket(int st, int port, char *hostname) 
// Socket constructor used to initialize the socket according to the
// socket type. A hostname name should only be specified for client
// sockets.
{
  Mysocket = -1;
  conn_socket = -1;
  bytes_read = bytes_moved = 0;
  is_connected = 0;
  is_bound = 0;
  socket_error = MySOCKET_NO_ERROR;

  // Initialize the socket. NOTE: Any errors detected during initialization
  // will be recorded in the socket_error member.
  InitSocket(st, port, hostname);
}

MySocket::~MySocket()
{
  Close();
}

int MySocket::Socket()
// Create a socket. Returns a valid socket descriptor or
// -1 if the socket cannot be initialized.
{
  Mysocket = socket(address_family, socket_type, protocol_family);
  if(Mysocket < 0)
    {
      socket_error = MySOCKET_INIT_ERROR;
      return -1;
    }

  return Mysocket;
}

int MySocket::InitSocket(sa_family_t af,
				 int st,
				 int pf,
				 int port, char *hostname)
// Create and initialize a socket according to the address family,
// socket type, and protocol family. The "hostname" variable is an
// optional parameter that allows clients to specify a server name.
// Returns a valid socket descriptor or -1 if the socket cannot be
// initialized. 
{

  address_family = af;
  socket_type = st;
  protocol_family = pf;
  port_number = port;

  // Put the server information into the server structure.
  sin.sin_family = address_family;

  if(hostname) {
    // Get the server's Internet address
    hostent *hostnm = gethostbyname(hostname); 
    if(hostnm == (struct hostent *) 0) {
      socket_error = MySOCKET_HOSTNAME_ERROR;
      return -1;
    }
    // Put the server information into the client structure.
    sin.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
  }
  else   
    sin.sin_addr.s_addr = INADDR_ANY; // Use my IP address 
  sin.sin_port = htons(port_number);

  // Create a TCP/IP
  if(Socket() < 0) {
    socket_error = MySOCKET_INIT_ERROR;
    return -1;
  }

  return Mysocket;
}

int MySocket::InitSocket(int st, int port,
				 char *hostname)
// Create and initialize a socket according to the socket type. This 
// cross-platform fucntion will only accept SOCK_STREAM and SOCK_DGRAM
// socket types. The "hostname" variable is an optional parameter that 
// allows clients to specify a server name. Returns a valid socket 
// descriptor or -1 if the socket cannot be initialized. 
{
  address_family = AF_INET;
  port_number = port; 

  if(st == SOCK_STREAM) {
    socket_type = SOCK_STREAM;
    protocol_family = IPPROTO_TCP;
  }
  else if(st == SOCK_DGRAM) {
    socket_type = SOCK_DGRAM;
    protocol_family = IPPROTO_UDP;
  }
  else {
    socket_error = MySOCKET_SOCKETTYPE_ERROR;
    return -1;
  }

  // Put the server information into the server structure.
  sin.sin_family = address_family;

  if(hostname) {
    // Get the server's Internet address
    hostent *hostnm = gethostbyname(hostname); 
    if(hostnm == (struct hostent *) 0) {
      socket_error = MySOCKET_HOSTNAME_ERROR;
      return -1;
    }

    // Put the server information into the client structure.
    sin.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
  }
  else   
    sin.sin_addr.s_addr = INADDR_ANY; // Use my IP address

  sin.sin_port = htons(port_number);

  // Create a TCP/IP socket
  if(Socket() < 0) {
    socket_error = MySOCKET_INIT_ERROR;
    return -1;
  }

  return Mysocket;
}

int MySocket::Bind()
// Bind the socket to a name so that other processes can
// reference it and allow this socket to receive messages.
// Returns -1 if an error occurs.
{
  int rv = bind(Mysocket, (struct sockaddr *)&sin, sizeof(sin));
  if(rv >= 0) {
    is_bound = 1;
  } 
  else {
    socket_error = MySOCKET_BIND_ERROR;
    is_bound = 0;
  }
  return rv;
}

int MySocket::Connect()
// Connect the socket to a client or server. On the client side
// a connect call is used to initiate a connection.
// Returns -1 if an error occurs.
{
  int rv = connect(Mysocket, (struct sockaddr *)&sin, sizeof(sin));
  if(rv >= 0) {
    is_connected = 1; 
  }
  else {
    socket_error = MySOCKET_CONNECT_ERROR;
    is_connected = 0;
  }
  return rv;
}

int MySocket::ReadSelect(int s, int seconds, int useconds)
// Function used to multiplex reads without polling. Returns false if a 
// reply time is longer then the timeout values. 
{
  struct timeval timeout;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);

  timeout.tv_sec = seconds;
  timeout.tv_usec = useconds;

  // This function calls select() giving it the file descriptor of
  // the socket. The kernel reports back to this function when the file
  // descriptor has woken it up.
  return select(s+1, &fds, 0, 0, &timeout);
}

int MySocket::Recv(void *buf, int bytes, int flags)
// Receive a block of data from the bound socket and do not return
// until all the bytes have been read. Returns the total number of 
// bytes received or -1 if an error occurs.
{
  return Recv(Mysocket, buf, bytes, flags);
}

int MySocket::Recv(void *buf, int bytes, int seconds, int useconds, int flags)
// Receive a block of data from the bound socket and do not return
// until all the bytes have been read or the timeout value has been 
// exceeded. Returns the total number of bytes received or -1 if an 
// error occurs.
{
  return Recv(Mysocket, buf, bytes, seconds, useconds, flags);
}

int MySocket::Send(const void *buf, int bytes, int flags)
// Send a block of data to the bound socket and do not return
// until all the bytes have been written. Returns the total number 
// of bytes sent or -1 if an error occurs.
{
  return Send(Mysocket, buf, bytes, flags);
}

int MySocket::Recv(int s, void *buf, int bytes, int flags)
// Receive a block of data from a specified socket and do not return
// until all the bytes have been read. Returns the total number of
// bytes received or -1 if an error occurs.
{
  bytes_read = 0;           // Reset the byte counter
  int num_read = 0;         // Actual number of bytes read
  int num_req = (int)bytes; // Number of bytes requested 
  char *p = (char *)buf;    // Pointer to the buffer

  while(bytes_read < bytes) { // Loop until the buffer is full
    if((num_read = recv(s, p, num_req-bytes_read, flags)) > 0) {
      bytes_read += num_read;   // Increment the byte counter
      p += num_read;            // Move the buffer pointer for the next read
    }
    if(num_read < 0) {
      socket_error = MySOCKET_RECEIVE_ERROR;
      return -1; // An error occurred during the read
    }
  }
  
  return bytes_read;
}

int MySocket::Recv(int s, void *buf, int bytes, 
		   int seconds, int useconds, int flags)
// Receive a block of data from a specified socket and do not return
// until all the bytes have been read or the timeout value has been 
// exceeded. Returns the total number of bytes received or -1 if an 
// error occurs.
{
  bytes_read = 0;           // Reset the byte counter
  int num_read = 0;         // Actual number of bytes read
  int num_req = (int)bytes; // Number of bytes requested 
  char *p = (char *)buf;    // Pointer to the buffer

  while(bytes_read < bytes) { // Loop until the buffer is full
    if(!ReadSelect(s, seconds, useconds)) {
      socket_error = MySOCKET_REQUEST_TIMEOUT;
      return -1; // Exceeded the timeout value
    }
    if((num_read = recv(s, p, num_req-bytes_read, flags)) > 0) {
      bytes_read += num_read;   // Increment the byte counter
      p += num_read;            // Move the buffer pointer for the next read
    }
    if(num_read < 0) {
      socket_error = MySOCKET_RECEIVE_ERROR;
      return -1; // An error occurred during the read
    }
  }
  
  return bytes_read;
}

int MySocket::Send(int s, const void *buf, int bytes, int flags)
// Send a block of data to a specified socket and do not return
// until all the bytes have been written. Returns the total number of
// bytes sent or -1 if an error occurs.
{
  bytes_moved = 0;           // Reset the byte counter
  int num_moved = 0;         // Actual number of bytes written
  int num_req = (int)bytes;  // Number of bytes requested 
  char *p = (char *)buf;     // Pointer to the buffer

  while(bytes_moved < bytes) { // Loop until the buffer is empty
    if((num_moved = send(s, p, num_req-bytes_moved, flags)) > 0) {
      bytes_moved += num_moved;  // Increment the byte counter
      p += num_moved;            // Move the buffer pointer for the next read
    }
    if(num_moved < 0) {
      socket_error = MySOCKET_TRANSMIT_ERROR;
      return -1; // An error occurred during the read
    }
  }

  return bytes_moved;
}

int MySocket::RemoteRecv(void *buf, int bytes, int seconds, int useconds, 
			 int flags)
// Receive a block of data from a remote socket in blocking mode with a
// specified timeout value. Returns the total number of bytes received or 
// -1 if an error occurs.
{
  return Recv(conn_socket, buf, bytes, seconds, useconds, flags);
}

int MySocket::RemoteRecv(void *buf, int bytes, int flags)
// Receive a block of data from a remote socket in blocking mode.
// Returns the total number of bytes received or -1 if an error occurs.
{
  return Recv(conn_socket, buf, bytes, flags);
}

int MySocket::RemoteSend(const void *buf, int bytes, int flags)
// Send a block of data to a remote socket and do not return
// until all the bytes have been written. Returns the total 
// number of bytes received or -1 if an error occurs.
{
  return Send(conn_socket, buf, bytes, flags);
}

void MySocket::ShutDown(int how)
// Used to close and un-initialize a full-duplex socket.
{
  bytes_moved = 0;
  bytes_read = 0;
  is_connected = 0;
  is_bound = 0;

  if(Mysocket != -1) shutdown(Mysocket, how);
  if(conn_socket != -1) shutdown(conn_socket, how);

  Mysocket = -1;
  conn_socket = -1;
}

void MySocket::ShutDown(int &s, int how)
// Used to close and un-initialize the specified full-duplex socket.
{
  if(s != -1) shutdown(s, how);
  s = -1;
}

void MySocket::ShutDownSocket(int how)
// Used to close a full-duplex server side socket.
{
  if(Mysocket != -1) shutdown(Mysocket, how);
  Mysocket = -1;
}

void MySocket::ShutDownRemoteSocket(int how)
// Used to close a full-duplex client side socket.
{
  if(conn_socket != -1) shutdown(conn_socket, how);
  conn_socket = -1;
}

void MySocket::Close()
// Close any and un-initialize any bound sockets.
{
  bytes_moved = 0;
  bytes_read = 0;
  is_connected = 0;
  is_bound = 0;

  if(Mysocket != -1) close(Mysocket);
  if(conn_socket != -1) close(conn_socket);

}

void MySocket::Close(int &s)
// Close the specified socket
{

  if(s != -1) close(s);
  s = -1;
}

void MySocket::CloseSocket()
// Close the server side socket
{
  if(Mysocket != -1) close(Mysocket);
  Mysocket = -1;
}

void MySocket::CloseRemoteSocket()
// Close the client socket
{

  if(conn_socket != -1) close(conn_socket);

  conn_socket = -1;
}

int MySocket::Listen(int max_connections)
// Listen for connections if configured as a server.
// The "max_connections" variable determines how many
// pending connections the queue will hold. Returns -1
// if an error occurs.
{
  int rv = listen(Mysocket,         // Bound socket
		  max_connections); // Number of connection request queue
  if(rv < 0) socket_error = MySOCKET_LISTEN_ERROR;
  return rv;
}

int MySocket::Accept()
// Accept a connect from a remote socket. An Accept() 
// call blocks the server until the a client requests 
// service. Returns a valid socket descriptor or -1 
// if an error occurs.
{
  // Length of client address
  int addr_size = (int)sizeof(remote_sin); 

  conn_socket = accept(Mysocket, (struct sockaddr *)&remote_sin, &addr_size);

  if(conn_socket < 0)
    {
      socket_error = MySOCKET_ACCEPT_ERROR;
      return -1;
    }

  return conn_socket;
}

int MySocket::GetSockName(int s, sockaddr_in *sa)
// Retrieves the current name for the specified socket descriptor.
// It is used on a bound and/or connected socket and returns the
// local association. This function is especially useful when a
// connect call has been made without doing a bind first in which
// case this function provides the only means by which you can
// determine the local association which has been set by the system.
// Returns -1 if an error occurs.
{
  int namelen = (int)sizeof(sockaddr_in);
  int rv = getsockname(s, (struct sockaddr *)sa, &namelen);
  if(rv < 0) socket_error = MySOCKET_SOCKNAME_ERROR;
  return rv;
}

int MySocket::GetSockName()
// Retrieves the current name for this objects socket descriptor.
// Returns -1 if an error occurs.
{
  return GetSockName(Mysocket, &sin);
}

int MySocket::GetPeerName(int s, sockaddr_in *sa)
// Retrieves the current name of the specified socket descriptor.
// Returns -1 if an error occurs.
{
  int namelen = (int)sizeof(sockaddr_in);
  int rv = getpeername(s, (struct sockaddr *)&sa, &namelen);
  if(rv < 0) socket_error = MySOCKET_PEERNAME_ERROR;
  return rv;
}

int MySocket::GetPeerName()
// Retrieves the current name for the remote socket descriptor.
// Returns -1 if an error occurs.
{
  return GetPeerName(conn_socket, &remote_sin);
}

int MySocket::GetSockOpt(int s, int level, int optName, 
			 void *optVal, unsigned *optLen)
// Gets the current socket option for the specified option level or name.
// Returns -1 if an error occurs.
{

  int rv = getsockopt(s, level, optName, optVal, (int *)optLen);
  if(rv < 0) socket_error = MySOCKET_SETOPTION_ERROR;
  return rv;
}

int MySocket::GetSockOpt(int level, int optName, void *optVal, 
			 unsigned *optLen)
// Gets the current socket option for the specified option level or name.
// Returns -1 if an error occurs.
{
  return GetSockOpt(Mysocket, level, optName, optVal, optLen);
}

int MySocket::SetSockOpt(int s, int level, int optName,
			 const void *optVal, unsigned optLen)
// Sets the current socket option for the specified option level or name.
// Returns -1 if an error occurs.
{

  int rv = setsockopt(s, level, optName, optVal, (int)optLen);
  if(rv < 0) socket_error = MySOCKET_SETOPTION_ERROR;
  return rv;
}

int MySocket::SetSockOpt(int level, int optName, const void *optVal, 
			 unsigned optLen)
// Sets the current socket option for the specified option level or name.
// Returns -1 if an error occurs.
{
  return SetSockOpt(Mysocket, level, optName, optVal, optLen);
}

servent *MySocket::GetServiceInformation(char *name, char *protocol)
// Function used to obtain service information about a specified name. 
// The source of this information is dependent on the calling function's
// platform configuration which should be a local services file or NIS 
// database. Returns a pointer to a servent data structure if service
// information is available or a null value if the service cannot be
// found. NOTE: The calling function must free the memory allocated
// for servent data structure upon each successful return.
{
  // If the "protocol" pointer is NULL, getservbyname returns
  // the first service entry for which the name matches the s_name
  // or one of the s_aliases. Otherwise getservbyname matches both
  // the name and the proto.
  servent *sp = getservbyname(name, protocol);
  if(sp == 0) return 0;

  servent *buf = new servent;
  if(!buf) return 0; // Memory allocation error
  memmove(buf, sp, sizeof(servent));
  return buf;
}

servent *MySocket::GetServiceInformation(int port, char *protocol)
// Function used to obtain service information about a specified port. 
// The source of this information is dependent on the calling function's
// platform configuration which should be a local services file or NIS 
// database. Returns a pointer to a servent data structure if service
// information is available or a null value if the service cannot be
// found. NOTE: The calling function must free the memory allocated
// for servent data structure upon each successful return.
{
  // If the "protocol" pointer is NULL, getservbyport returns the
  // first service entry for which the port matches the s_port.
  // Otherwise getservbyport matches both the port and the proto.
  servent *sp = getservbyport(port, protocol);
  if(sp == 0) return 0;

  servent *buf = new servent;
  if(!buf) return 0; // Memory allocation error
  memmove(buf, sp, sizeof(servent));
  return buf;
}

int MySocket::GetServByName(char *name, char *protocol)
// Set service information corresponding to a service name and protocol.
// Returns -1 if an unknown service or protocol is requested. NOTE: This
// information is obtained from this machines local services file or
// from a NIS database.
{
  // If the "protocol" pointer is NULL, getservbyname returns
  // the first service entry for which the name matches the s_name
  // or one of the s_aliases. Otherwise getservbyname matches both
  // the name and the proto.
  servent *sp = getservbyname(name, protocol);
  if(sp == 0) {
    socket_error = MySOCKET_PROTOCOL_ERROR;
    return -1;
  }
  sin.sin_port = sp->s_port;
  return 0;
}

int MySocket::GetServByPort(int port, char *protocol)
// Set service information corresponding to a port number and protocol.
// Returns -1 if an unknown service or protocol is requested. NOTE: This
// information is obtained from this machines local services file or
// from a NIS database.
{
  // If the "protocol" pointer is NULL, getservbyport returns the
  // first service entry for which the port matches the s_port.
  // Otherwise getservbyport matches both the port and the proto.
  servent *sp = getservbyport(port, protocol);
  if(sp == 0) {
    socket_error = MySOCKET_PROTOCOL_ERROR;
    return -1;
  }
  sin.sin_port = sp->s_port;
  return 0;
}

int MySocket::GetPortNumber()
// Return the port number actually set by the system. Use this function
// after a call to MySocket::GetSockName();
{
  return ntohs(sin.sin_port);
}

int MySocket::GetRemotePortNumber()
// Return the port number of the client socket.
{
  return ntohs(remote_sin.sin_port);
}

sa_family_t MySocket::GetAddressFamily()
// Returns the address family of this socket
{
  return sin.sin_family;
}

sa_family_t MySocket::GetRemoteAddressFamily()
// Returns the address family of the remote socket.
{
  return remote_sin.sin_family;  
}

hostent *MySocket::GetHostInformation(char *hostname)
// Function used to obtain hostname information about a specified host. 
// The source of this information is dependent on the calling function's
// platform configuration which should be a DNS, local host table, and/or
// NIS database. Returns a pointer to a hostent data structure
// if information is available or a null value if the hostname cannot be
// found. NOTE: The calling function must free the memory allocated
// for hostent data structure upon each successful return.
{
  in_addr hostia;
  hostent *hostinfo;
  hostia.s_addr = inet_addr(hostname);

  if(hostia.s_addr == NULL) { // Look up host by name
    hostinfo = gethostbyname(hostname); 
  }
  else {  // Look up host by IP address
    hostinfo = gethostbyaddr((const char *)&hostia, 
			     sizeof(in_addr), AF_INET);
  }
  if(hostinfo == (hostent *) 0) { // No host name info avialable
    return 0;
  }

  hostent *buf = new hostent;
  if(!buf) return 0; // Memory allocation error
  memmove(buf, hostinfo, sizeof(hostent));
  return buf;
}
  
int MySocket::GetHostName(char *sbuf)
// Pass back the host name of this machine in the "sbuf" variable.
// A memory buffer equal to "MysMAX_NAME_LEN" must be pre-allocated
// prior to using this function. Return -1 if an error occurs.
{
  // Prevent crashes if memory has not been allocated
  if(!sbuf) sbuf = new char[MysMAX_NAME_LEN]; 
  int rv = gethostname(sbuf, MysMAX_NAME_LEN);
  if(rv < 0) socket_error = MySOCKET_HOSTNAME_ERROR;
  return rv;
}

int MySocket::GetIPAddress(char *sbuf)
// Pass back the IP Address of this machine in the "sbuf" variable.
// A memory buffer equal to "MysMAX_NAME_LEN" must be pre-allocated
// prior to using this function. Return -1 if an error occurs.
{
  char hostname[MysMAX_NAME_LEN];
  int rv = GetHostName(hostname);
  if(rv < 0) return rv;

  in_addr *ialist;
  hostent *hostinfo = GetHostInformation(hostname);
  if(!hostinfo) {
    socket_error = MySOCKET_HOSTNAME_ERROR;
    return -1;
  }
  ialist = (in_addr *)hostinfo->h_addr_list[0];

  // Prevent crashes if memory has not been allocated
  if(!sbuf) sbuf = new char[MysMAX_NAME_LEN]; 

  strcpy(sbuf, inet_ntoa(*ialist));
  delete hostinfo;
  return 0;
}

int MySocket::GetDomainName(char *sbuf)
// Pass back the domain name of this machine in the "sbuf" variable.
// A memory buffer equal to "MysMAX_NAME_LEN" must be pre-allocated
// prior to using this function. Return -1 if an error occurs.
{
  char hostname[MysMAX_NAME_LEN];
  int rv = GetHostName(hostname);
  if(rv < 0) return rv;

  hostent *hostinfo = GetHostInformation(hostname);
  if(!hostinfo) {
    socket_error = MySOCKET_HOSTNAME_ERROR;
    return -1;
  }
  // Prevent crashes if memory has not been allocated
  if(!sbuf) sbuf = new char[MysMAX_NAME_LEN]; 

  strcpy(sbuf, hostinfo->h_name);
  int i; int len = strlen(sbuf);
  for(i = 0; i < len; i++) {
    if(sbuf[i] == '.') break;
  }
  if(++i < len) {
    len -= i;
    memmove(sbuf, sbuf+i, len);
    sbuf[len] = 0; // Null terminate the string
  }
  delete hostinfo;
  return 0;
}

int MySocket::GetBoundIPAddress(char *sbuf)
// Pass back the local or server IP address in the "sbuf" variable.
// A memory buffer equal to "MysMAX_NAME_LEN" must be pre-allocated
// prior to using this function. Return -1 if an error occurs.
{
  char *s = inet_ntoa(sin.sin_addr);
  if(s == 0) {
    socket_error = MySOCKET_HOSTNAME_ERROR;
    return -1;
  }

  // Prevent crashes if memory has not been allocated
  if(!sbuf) sbuf = new char[MysMAX_NAME_LEN]; 

  strcpy(sbuf, s);
  return 0;
}

int MySocket::GetRemoteHostName(char *sbuf)
// Pass back the client host name client in the "sbuf" variable.
// A memory buffer equal to "MysMAX_NAME_LEN" must be pre-allocated
// prior to using this function. Return -1 if an error occurs.
{
  char *s = inet_ntoa(remote_sin.sin_addr);
  if(s == 0) {
    socket_error = MySOCKET_HOSTNAME_ERROR;
    return -1;
  }

  // Prevent crashes if memory has not been allocated
  if(!sbuf) sbuf = new char[MysMAX_NAME_LEN]; 

  strcpy(sbuf, s);
  return 0;
}

void MySocket::GetClientInfo(char *client_name, int &r_port)
// Get the client's host name and port number. NOTE: This
// function assumes that a block of memory equal to the
// MysMAX_NAME_LEN constant has already been allocated.
{
  int rv = GetRemoteHostName(client_name);
  if(rv < 0) {
    char *unc = "UNKNOWN";
    for(unsigned i = 0; i < MysMAX_NAME_LEN; i++) client_name[i] = '\0';
    strcpy(client_name, unc);
  }
  r_port = GetRemotePortNumber();
}

int MySocket::RecvFrom(void *buf, int bytes, int seconds, int useconds, 
		       int flags)
// Receive a block of data from a remote datagram socket 
// and do not return until all the bytes have been read 
// or the timeout value has been exceeded. Returns the total 
// number of bytes received or -1 if an error occurs.
{
  return RecvFrom(Mysocket, &remote_sin, buf, bytes, seconds, useconds, flags);
}

int MySocket::RecvFrom(void *buf, int bytes, int flags)
// Receive a block of data from a remote datagram socket 
// and do not return until all the bytes have been read. 
// Returns the total number of bytes received or -1 if 
// an error occurs.
{
  return RecvFrom(Mysocket, &remote_sin, buf, bytes, flags);
}

int MySocket::SendTo(void *buf, int bytes, int flags)
// Send a block of data to a datagram socket and do not return
// until all the bytes have been written. Returns the total number
// of bytes sent or -1 if an error occurs.
{
  return SendTo(Mysocket, &sin, buf, bytes, flags);
}

int MySocket::RecvFrom(int s, sockaddr_in *sa, void *buf,
		       int bytes, int seconds, int useconds, int flags)
// Receive a block of data from a remote datagram socket 
// and do not return until all the bytes have been read 
// or the timeout value has been exceeded. Returns the total 
// number of bytes received or -1 if an error occurs.
{
  // Length of client address
  int addr_size = (int)sizeof(sockaddr_in); 
  bytes_read = 0;           // Reset the byte counter
  int num_read = 0;         // Actual number of bytes read
  int num_req = (int)bytes; // Number of bytes requested 
  char *p = (char *)buf;    // Pointer to the buffer

  while(bytes_read < bytes) { // Loop until the buffer is full
    if(!ReadSelect(s, seconds, useconds)) {
      socket_error = MySOCKET_REQUEST_TIMEOUT;
      return -1; // Exceeded the timeout value
    }
    if((num_read = recvfrom(s, p, num_req-bytes_read, flags, (struct sockaddr *)sa, &addr_size)) > 0) {
      bytes_read += num_read;   // Increment the byte counter
      p += num_read;            // Move the buffer pointer for the next read
    }
    if(num_read < 0) {
      socket_error = MySOCKET_RECEIVE_ERROR;
      return -1; // An error occurred during the read
    }
  }
  return bytes_read;
}

int MySocket::RecvFrom(int s, sockaddr_in *sa, void *buf,
		       int bytes, int flags)
// Receive a block of data from a remote datagram socket 
// and do not return until all the bytes have been read. 
// Returns the total number of bytes received or -1 if 
// an error occurs.
{
  // Length of client address
  int addr_size = (int)sizeof(sockaddr_in); 
  bytes_read = 0;           // Reset the byte counter
  int num_read = 0;         // Actual number of bytes read
  int num_req = (int)bytes; // Number of bytes requested 
  char *p = (char *)buf;    // Pointer to the buffer

  while(bytes_read < bytes) { // Loop until the buffer is full
    if((num_read = recvfrom(s, p, num_req-bytes_read, flags, (struct sockaddr *)sa, &addr_size)) > 0) {
      bytes_read += num_read;   // Increment the byte counter
      p += num_read;            // Move the buffer pointer for the next read
    }
    if(num_read < 0) {
      socket_error = MySOCKET_RECEIVE_ERROR;
      return -1; // An error occurred during the read
    }
  }
  return bytes_read;
}

int MySocket::SendTo(int s, sockaddr_in *sa, void *buf,
		     int bytes, int flags)
// Send a block of data to a datagram socket and do not return
// until all the bytes have been written. Returns the total number
// of bytes sent or -1 if an error occurs.
{
  // Length of address
  int addr_size = (int)sizeof(sockaddr_in);
  bytes_moved = 0;             // Reset the byte counter
  int num_moved = 0;           // Actual number of bytes written
  int num_req = (int)bytes;    // Number of bytes requested 
  char *p = (char *)buf;       // Pointer to the buffer

  while(bytes_moved < bytes) { // Loop until the buffer is full
    if((num_moved = sendto(s, p, num_req-bytes_moved, flags, (const struct sockaddr *)sa, addr_size)) > 0) {
      bytes_moved += num_moved;  // Increment the byte counter
      p += num_moved;            // Move the buffer pointer for the next read
    }
    if(num_moved < 0) {
      socket_error = MySOCKET_TRANSMIT_ERROR;
      return -1; // An error occurred during the read
    }
  }
  return bytes_moved;
}








