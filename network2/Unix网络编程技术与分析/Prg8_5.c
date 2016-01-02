// File: prg8_3.c
	#include <stdio.h>          /* These are the usual header files */ 
	#include <strings.h>          /* for bzero() */
	#include <unistd.h>         /* for close() */
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <arpa/inet.h>
	#include <signal.h>
	#include <wait.h>
	#include <errno.h>

	#define PORT 1234   /* Port that will be opened */ 
	#define BACKLOG 2   /* Number of allowed connections */ 
	#define MAXDATASIZE 1000  

	void process_cli(int connectfd, sockaddr_in client);
	void sig_handler(int s);

	main() 
	{ 
	int listenfd, connectfd; /* socket descriptors */ 
	pid_t pid;
	struct sockaddr_in server; /* server's address information */ 
	struct sockaddr_in client; /* client's address information */ 
	int sin_size; 

	struct sigaction	act, oact;
	act.sa_handler = sig_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGCHLD, &act, &oact) < 0) {
	   perror("Sigaction failed!");
	   exit(1);
	   }

	/* Create TCP socket  */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
	   /* handle exception */
	   perror("Creating socket failed.");
	   exit(1);
	}

	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server,sizeof(server));
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
	while(1)
	{
	   /* accept connection */
	   if ((connectfd = accept(listenfd,(struct sockaddr *)&client,&sin_size))==-1) {
	      if (errno == EINTR) continue;
	      perror("accept() error\n"); 
	      exit(1); 
	      } 
	   /*  create child process */
	   if ((pid=fork())>0) {
	      /* parent process */
	      close(connectfd);
	      continue;
	      }
	   else if (pid==0) {
	      /* child process */
	      close(listenfd);
	      process_cli(connectfd, client);
	      exit(0);     
	   }
	   else {
	      printf("fork error\n");
	      exit(0);
	      }
	}
	close(listenfd);   /* close listenfd */         
	} 

	void process_cli(int connectfd, sockaddr_in client)
	{
	int num;
	char recvbuf[MAXDATASIZE], sendbuf[MAXDATASIZE], cli_name[MAXDATASIZE];

	printf("You got a connection from %s.  ",inet_ntoa(client.sin_addr) ); 
	/* Get client's name from client */
	num = recv(connectfd, cli_name, MAXDATASIZE,0);
	if (num == 0) {
	   close(connectfd);
	   printf("Client disconnected.\n");
	   return;
	   }
	cli_name[num - 1] = '\0';
	printf("Client's name is %s.\n",cli_name);

	while (num = recv(connectfd, recvbuf, MAXDATASIZE,0)) {
	   recvbuf[num] = '\0';
	   printf("Received client( %s ) message: %s",cli_name, recvbuf);
	   for (int i = 0; i < num - 1; i++) {
	      sendbuf[i] = recvbuf[num - i -2];
      }
   sendbuf[num - 1] = '\0';
   send(connectfd,sendbuf,strlen(sendbuf),0); 
   }
 close(connectfd); /*  close connectfd */ 
 }

 void sig_handler(int s)
 {
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
   printf("child %d terminated\n", pid);
	   return;
}



