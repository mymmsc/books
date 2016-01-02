// File: prg1_1.c
 #include <stdio.h>		
 #include <sys/types.h>		
 #include <netinet/in.h>		
 #include <sys/socket.h>		
 #include <netdb.h>		
 #include <unistd.h>
 #include <string.h>

 #define	HOSTNAMELEN	40	
 #define	BUFLEN		1024	
 #define	PORT		13	/* port of daytime server   */

 int main(int argc, char *argv[])
 {
     int			rc;            
     int            sockfd;             /* socket descriptor */
     char		buf[BUFLEN+1]; 
     char*		pc;            
     struct sockaddr_in	sa;            
     struct hostent*     hen; 	      

     if (argc < 2) {
 	     fprintf(stderr, "Missing host name\n");
 	     exit (1);
     }

     /* Address resolution */
     hen = gethostbyname(argv[1]);
     if (!hen) {
   	perror("couldn't resolve host name");
     }

     memset(&sa, 0, sizeof(sa));
     sa.sin_family = AF_INET;
     sa.sin_port = htons(PORT);
     memcpy(&sa.sin_addr.s_addr, hen->h_addr_list[0], hen->h_length);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
 	     perror("socket() failed");
     }

     rc = connect(sockfd, (struct sockaddr *)&sa, sizeof(sa));
     if (rc) {
 	     perror("connect() failed");
     }

     /*  reading the socket	*/
     pc = buf;
     while (rc = read(sockfd, pc, BUFLEN - (pc-buf))) {
         pc += rc;
     }

     /* close the socket */
     close(sockfd);
     /* pad a null character to the end of the result */
     *pc = '\0';
     /* print the result */
     printf("Time: %s", buf);
     /* and terminate */
     return 0;
 }


