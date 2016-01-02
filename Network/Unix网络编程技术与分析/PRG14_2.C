// File: prg14_1.c
 #include <stdio.h>          /* These are the usual header files */ 
 #include <strings.h>          /* for bzero() */
 #include <unistd.h>         /* for close() */
 #include <sys/types.h> 
 #include <sys/socket.h> 
 #include <netinet/in.h> 
 #include <arpa/inet.h>

 #define PORT 1234   /* Port that will be opened */ 
 #define BACKLOG 2   /* Number of allowed connections */ 
 void process_cli(int connectfd, sockaddr_in client);

 main(int argc, char* argv[]) 
 { 
 int listenfd, connectfd; /* socket descriptors */ 
 pid_t pid;

 struct sockaddr_in server; /* server's address information */ 
 struct sockaddr_in client; /* client's address information */ 
 int sin_size; 

 if (argc !=2) {       
   printf("Usage: %s <IP Address>\n",argv[0]); 
   exit(1); 
   } 

 if ((he=gethostbyname(argv[1]))==NULL){ /* calls gethostbyname() */ 
   printf("gethostbyname() error\n"); 
   exit(1); 
   } 

 /* Create TCP socket  */
 if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 {
   /* handle exception */
   perror("Creating socket failed.");
   exit(1);
   }

 bzero(&server,sizeof(server));
 server.sin_family=AF_INET; 
 server.sin_port=htons(PORT); 
 server.sin_addr = *((struct in_addr *)he->h_addr);
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
 /* accept  connection */
 if ((connectfd = accept(listenfd,(struct sockaddr *)&client,&sin_size))==-1) {
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
 sockaddr_in address;
 int namelen;

 getsockname(connectfd,  (sockaddr  *)&address,  &namelen);
 printf("Connected to address ( %s )\n  ",inet_ntoa(address.sin_addr) ); 
 close(connectfd); /*  close connectfd */ 
 }



