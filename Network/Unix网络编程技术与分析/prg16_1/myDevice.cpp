#include <unistd.h>  // UNIX standard function definitions 
#include <termios.h> // POSIX terminal control definitions 
#include <fcntl.h>   // File control definitions
#include <string.h>
#include "SerialComm.h" 
#include "myDevice.h"

myDevice::myDevice(char* dev): SerialComm()
{
	h.flag = 0x7e;
	OpenSerialPort(dev);
	InitSerialPort();
}

int myDevice::read(void* buf)
{
	char buffer[2000];
    int len;

	len = RawRead(void *buffer, 2000);
	if (len < 0 ) {
		error = 1;
		return len;
	}
	memcpy(buf, buffer + sizeof(h), len - sizeof(h);
	return len - sizeof(h);
}

int myDevice::write(void* buf, int buflen, short src, short dst)
{
	char buffer[2000];

	h.src = src;
	h.dst = dst;
	h.len = buflen + sizeof(h);
	memcpy(buffer, h, sizeof(h));
	memcpy(buffer + sizeof(h), buf, buflen);

	int l = RawWrite(buffer, h.len);
	if ( l < 0) {
		error = 2;
	}
	return l;
}


