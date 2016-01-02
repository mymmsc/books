class myRoute
{
	int sockfd;
	int buflen;
	int seq;
	pid_t	pid;
	char* buf;
	rt_msghdr	*rtm;
	sockaddr_in	*rt_info[RTA_NUMBITS];
	int error;

	int cp_rtaddrs(int, sockaddr_in *, sockaddr_in **);
public:
	myRoute();
	myRoute(int buflen1, int seq1);
	~myRoute();

	int Get(char *ipaddr);
	sockadd_in getDst();
	sockadd_in getGateway();
	sockadd_in getNetmask();

	int Error(){return error;}
};
