// File: prg4_1.c
	#include <stdio.h>          /* These are the usual header files */ 
	#include <strings.h>         /* for bzero() */
	#include <unistd.h>          /* for close() */
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <arpa/inet.h>

	#define PORT 1234   /* Port that will be opened */ 
	#define BACKLOG 1   /* Number of allowed connections */ 

	main() 
	{ 
	int listenfd, connectfd; /* socket descriptors */ 
	struct sockaddr_in server; /* server's address information */ 
	struct sockaddr_in client; /* client's address information */ 
	int sin_size; 

	/* Create TCP socket */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
	/* handle exception */
	perror("Creating socket failed.");
	exit(1);
	}

	/* set socket can be reused */
	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server,sizeof(server));   /* fill server with 0s */
	server.sin_family=AF_INET; 
	server.sin_port=htons(PORT); 
	server.sin_addr.s_addr = htonl (INADDR_ANY); 
	if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) { 
	/* handle exception */
	perror("Bind error.");
	exit(1); 
	}    

	if(listen(listenfd,BACKLOG) == -1){  /* calls listen() */ 
	perror("listen() error\n"); 
	exit(1); 
	} 

	sin_size=sizeof(struct sockaddr_in); 
	if ((connectfd = accept(listenfd,(struct sockaddr *)&client,&sin_size))==-1) {
	/* calls accept() */ 
	perror("accept() error\n"); 
	exit(1); 
	} 

	printf("You got a connection from %s\n",inet_ntoa(client.sin_addr) ); /* prints client's IP */
	send(connectfd,"Welcome to my server.\n",22,0); /* send to the client welcome message */ 

	close(connectfd); /*  close connectfd */ 
	close(listenfd);   /* close listenfd */         
	}


