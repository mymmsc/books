// File: prg6_7.c
	#include <stdio.h>          /* These are the usual header files */ 
	#include <strings.h>          /* for bzero() */
	#include <unistd.h>         /* for close() */
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <arpa/inet.h>
	#include <pthread.h>
	#include <stdlib.h>

	#define PORT 1234   /* Port that will be opened */ 
	#define BACKLOG 5   /* Number of allowed connections */ 
	#define MAXDATASIZE 1000  

	void process_cli(int connectfd, sockaddr_in client);
	void savedata_r(char* recvbuf, int len, char* cli_data);
	/* function to be executed by the new thread */
	void* start_routine(void* arg);
	struct  ARG  {
	   int connfd;
	   sockaddr_in client;  
	   };

	/* Thread saft  */
	static pthread_key_t	key;
	static pthread_once_t	once = PTHREAD_ONCE_INIT;
	static void destructor(void *ptr)
	{
	   free(ptr);
	}
	static void getkey_once(void)
	{
	   pthread_key_create(&key, destructor);
	}
	typedef struct DATA_THR{
	   int	 index;						
	};

	main() 
	{ 
	int listenfd, connectfd; /* socket descriptors */ 
	pthread_t  thread;
	ARG *arg;
	struct sockaddr_in server; /* server's address information */ 
	struct sockaddr_in client; /* client's address information */ 
	int sin_size; 

	/* Create TCP socket  */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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
	      perror("accept() error\n"); 
	      exit(1); 
	      } 
	   /* create thread */
	   arg = new ARG;
	   arg->connfd = connectfd;
	   memcpy((void *)&arg->client, &client, sizeof(client));
	   if (pthread_create(&thread, NULL, start_routine, (void*)arg)) {
	      /* handle exception */
	      perror("Pthread_create() error");
	      exit(1);
	      }
	}
	close(listenfd);   /* close listenfd */         
	} 

	void process_cli(int connectfd, sockaddr_in client)
	{
	int num;
	char cli_data[5000];
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

	   /* save user's data */
	   savedata_r(recvbuf,num,cli_data);

	   /* reverse usr's data */
	   for (int i = 0; i < num - 1; i++) {
	      sendbuf[i] = recvbuf[num - i -2];
	      }
	   sendbuf[num - 1] = '\0';

	   send(connectfd,sendbuf,strlen(sendbuf),0); /* send to the client welcome message */ 
	}

	close(connectfd); /*  close connectfd */ 
	printf("Client( %s ) closed connection. User's data: %s\n",cli_name,cli_data);
	}

	void* start_routine(void* arg)
	{
	ARG *info;
	info = (ARG *)arg;

	/* 处理客户请求 */
	process_cli(info->connfd, info->client);

	delete info;
	pthread_exit(NULL);
	}

	void savedata_r(char* recvbuf, int len, char* cli_data)
	{
	DATA_THR* data;

	/* thread_special data */
	pthread_once(&once, getkey_once);
	if ( (data = (DATA_THR *)pthread_getspecific(key)) == NULL) {
	   data = (DATA_THR *)calloc(1, sizeof(DATA_THR));		
	   pthread_setspecific(key, data);
	   data->index = 0; }

	for (int i = 0; i < len - 1; i++) {
	   cli_data[data->index++] = recvbuf[i];
	   }  
	cli_data[data->index] = '\0';       
	}


