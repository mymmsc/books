#define ERRORLEN 100

class myCap
{
protected:
	char *device;
	int snaplen;
	int promisc;
	int to_ms;
	int linktype;
	char ebuf[ERRORLEN];
	bpf_u_int32 netp;
	bpf_u_int32 maskp;
	pcap_t *p;
	bpf_program fp;
public:
	myCap();
	myCap(char *device, int snaplen, int promisc, int to_ms);
	virtual ~myCap();

	int init();
	virtual int init(char* rule);
	virtual int capture(char* buf);
	
	pcap_t *open(){
		return pcap_open_live(device, snaplen, promisc, to_ms, ebuf); 
	}
	char *lookupdev(){
		return pcap_lookupdev(ebuf) ;
	}
	int lookupnet( bpf_u_int32 *netp,bpf_u_int32 *maskp){
		return pcap_lookupnet(device, &netp,&maskp, ebuf);
	}
	int dispatch( int cnt, pcap_handler callback, u_char *user){
		return  pcap_dispatch(p, cnt, callback, user);
	}
	int loop(int cnt, pcap_handler callback, u_char *user){
		return pcap_loop(p, cnt, callback, user);
	}
	int compile( char *str, int optimize) {
		return pcap_compile(p, &fp, str, optimize, maskp);
	}
	int setfilter(){
		return pcap_setfilter(p, &fp);
	}
	u_char *next(pcap_pkthdr *h){
		return pcap_next(p, h);
	}
	int datalink(){
		return pcap_datalink(p);
	}
	void close() {
	   pcap_close(p);
	}
	char *geterr() {
	   char str = pcap_geterr(p);
	   strcpy(ebuf,(const  char *)str);
	   return ebuf;
	   }


	void setDevice(char *d) {device = d;}
	void setSnaplen(int s) {snaplen = s;}
	void setPromisc(int p) {promisc = p;}
	void setTimeout(int t) {to_ms = t;}
	char* getDevice() {return device;}
	int getSnaplen() {return snaplen;}
	int getMode() {return promisc;}
	int getTimeout() {return to_ms;}
};

class myCapIP: public myCap
{
public:
	myCapIP(){}
	myCapIP(char *device, int snaplen, int promisc, int to_ms);

	int capture(char* buf);
	int myCapIP::init();
	int getProto(char* buf);
	in_addr getSaddr(char* buf);
	in_addr getDaddr(char* buf);
};
