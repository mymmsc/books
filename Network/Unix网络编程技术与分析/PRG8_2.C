// File: prg8_2.c
	#include <stdio.h>       /* standard I/O routines                     */
	#include <pthread.h>     /* pthread functions and data structures     */
	#include <stdlib.h>      /* These are the usual header files */ 
	#include <strings.h>     /* for bzero() */
	#include <unistd.h>      /* for close() */
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <arpa/inet.h>

	#define NUM_HANDLER_THREADS 3  /* number of threads used to service requests */
	#define PORT 1234   /* Port that will be opened */ 
	#define MAXDATASIZE 100   /* Max number of bytes of data */ 

	pthread_mutex_t request_mutex =  PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t list_mutex =  PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t  got_request   = PTHREAD_COND_INITIALIZER;
	int num_requests = 0;   /* number of pending requests, initially none */

	int sockfd; /* socket descriptors */ 
	struct sockaddr_in server; /* server's address information */ 
	struct sockaddr_in client; /* client's address information */ 
	int sin_size; 

	/* format of a single request. */
	struct request {
	   char info[MAXDATASIZE];             /* client's data             */
	   struct request* next;   /* pointer to next request, NULL if none. */
	};
	struct request* requests = NULL;     /* head of linked list of requests. */
	struct request* last_request = NULL; /* pointer to last request.         */

	void add_request(char* info,  pthread_mutex_t* p_mutex, pthread_cond_t*  p_cond_var)
	{
	int rc;                         /* return code of pthreads functions.  */
	struct request* a_request;      /* pointer to newly added request.     */

	/* create structure with new request */
	a_request = (struct request*)malloc(sizeof(struct request));
	if (!a_request) { /* malloc failed? */
	   fprintf(stderr, "add_request: out of memory\n");
	   exit(1);
	   }
	memcpy(a_request->info, info, MAXDATASIZE);
	a_request->next = NULL;

	/* lock the mutex, to assure exclusive access to the list */
	rc = pthread_mutex_lock(p_mutex);

	/* add new request to the end of the list, updating list */
	/* pointers as required */
	if (num_requests == 0) { /* special case - list is empty */
	   requests = a_request;
	   last_request = a_request;
	   }
	else {
	   last_request->next = a_request;
	   last_request = a_request;
	   }

	/* increase total number of pending requests by one. */
	num_requests++;

	/* unlock mutex */
	rc = pthread_mutex_unlock(p_mutex);

	/* signal the condition variable - there's a new request to handle */
	rc = pthread_cond_signal(p_cond_var);
	}

	struct request* get_request(pthread_mutex_t* p_mutex)
	{
	int rc;                         /* return code of pthreads functions.  */
	struct request* a_request;      /* pointer to request.                 */

	/* lock the mutex, to assure exclusive access to the list */
	rc = pthread_mutex_lock(p_mutex);

	if (num_requests > 0) {
	   a_request = requests;
	   requests = a_request->next;
	   if (requests == NULL) { /* this was the last request on the list */
	      last_request = NULL;
	      }
	   /* decrease the total number of pending requests */
	   num_requests--;
	   }
	   else { /* requests list is empty */
	      a_request = NULL;
	      }

	/* unlock mutex */
	rc = pthread_mutex_unlock(p_mutex);

	/* return the request to the caller. */
	return a_request;
	}

	void handle_request(struct request* a_request, int thread_id)
	{
	char msg[MAXDATASIZE+40];

	if (a_request) {
	   printf("Thread '%d' handled request '%s'\n", thread_id, a_request->info);
	   fflush(stdout);
	   sprintf(msg,"Thread '%d' handled your request '%s'\n", thread_id, a_request->info);
 sendto(sockfd,msg,strlen(msg),0,(struct sockaddr *)&client,sin_size); 
	   }
	}

	void* handle_requests_loop(void* data)
	{
	int rc;                         /* return code of pthreads functions.  */
	struct request* a_request;      /* pointer to a request.               */
	int thread_id = *((int*)data);  /* thread identifying number           */

	/* lock the mutex, to access the requests list exclusively. */
	rc = pthread_mutex_lock(&request_mutex);

	while (1) {
	if (num_requests > 0) { /* a request is pending */
	   a_request = get_request(&list_mutex);
	   if (a_request) { /* got a request - handle it and free it */
	       rc = pthread_mutex_unlock(&list_mutex);
	       free(a_request);
	      }
	}
	else {
	   rc = pthread_cond_wait(&got_request, &request_mutex);
	   }
	}
}

 int main(int argc, char* argv[])
 {

 int        thr_id[NUM_HANDLER_THREADS];      /* thread IDs            */
 pthread_t  p_threads[NUM_HANDLER_THREADS];   /* thread's structures   */
 int num;
 char msg[MAXDATASIZE];

 /* create the request-handling threads */
 for (int i=0; i<NUM_HANDLER_THREADS; i++) {
    thr_id[i] = i;
    pthread_create(&p_threads[i], NULL, handle_requests_loop, (void*)&thr_id[i]);
    }

 /* Create UDP socket  */
 if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
 {
	/* handle exception */
	perror("Creating socket failed.");
	exit(1);
 }

 int opt = SO_REUSEADDR;
 setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
 while (1) 
 {
	num = recvfrom(sockfd,msg,MAXDATASIZE,0,(struct sockaddr *)&client,&sin_size);                                             
	if (num < 0){
	   perror("recvfrom error\n"); 
	   exit(1); 
	   } 

	msg[num] = '\0';
	printf("You got a message (%s%) from %s\n",msg,inet_ntoa(client.sin_addr) ); /* prints client's IP */ 

	add_request(msg, &list_mutex, &got_request);

	if (!strcmp(msg,"quit")) break;
 } 

 close(sockfd);   /* close listenfd */         
 return 0;
 }



