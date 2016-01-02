struct head
{
	char flag;
	short src;
	short dst;
	short len;
};

class myDevice : public SerialComm
{
	head h;
	int error;
public:
	myDevice(char* dev);

	int read(void* buf);
	int write(void* buf,int buflen);
	int Error() {return error;}
};
