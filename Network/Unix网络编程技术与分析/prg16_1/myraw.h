class myRaw
{
public:
   int sock;

   int error;
   myRaw(int protocol =0);
virtual  ~myRaw();
   int send(const void* msg, int msglen,sockaddr* addr,unsigned int len);
   int send(const void* msg, int msglen,char* addr);
   int sendIP(const void* msg,int msglen,sockaddr* to, unsigned int len);
   int receive(void* buf,int buflen,sockaddr* from,int* len);
   int Error() {return error;}
}; 
