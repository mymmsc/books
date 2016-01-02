// File: prg6_2.c
	#include <stdio.h> 
	#include <unistd.h>
	#include <strings.h>
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <netdb.h>        /* netbd.h is needed for struct hostent =) */ 

	#define PORT 1234   /* Open Port on Remote Host */ 
	#define MAXDATASIZE 1000   /* Max number of bytes of data */ 

	void process(FILE *fp, int sockfd);

	int main(int argc, char *argv[]) 
	{ 
	int fd;   /* files descriptors */ 
	struct hostent *he;         /* structure that will get information about remote host */ 
	struct sockaddr_in server;  /* server's address information */ 

	if (argc !=2) {       
	   printf("Usage: %s <IP Address>\n",argv[0]); 
	   exit(1); 
	   } 

	if ((he=gethostbyname(argv[1]))==NULL){ /* calls gethostbyname() */ 
	   printf("gethostbyname() error\n"); 
	   exit(1); 
	   } 

	if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){  /* calls socket() */ 
	   printf("socket() error\n"); 
	   exit(1); 
	   } 

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_port = htons(PORT); /* htons() is needed again */ 
	server.sin_addr = *((struct in_addr *)he->h_addr);  

	if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1){ /* calls connect() */ 
	   printf("connect() error\n"); 
	   exit(1); 
	   } 
	process(stdin,fd);
	close(fd);   /* close fd */  
	} 

	void process(FILE *fp, int sockfd)
	{
	char	sendline[MAXDATASIZE], recvline[MAXDATASIZE];
	int numbytes;

	printf("Connected to server. \n");
	while (fgets(sendline, MAXDATASIZE, fp) != NULL) {
	   send(sockfd, sendline, strlen(sendline),0);

	   if ((numbytes = recv(sockfd, recvline, MAXDATASIZE,0)) == 0) {
	      printf("Server terminated.\n");
	      return;
	      }
	   recvline[numbytes]='\0'; 
	   printf("Server Message: %s\n",recvline); /* it prints server's welcome message  */ 
	   }
 }


