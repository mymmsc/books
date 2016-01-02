// File: prg5_1.c
	#include <stdio.h>          /* These are the usual header files */ 
	#include <strings.h>          /* for bzero() */
	#include <unistd.h>         /* for close() */
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <arpa/inet.h>

	#define PORT 1234   /* Port that will be opened */ 
	#define MAXDATASIZE 100   /* Max number of bytes of data */ 

	main() 
	{ 
	int sockfd; /* socket descriptors */ 
	struct sockaddr_in server; /* server's address information */ 
	struct sockaddr_in client; /* client's address information */ 
	int sin_size; 
	int num;
	char msg[MAXDATASIZE];  /* buffer for message */

	/* Creating UDP socket  */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)  {
	/* handle exception */
	perror("Creating socket failed.");
	exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family=AF_INET; 
	server.sin_port=htons(PORT); 
	server.sin_addr.s_addr = htonl (INADDR_ANY); 
	if (bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) { 
	/* handle exception */
	perror("Bind error.");
	exit(1); 
	}    

	sin_size=sizeof(struct sockaddr_in); 
	while (1)   {
	num = recvfrom(sockfd,msg,MAXDATASIZE,0,(struct sockaddr *)&client,&sin_size);                                    

	if (num < 0){
	perror("recvfrom error\n"); 
	exit(1); 
	} 

	msg[num] = '\0';
	printf("You got a message (%s%) from %s\n",msg,inet_ntoa(client.sin_addr) ); /* prints client's IP */  
	sendto(sockfd,"Welcome to my server.\n",22,0,(struct sockaddr *)&client,sin_size); /* send to the client 
welcome message */ 
	if (!strcmp(msg,"quit")) break;
	} 

	close(sockfd);   /* close listenfd */         
	}


