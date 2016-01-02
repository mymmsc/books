// File: prg15_1.c
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <net/route.h>
 #include <sys/param.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>

 void cp_rtaddrs(int, sockaddr_in *, sockaddr_in **);

 #define	BUFLEN	(sizeof(rt_msghdr) + 512)
 #define	SEQ	1234

 int main(int argc, char **argv)
 {
 int			sockfd;
 char			*buf;
 pid_t			pid;
 ssize_t			n;
 struct rt_msghdr	*rtm;
 struct sockaddr_in	*sa, *rt_info[RTA_NUMBITS];
 struct sockaddr_in	*sin;

 if (argc != 2) {
   printf("usage: %s <IPaddress>\n", argv[0]);
   exit(0);
   }

 sockfd = socket(AF_ROUTE, SOCK_RAW, 0);	/* need superuser privileges */
 if (sockfd == -1) {
   perror("socket: ");
   exit(1);
   }

 buf = (char * ) calloc(1, BUFLEN);	/* and initialized to 0 */

 rtm = (rt_msghdr *) buf;
 rtm->rtm_msglen = sizeof(rt_msghdr) + sizeof(sockaddr_in);
 rtm->rtm_version = RTM_VERSION;
 rtm->rtm_type = RTM_GET;
 rtm->rtm_addrs = RTA_DST;
 rtm->rtm_pid = pid = getpid();
 rtm->rtm_seq = SEQ;

 sin = (sockaddr_in *) (rtm + 1);
 sin->sin_family = AF_INET;
 inet_pton(AF_INET, argv[1], &sin->sin_addr);

 write(sockfd, rtm, rtm->rtm_msglen);

 do {
   n = read(sockfd, rtm, BUFLEN);
 } while (rtm->rtm_type != RTM_GET || rtm->rtm_seq != SEQ || rtm->rtm_pid != pid);

 sa = (sockaddr_in *) (rtm + 1);
 cp_rtaddrs(rtm->rtm_addrs, sa, rt_info);

 if ( (sa = rt_info[RTA_DST]) != NULL)
   printf("dest: %s\n", inet_ntoa(sa->sin_addr));

 if ( (sa = rt_info[RTA_GATEWAY]) != NULL)
   printf("gateway: %s\n", inet_ntoa(sa->sin_addr));

 if ( (sa = rt_info[RTA_NETMASK]) != NULL)
   printf("netmask: %s\n", inet_ntoa(sa->sin_addr));

 exit(0);
 }

 void  cp_rtaddrs(int addrs, struct sockaddr_in *sa, struct sockaddr_in **rt_info)
 {
 int		i;

 for (i = 0; i < RTA_NUMBITS; i++) {
   if (addrs & (1 << i)) {
   rt_info[i] = sa;
   sa++;
   } else
   rt_info[i] = NULL;
 }
 }



