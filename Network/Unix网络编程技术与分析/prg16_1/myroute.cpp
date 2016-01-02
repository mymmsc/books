#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/route.h>		
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "myroute.h"

myRoute::myRoute()
{
	buflen = 200;
	seq = 1234;
    buf = (char * ) calloc(1, buflen);	/* and initialized to 0 */
	pid = getpid();

	sockfd = socket(AF_ROUTE, SOCK_RAW, 0);	/* need superuser privileges */
	if (sockfd == -1) {
	    error = 1;
	    }
}

myRoute::myRoute(int buflen1, int seq1)
{
	buflen = buflen1;
	seq = seq1;
    buf = (char * ) calloc(1, buflen);	/* and initialized to 0 */
	pid = getpid();

	sockfd = socket(AF_ROUTE, SOCK_RAW, 0);	/* need superuser privileges */
	if (sockfd == -1) {
	    error = 1;
	    }
}

myRoute::~myRoute() 
{
	close(sockfd);
	free(buf);
}

int myRoute::Get(char *ipaddr)
{
	sockaddr_in	*sa;
	sockaddr_in	*sin;

	rtm = (rt_msghdr *) buf;
	rtm->rtm_msglen = sizeof(rt_msghdr) + sizeof(sockaddr_in);
	rtm->rtm_version = RTM_VERSION;
	rtm->rtm_type = RTM_GET;
	rtm->rtm_addrs = RTA_DST;
	rtm->rtm_pid = pid;
	rtm->rtm_seq = SEQ;

	sin = (sockaddr_in *) (rtm + 1);
	sin->sin_family = AF_INET;
    inet_pton(AF_INET, ipaddr, &sin->sin_addr);

	write(sockfd, rtm, rtm->rtm_msglen);

	do {
		n = read(sockfd, rtm, buflen);
	} while (rtm->rtm_type != RTM_GET || rtm->rtm_seq != SEQ || rtm->rtm_pid != pid);

	sa = (sockaddr_in *) (rtm + 1);
	return cp_rtaddrs(rtm->rtm_addrs, sa, rt_info);
}

sockadd_in myRoute::getDst()
{
	if ( (sa = rt_info[RTA_DST]) != NULL)
		return sa->sin_addr;
	return NULL;
}

sockadd_in myRoute::getGateway()
{
	if ( (sa = rt_info[RTA_GATEWAY]) != NULL)
		return sa->sin_addr;
	return NULL;
}

sockadd_in myRoute::getNetmask()
{
	if ( (sa = rt_info[RTA_NETMASK]) != NULL)
		return sa->sin_addr;
	return NULL;
}

int  myRoute::cp_rtaddrs(int addrs, struct sockaddr_in *sa, struct sockaddr_in **rt)
{
	int		i, num;

	num = 0;
	for (i = 0; i < RTA_NUMBITS; i++) {
		if (addrs & (1 << i)) {
			rt[i] = sa;
			sa++;
			num++;
		} else
			rt[i] = NULL;
	}
	return num;
}

