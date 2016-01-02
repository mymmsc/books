
#include <unistd.h>  // UNIX standard function definitions 
#include <termios.h> // POSIX terminal control definitions 
#include <fcntl.h>   // File control definitions
#include <string.h>
#include "SerialComm.h"

SerialComm::SerialComm()
{
  // Default setting 
  baud_rate = 9600;
  parity = 'N';
  character_size = 8;
  stop_bits = 1;
  flow_control = SerialComm::scommNO_FLOW_CONTROL;
  binary_mode = 1; 
  scomm_error = SerialComm::scomm_NO_ERROR;

  memset(&options, 0, sizeof(options));
}

SerialComm::~SerialComm()
{
  Close();
}

int SerialComm::OpenSerialPort(char *device_name)
// Open a serial device for read/write operations. 
{

  device_handle = open(device_name, O_RDWR | O_NOCTTY | O_NDELAY);

  if(device_handle < 0) 
    device_handle = open(device_name, O_RDONLY | O_NOCTTY | O_NDELAY);
  else 
    return scommREAD_WRITE;
  
  if(device_handle < 0) 
    device_handle = open(device_name, O_WRONLY | O_NOCTTY | O_NDELAY);
  else 
    return scommREAD_ONLY;
  
  if(device_handle < 0) {
    scomm_error = SerialComm::scomm_OPEN_ERROR;
    return -1;
  }
  else
    return scommWRITE_ONLY;

}

int SerialComm::InitSerialPort()
{
  // Reset the port options
  memset(&options, 0, sizeof(options));

  // Set the baud rates
  switch(baud_rate) {
    case 0: // 0 baud (drop DTR)
      cfsetispeed(&options, B0);
      cfsetospeed(&options, B0);
      break;
    case 50: 
      cfsetispeed(&options, B50);
      cfsetospeed(&options, B50);
      break;
    case 75: 
      cfsetispeed(&options, B75);
      cfsetospeed(&options, B75);
      break;
    case 110: 
      cfsetispeed(&options, B110);
      cfsetospeed(&options, B110);
      break;
    case 134: 
      cfsetispeed(&options, B134);
      cfsetospeed(&options, B134);
      break;
    case 150: 
      cfsetispeed(&options, B150);
      cfsetospeed(&options, B150);
      break;
    case 200: 
      cfsetispeed(&options, B200);
      cfsetospeed(&options, B200);
      break;
    case 300: 
      cfsetispeed(&options, B300);
      cfsetospeed(&options, B300);
      break;
    case 600: 
      cfsetispeed(&options, B600);
      cfsetospeed(&options, B600);
      break;
    case 1200: 
      cfsetispeed(&options, B1200);
      cfsetospeed(&options, B1200);
      break;
    case 1800: 
      cfsetispeed(&options, B1800);
      cfsetospeed(&options, B1800);
      break;
    case 2400: 
      cfsetispeed(&options, B2400);
      cfsetospeed(&options, B2400);
      break;
    case 4800: 
      cfsetispeed(&options, B4800);
      cfsetospeed(&options, B4800);
      break;
    case 9600: 
      cfsetispeed(&options, B9600);
      cfsetospeed(&options, B9600);
      break;
    case 19200: 
      cfsetispeed(&options, B19200);
      cfsetospeed(&options, B19200);
      break;
    case 57600: 
      cfsetispeed(&options, B57600);
      cfsetospeed(&options, B57600);
      break;
    case 115200: 
      cfsetispeed(&options, B115200);
      cfsetospeed(&options, B115200);
      break;
    default:
      scomm_error = SerialComm::scomm_BAUDRATE_ERROR;
      return -1; // Invalid baud rate
  }

  // Set the character size, parity and stop bits
  if((character_size == 8) && (parity == 'N') && (stop_bits == 1)) {
    // No parity (8N1) 
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_iflag = IGNPAR;
  }
  else if((character_size == 7) && (parity == 'E') && (stop_bits == 1)) {
    // Even parity (7E1) 
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    options.c_iflag |= (INPCK | ISTRIP);
  }
  else if((character_size == 7) && (parity == 'O') && (stop_bits == 1)) {
    // Odd parity (7O1) 
    options.c_cflag |= PARENB;
    options.c_cflag |= PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    options.c_iflag |= (INPCK | ISTRIP);
  }
  else if((character_size == 7) && (parity == 'M') && (stop_bits == 1)) {
    // Mark parity is simulated by using 2 stop bits (7M1)
    options.c_cflag &= ~PARENB;
    options.c_cflag |= CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    options.c_iflag |= (INPCK | ISTRIP);
  }
  else if((character_size == 7) && (parity == 'S') && (stop_bits == 1)) {
    // Space parity is setup the same as no parity (7S1)
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    options.c_iflag |= (INPCK | ISTRIP);
  }
  else {
    scomm_error = SerialComm::scomm_INVALIDPARM;
    return -1; // Invalid character size, parity and stop bits combination
  }
  
  switch(flow_control) {
    case scommHARD_FLOW :
#if defined(CRTSCTS)
      options.c_cflag |= CRTSCTS;
      break;
#else
      break; // Hard flow control is not supported
#endif

    case scommXON_XOFF :
      options.c_iflag |= (IXON | IXOFF | IXANY);
#if defined(CRTSCTS)
      options.c_cflag &= ~CRTSCTS;
#endif
      break;

    case scommSOFT_FLOW :
#if defined(CRTSCTS)
      options.c_cflag &= ~CRTSCTS;
      break;
#else
      break; // Hard flow control is not supported
#endif

    case scommNO_FLOW_CONTROL :
#if defined(CRTSCTS)
      options.c_cflag &= ~CRTSCTS;
      break;
#else
      break; // Hard flow control is not supported
#endif

    default:
      scomm_error = SerialComm::scomm_FLOWCONTROL_ERROR;
      return -1; // Invalid flow control
  }

  if(!binary_mode) { // Set the port for canonical input (line-oriented) 
    // Input characters are put into a buffer until a CR (carriage return)
    // or LF (line feed) character is received.
    options.c_lflag |= (ICANON | ECHOE);

    // Postprocess the output.
    // The ONLCR flag will map NL (linefeeds) to CR-NL on output.
    // The OLCUC flag will map characters to upper case for tty output.
    options.c_oflag = OPOST | ONLCR | OLCUC;
  }
  else { // Use raw input/output
    // Input characters are passed through exactly as they are received,
    // when they are received.
    options.c_lflag = 0;
    options.c_oflag = 0;
  }
  
  // Enable the receiver and set local mode
  options.c_cflag |= (CLOCAL | CREAD);
  
  // Initialize control characters if needed.
  // Default values can be found in /usr/include/termios.h, and are given
  // in the comments.

  options.c_cc[VTIME]    = 0;     // inter-character timer unused 
  options.c_cc[VMIN]     = 1;     // blocking read until 1 character arrives 

 

  // Set the new options for the port. The TCSANOW constant specifies
  // that all changes should occur immediately without waiting for
  // output data to finish sending or input data to finish receiving. 
  tcflush(device_handle, TCIFLUSH); // Clean the serial line
  tcsetattr(device_handle, TCSANOW, &options);

return 1; // No errors reported
}

void SerialComm::Close()
{

  close(device_handle);

}

int SerialComm::RawRead(void *buf, int bytes)
// Read a specified number of bytes from the serial port
// and return whether or not the read was completed.
// Returns the number of bytes received or -1 if an
// error occurred.
{

  bytes_read = read(device_handle, (char *)buf, bytes);
  if(bytes_read < 0) {
    scomm_error = SerialComm::scomm_RECEIVE_ERROR;
    return -1;
  }
  return (int)bytes_read;
}

int SerialComm::RawWrite(const void *buf, int bytes)
// Write a specified number of bytes to a serial port
// and return whether or not the write was complete.
// Returns the total number of bytes moved or -1 if
// an error occurred.
{
  bytes_moved = write(device_handle, (char *)buf, bytes);
  if(bytes_moved < 0) {
    scomm_error = SerialComm::scomm_TRANSMIT_ERROR;
    return -1;
  }
  return (int)bytes_moved;
}

int SerialComm::Recv(void *buf, int bytes)
// Receive a specified number of bytes from a serial port
// and do not return until all the byte have been read.
// Returns the total number of bytes read or -1 if an
// error occurred.
{
  int br = 0;               // Byte counter
  int num_read = 0;         // Actual number of bytes read
  int num_req = (int)bytes; // Number of bytes requested 
  char *p = (char *)buf;    // Pointer to the buffer

  while(br < bytes) { // Loop unitl the buffer is full
    if((num_read = RawRead(p, num_req-br)) > 0) {
      br += num_read;   // Increment the byte counter
      p += num_read;    // Move the buffer pointer for the next read
    }
    if(num_read < 0) {
      scomm_error = SerialComm::scomm_RECEIVE_ERROR;
      return -1; // An error occurred during the read
    }
  }
  
  bytes_read = br; // Undate the object's byte counter
  return bytes_read;
}

int SerialComm::Send(const void *buf, int bytes)
// Write a specified number of bytes to a serial port and do
// not return until all the bytes have been written. Returns
// the total number of bytes written or -1 if an error occurred.
{
  int bm = 0;                // Byte counter
  int num_moved = 0;         // Actual number of bytes written
  int num_req = (int)bytes;  // Number of bytes requested 
  char *p = (char *)buf;     // Pointer to the buffer

  while(bm < bytes) { // Loop unitl the buffer is empty
    if((num_moved = RawWrite(p, num_req-bm)) > 0) {
      bm += num_moved;  // Increment the byte counter
      p += num_moved;   // Move the buffer pointer for the next read
    }
    if(num_moved < 0) {
      scomm_error = SerialComm::scomm_TRANSMIT_ERROR;
      return -1; // An error occurred during the read
    }
  }
  
  bytes_moved = bm; // Update the object's byte counter
  return bytes_moved;
}

int SerialComm::InitSerialPort(char *device_name, int sp, char pr, int cs,
				 int sb, int flow, int bin_mode)
// Initialize a serial device using the specified parameters. Returns
// -1 if an error occurred during initialization or the current access
// mode of the port (scommREAD_WRITE, scommREAD_ONLY, scommREAD_WRITE).
{
  int status = OpenSerialPort(device_name); 
  if(status < 0) return -1;

  SetBaudRate(sp);
  SetCharacterSize(cs);
  SetParity(pr);
  SetStopBits(sb);
  SetFlowControl(flow);
  if(bin_mode) BinaryMode(); else CharacterMode();

  if(InitSerialPort() < 0) return -1;
  
  return status;
}



