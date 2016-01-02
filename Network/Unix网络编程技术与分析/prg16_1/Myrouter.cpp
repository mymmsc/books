#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "MyThread.h"
#include "mycap.h"
#include "myroute.h"
#include "myraw.h"
#include "mydevice.h"

/****************************************************
class name : sendThr
******************************************************/

class sendThr : public MyThread
{
	int error;
	short src, dst;
	char* buf;
	myCapIP  *capIP;
	myRoute  *route;
	myDevice *device;
public:
	sendThr(char*);
    sendThr(char*, short , short);
	~sendThr();
    void run();
	int Error() {return error;}
	void setSrc(short s) {src = s;}
	short getSrc() {return src;}
	void setDst(short d) {dst = d;}
	short getDst() { return dst;}
	
};

sendThr::sendThr(char* dev)
{
	src = 0;
	dst = 1;
	capIP = new myCapIP;
	capIP->init();
	route = new myRoute;
	device = new myDevice(dev);
	buf = new char[2000];
}

sendThr::sendThr(char* dev, short src1, short dst1)
{
	src = src1;
	dst = dst1;
	capIP = new myCapIP;
	capIP->init();
	route = new myRoute;
	device = new myDevice(dev);
	buf = new char[2000];
}

sendThr::~sendThr()
{
	delete capIP;
	delete device;
	delete route;
	delete[] buf;
}

void run()
{
	int len;

    while (1)  {
		// capture IP packet
		len = capIP->capture(buf);
        if (len < 0) {
		   error = 1;
		   continue;
		}

		// get IP address
		in_addr addr = capIP->getSaddr(buf);

		// check routing table
		char* ipaddr;
        ipaddr = inet_ntoa(addr);  //thread_unsaft 
		int exist = route->Get(ipaddr);
		if (exist < 1) continue; // no routing. 

		// send to serial com
		error = device->write(buf, len, src, dst);
	}
}

/****************************************************
class name : recvThr
******************************************************/

class recvThr : public MyThread
{
	int error;
	short src, dst;
	char* buf;
	myRaw  *ip;
	myDevice *device;
public:
    recvThr(char*);
	~recvThr();
    void run();
	int Error() {return error;}
};

recvThr::recvThr(char* dev)
{
	ip = new myRaw;
	device = new myDevice(dev);
	buf = new char[2000];
}

recvThr::~recvThr()
{
	delete ip;
	delete device;
	delete[] buf;
}

void run()
{
	int len;

    while (1)  {
		// receive IP packet from com
		len = device->read(buf);
        if (len < 0) {
		   error = 1;
		   continue;
		}

		// Get IP addr
		ip  *packet;
		packet = (ip *)buf;
		in_addr addr = ip->ip_src;
		// Build dst's addr
		sockaddr_in to;
		to.sin_addr = addr;
		to.sin_family = AF_INET;

		// send IP packet to network
		error = ip->sendIP((const void*)buf,len,(sockaddr*) &to, sizeof(to)); 
	}
}

/****************************************************
class name : myRouter
******************************************************/
class myRouter
{
	char* dev;
	int src;
	int dst;
	int error;
	sendThr *sender;
	recvThr *receiver;
	int MsgPriority;
public:
	myRouter(char* dev);
	~myRouter;

    int run();
	int daemon(char* name,int facility = 0);
	void SetPriority(int p) {MsgPriority = p;}
	int GetPriority() {return MsgPriority;}
	void setSrc(int s) {src = s;}
	int getSrc() {return src;}
	void setDst(int d) {dst = d;}
	int getDst() { return dst;}
	int Error() { return error;}
};

myRouter::myRouter(char* d)
{
	dev = d;
	MsgPriority = LOG_NOTICE|LOG_LOCAL0;
	sender = new sendThr(dev);
	receiver = new recvThr(dev);
}

myRouter::~myRouter()
{
	delete sender;
	delete receiver;
}

int myRouter::run()
{
	sender.setSrc((short)src);
	sender.setDst((short)dst);
	error = sender.Start();
	if (error) {
		syslog(MsgPriority,"Thread to send failed.\n");
		return;
	}
	error = receiver.Start();
	if (error) {
		syslog(MsgPriority,"Thread to receive failed.\n");
		return;
	}

	// wait for the threads 
    pthread_join(sender->getId(),NULL);
	pthread_join(receiver->getId(),NULL);
}

int myRouter::daemon(char* name,int facility)
{
    switch (fork())
    {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);          // exit the original process 
    }

    if (setsid() < 0)               
      return -1;

    signal(SIGHUP,SIG_IGN);  // ignore SIGHUP

    switch (fork())
    {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);
    }

    chdir("/");

    closeall(0);
    open("/dev/null",O_RDWR);
    dup(0); 
    dup(0);

    openlog(name,LOG_PID,facility);
    return 0;
}

/************  main program  *********************************/
int main(int argc, char* argv[])
{
	if (argc == 3) {
		myRouter router(argv[1]);
		router.setSrc(atoi(argv[2]));
		router.setDst(atoi(argv[3]));
		if (router.daemon(argv[0]) < 0) {
			perror("daemon");
			exit(1);
			}
		router.run();
	}
	else {
		printf("Usage: %s <device> <src> <dst>\n",argv[0]); 
		exit(1); 
		} 
}
