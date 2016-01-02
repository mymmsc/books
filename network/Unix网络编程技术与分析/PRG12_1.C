// File: prg12_1.c
 #include <iostream.h>
 #include <string.h>
 #include <errno.h>
 #include <signal.h>
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
 #include <netinet/ip_icmp.h> /* ICMP */ 

 #define ICMPHEAD  8  // ICMP packet header's length
 #define MAXICMPLEN 200  

 /****************
 class: RawSock
 ******************/
 class RawSock
 {
 public:
 int sock;
 int error;
 RawSock(int protocol =0);
 virtual  ~RawSock();
 int send(const void* msg, int msglen,sockaddr* addr,unsigned int len);
 int send(const void* msg, int msglen,char* addr);
 int receive(void* buf,int buflen,sockaddr* from,int* len);
 int Error() {return error;}
 }; 

 class ICMP: public RawSock
 {
 public:
 struct icmp *packet;
 int max_len;
 int length;

 ushort_t checksum(ushort_t *addr,int len);
 ICMP();
 ICMP(int len);
 ~ICMP();
 int send_icmp(char *to, void* buf,int len);
 int recv_icmp(sockaddr* from);
 void setCode(int c) {  packet->icmp_code = c;}
 void setId(int i) { packet->icmp_id = i; }
 void setSeq(int s) { packet->icmp_seq = s;}
 void setType(int t) { packet->icmp_type = t;}
 };

 RawSock::RawSock(int protocol = 0) {
 sock = socket(AF_INET, SOCK_RAW,protocol);
 setuid(getuid());
 if (sock == -1) error= 1;
   else error = 0;
 }

 RawSock::~RawSock(){
 close(sock);
 }

 int RawSock::send(const void* msg,int msglen,sockaddr* to, unsigned int len) {
 if (error) return -1;
 int length = sendto(sock,msg,msglen,0,(const sockaddr*)to,len); 
 if (length == -1) {
   error = 2;
   return -1;
   }
 return length; 
 }

 int RawSock::send(const void* msg,int msglen,char* hostname) {
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

 int RawSock::receive(void* buf,int buflen,sockaddr* from,int* len)
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

 /****************
 class: ICMP
 ******************/
 ICMP::ICMP():  RawSock(IPPROTO_ICMP)
 {
 max_len = MAXICMPLEN;
 packet = (icmp*) new char[max_len];

 packet->icmp_code = 0;
 packet->icmp_id = 0;
 packet->icmp_seq = 0;
 packet->icmp_type = ICMP_ECHO;
 }

 ICMP::ICMP(int len):  RawSock(IPPROTO_ICMP) 
 {
 max_len = len;
 packet = (icmp*) new char[max_len];

 packet->icmp_code = 0;
 packet->icmp_id = 0;
 packet->icmp_seq = 0;
 packet->icmp_type = ICMP_ECHO;
 }   

 ICMP::~ICMP()
 {
 delete[] (char*) packet;
 }

 ushort_t ICMP::checksum(ushort_t *addr,int len)
 {
 int nleft = len;
 int sum = 0;
 unsigned short *w = addr;
 unsigned short answer = 0;

 while (nleft > 1) {
   sum+=*w++;
   nleft -= 2;
   }

 if (nleft == 1) {
   *(unsigned char*) (&answer) = *(unsigned char*) w;
   sum += answer;
   }

 sum = (sum >> 16) + (sum & 0xffff);
 sum += (sum>>16);
 answer = ~sum;
 return (answer);
 }

 int ICMP::send_icmp(char *host, void* buf,int len)
 {  
 memcpy(packet->icmp_data,buf,len);
 packet->icmp_cksum = 0;
 packet->icmp_cksum = checksum((unsigned short *)packet, ICMPHEAD + 6);

 int err = send(packet,MAXICMPLEN,host);
 return err;
 }

 int ICMP::recv_icmp(sockaddr* from)
 {
 char buf[MAXICMPLEN + 100];
 int	hlen1, icmplen;
 struct ip	*ip;
 struct icmp  *icmp;

 if (Error()) return -1;
 int addrlen = 0;
 int len = receive(buf,MAXICMPLEN+100,from,&addrlen);

 if (len == -1) {
   cout << "Receiving Failed!\n" ;
   return -1;
   }

 ip = (struct ip *) buf;		/* start of IP header */
 hlen1 = ip->ip_hl << 2;		/* length of IP header */

 icmp = (struct icmp *) (buf + hlen1);	/* start of ICMP header */
 if ( (icmplen = len - hlen1) < 8){
   cout << "Receiving Failed!\n" ;
   return -1;
   }

 length = len - hlen1;
 memcpy(packet,icmp,length);
 return 0;
 }

 main(int argc, char *argv[])
 {
 ICMP icmp;
 sockaddr from;
 char *host;
 int count;

 if (argc < 2) {       
   printf("Usage: %s <IP Address> <try_number>\n",argv[0]); 
   exit(1); 
   } 
 if (argc == 2) {
   host = argv[1];
   count = 5;
   }
 if (argc == 3) {
   host = argv[1];
   count = atoi(argv[2]);
   }

 for (int i = 0; i <= count; i++) {
   icmp.setId(getpid());
   icmp.setSeq(i);
   char* test_data= "abcde";
   icmp.send_icmp(host,test_data,strlen(test_data));
   }
 int num = 1;
 while(1) {
   if (icmp.recv_icmp(&from) < 0) continue;
   if (icmp.packet->icmp_type == ICMP_ECHOREPLY) {
     if (icmp.packet->icmp_id == getpid()) {
       printf("%d bytes from %s: seq=%u, data=%s\n",
         icmp.length, host,icmp.packet->icmp_seq, icmp.packet->icmp_data);
       num ++;
       if (num > count) break;
       }
     }  
  }
 }


