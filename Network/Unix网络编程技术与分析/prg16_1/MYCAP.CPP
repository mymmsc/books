#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> 
#include "mycap.h"

 /****************************************************
   class name : myCap
 ******************************************************/

myCap::myCap()
{
	device = lookupdev();
	snaplen = 1000;
	promisc = 0;
	to_ms = -1;
}
myCap::myCap(char *device1, int snaplen1, int promisc1, int to_ms1)
{
	device = device1;
	snaplen = snaplen1;
	promisc = promisc1;
	to_ms = to_ms1;
}
myCap::~myCap()
{
	close();
}

int myCap::init()
{
    /* ask pcap for the network address and mask of the device */
    int err = lookupnet(&netp,&maskp);
	if (err == -1) return -1;
	
    /* open device for reading  */
    p = open();
    if(p == NULL) { 
       return -1;
	}

	linktype = datalink();
}

int myCap::init(char *rule)
{
    /* ask pcap for the network address and mask of the device */
    int err = lookupnet(&netp,&maskp);
	if (err == -1) return -1;
	
    /* open device for reading  */
    p = open();
    if(p == NULL) { 
       return -1;
	}

	linktype = datalink();
 
	/* Lets try and compile the program.. non-optimized */
    if(compile(rule,0) == -1) { 
      return -1;
    }

    /* set the compiled program as the filter */
    if(pcap_setfilter(descr,&fp) == -1) { 
      return -1;
	}
}

int myCap::capture(char* buf)
{
	pcap_pkthdr h;
	char* b = next(&h);
	if (b == NULL) return -1;
	memcpy(buf,b,h.caplen);
	return h.caplen;
}

/****************************************************
   class name : myCapIP
******************************************************/
myCapIP::myCapIP(char *device, int snaplen, int promisc, int to_ms):
   myCap(device, snaplen, promisc, to_ms)
{
}

int myCapIP::init()
{
	return myCap::init("ip");
}

int myCapIP::capture(char* buf) 
{

	int	len;
	char	*ptr;
	struct ether_header	*eptr;
	pcap_pkthdr h;

	char* ptr = next(&h);
	if (ptr == NULL) return -1;

	len = h.caplen;	
	switch (linktype) {
		case DLT_NULL:	/* loopback header = 4 bytes */
			ptr += 4;
			len -= 4;
			break;
		case DLT_EN10MB:
			eptr = (struct ether_header *) ptr;
			if (ntohs(eptr->ether_type) != ETHERTYPE_IP)
				return -1;
			ptr += 14;
			len -= 14;
			break;			

		case DLT_SLIP:	/* SLIP header = 24 bytes */
			ptr += 24;
			len -= 24;
			break;
		case DLT_PPP:	/* PPP header = 24 bytes */
			ptr += 24;
			len -= 24;
			break;
		default:
			return -1;
		}
	}
	memcpy(buf,ptr,len);
	return 0;
}

int myCapIP::getProto(char* buf)
{
	ip  *packet;

	packet = (ip *)buf;
	return ip->ip_p;
}

in_addr myCapIP::getSaddr(char* buf)
{
	ip  *packet;

	packet = (ip *)buf;
	return ip->ip_src;
}

in_addr myCapIP::getDaddr(char* buf)
{
	ip  *packet;

	packet = (ip *)buf;
	return ip->ip_dst;
}



