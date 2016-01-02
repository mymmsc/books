// File: prg5_2.c
	#include <stdio.h> 
	#include <unistd.h>
	#include <strings.h>
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <netdb.h>        /* netbd.h is needed for struct hostent =) */ 

	#define PORT 1234   /* Open Port on Remote Host */ 
	#define MAXDATASIZE 100   /* Max number of bytes of data */ 

	int main(int argc, char *argv[]) 
	{ 
	int fd, numbytes;   /* files descriptors */ 
	char buf[MAXDATASIZE];  /* buf will store received text */ 

	struct hostent *he;         /* structure that will get information about remote host */ 
	struct sockaddr_in server,reply;  /* server's address information */ 

	if (argc !=3) {  /* this is used because our program will need two argument (IP address and a message */ 
	printf("Usage: %s <IP Address> <message>\n",argv[0]); 
	exit(1); 
	} 

	if ((he=gethostbyname(argv[1]))==NULL){ /* calls gethostbyname() */ 
	printf("gethostbyname() error\n"); 
	exit(1); 
	} 

	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1){  /* calls socket() */ 
	printf("socket() error\n"); 
	exit(1); 
	} 

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_port = htons(PORT); /* htons() is needed again */ 
	server.sin_addr = *((struct in_addr *)he->h_addr);  /*he->h_addr passes "*he"'s info to "h_addr" */ 
	sendto(fd, argv[2], strlen(argv[2]),0,(struct sockaddr *)&server,sizeof(struct sockaddr)); 

	while (1) {
	int len;
	if ((numbytes=recvfrom(fd,buf,MAXDATASIZE,0,(struct sockaddr *)&reply,&len)) == -1){  /* calls recvfrom() */ 
	printf("recvfrom() error\n"); 
	exit(1); 
	} 

	if (len != sizeof(struct sockaddr) || memcmp((const void *)&server, (const void *)&reply,len) != 0) {
	printf("Receive message from other server.\n");
	continue;
	}

	buf[numbytes]='\0';
	printf("Server Message: %s\n",buf); /* it prints server's welcome message */
	break;
	}

	close(fd);   /* close fd */ 
	}


