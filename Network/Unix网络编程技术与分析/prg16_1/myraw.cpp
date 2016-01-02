
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>   /* inet_ntoa */
#include <unistd.h>      /* close */
#include <stdio.h>       /* Basic I/O routines          */
#include <sys/types.h>   /* standard system types       */
#include <netinet/in.h>  /* Internet address structures */
#include <sys/socket.h>  /* socket interface functions  */
#include <netdb.h>       /* host to IP resolution       */
#include <netinet/in_systm.h>
#include <netinet/ip.h>  /* ICMP */
#include "myraw.h"


/****************
class: myRaw
******************/

myRaw::myRaw(int protocol = 0) 
{
   sock = socket(AF_INET, SOCK_RAW,protocol);
   setuid(getuid());
   
   if (sock == -1) error= 1;
   else error = 0;
}

myRaw::~myRaw()
{
   close(sock);
}

int myRaw::send(const void* msg,int msglen,sockaddr* to, unsigned int len) 
{
   if (error) return -1;
   int length = sendto(sock,msg,msglen,0,(const sockaddr*)to,len); 
   if (length == -1) {
      error = 2;
      return -1;
      }
   return length; 
}

int myRaw::send(const void* msg,int msglen,char* hostname) 
{
   sockaddr_in sin;        // Sock Internet address
   
   if (error) return -1;
   
   if(hostname) {
    hostent *hostnm = gethostbyname(hostname); 
    if(hostnm == (struct hostent *) 0) {
      return -1;
    }
    sin.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
   }
   else   
    return -1; 
   sin.sin_family = AF_INET;
   
   return send(msg,msglen,(sockaddr *)&sin, sizeof(sin));
}

int myRaw::sendIP(const void* msg,int msglen,sockaddr* to, unsigned int len) 
{
	int on = 1;
    setsockopt(sock,IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
	
    return send(msg, msglen, to, len); 
}

int myRaw::receive(void* buf,int buflen,sockaddr* from,int* len)
{
   if (error) return -1;
   while (1) {
      int length = recvfrom(sock,buf,buflen,0,from,len); 
      if (length == -1) 
        if (errno == EINTR) continue;
      else {
        error = 3;
        return -1;
        }
      return length;
      }
}

