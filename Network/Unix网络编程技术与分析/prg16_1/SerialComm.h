typedef int scommDeviceHandle;

// Serial communications base class
class SerialComm
{
public: // Enumerations
  enum {
    // Internal error codes used to report the last error
    scomm_NO_ERROR  = 0,      // No errors reported
    scomm_INVALID_ERROR_CODE, // Invalid error code

    scomm_BAUDRATE_ERROR,    // Invalid baud rate
    scomm_CS_ERROR,          // Invalid character size
    scomm_FLOWCONTROL_ERROR, // Invalid flow control
    scomm_INIT_ERROR,        // Cannot initialize serial device
    scomm_INVALIDPARM,       // Invalid initialization parameter
    scomm_OPEN_ERROR,        // Cannot open serial device
    scomm_PARITY_ERROR,      // Invalid parity
    scomm_RECEIVE_ERROR,     // Serial device receive error
    scomm_STOPBIT_ERROR,     // Invalid stop bit
    scomm_TRANSMIT_ERROR,    // Serial device transmit error
  };
  
  enum {
    // Flow control constants
    scommHARD_FLOW,
    scommSOFT_FLOW,
    scommXON_XOFF,
    scommNO_FLOW_CONTROL,

    // Device access constants
    scommREAD_ONLY,
    scommWRITE_ONLY,
    scommREAD_WRITE
  };

public:
  SerialComm();
  virtual ~SerialComm();
  
public:
  int OpenSerialPort(char *device_name);
  int InitSerialPort();
  int InitSerialPort(char *device_name, int sp = 9600, char pr = 'N',
		     int cs = 8, int sb = 1,
		     int flow = SerialComm::scommNO_FLOW_CONTROL,
		     int bin_mode = 1);
  void Close();
  int RawRead(void *buf, int bytes);
  int RawWrite(const void *buf, int bytes);
  int Recv(void *buf, int bytes);
  int Send(const void *buf, int bytes);
  void SetBaudRate(int br) { baud_rate = br; }
  void SetCharacterSize(int cs) { character_size = cs; }
  void SetParity(char p) { parity = p; }
  void SetStopBits(int sb) { stop_bits = sb; }
  void SetFlowControl(int f) { flow_control = f; }
  scommDeviceHandle DeviceHandle() { return device_handle; }
  
  void BinaryMode() { binary_mode = 1; }
  void CharacterMode() { binary_mode = 0; }
  
  termios *GetTermIOS() { return &options; }
  int BytesRead() { return bytes_read; }
  int BytesMoved() { return bytes_moved; }

protected:
  scommDeviceHandle device_handle; // Device handle for the port
  int baud_rate;      // Baud rate
  int character_size; // Character size
  char parity;        // Parity 
  int stop_bits;      // Stop bits
  int flow_control;   // Flow control
  int binary_mode;    // True to enable raw reads and raw writes
  int scomm_error;    // The last reported serial port error
  
protected: // POSIX terminal (serial) interface extensions
  termios options; // Terminal control structure
  int bytes_read;  // Number of bytes read following a Read() call
  int bytes_moved; // Number of bytes written following a Write() call
};
