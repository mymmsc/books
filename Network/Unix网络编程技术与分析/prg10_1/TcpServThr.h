/****************************************************
class name : TcpServThr
Function: support TCP Server with multithread
******************************************************/

class TcpServThr : public MySocket
{
   int max_connections;
   vector<MyThread*>* ThrSet;
   
public:
   TcpServThr();
   TcpServThr(int port, char *hostname = NULL); 
   TcpServThr(int port, int maxconn, char *hostname = NULL);
   virtual ~TcpServThr();
   
   void SetMaxConn(int num) {max_connections = num;}
   int GetMaxConn() {return max_connections;}
   int Init();
   int Run(); 
   
   virtual void DealRecv(MyThread* thread);
   virtual void DealSend(MyThread* thread);

protected:
    int CreateThr(MyThread** Rthread, MyThread** Wthread);
    void AddThread(MyThread* thread);
    void DelThread(MyThread* thread);
    int WaitAllThr();
    
    class Receiver : public MyThread
    {
    public:
       int socket;
       TcpServThr* server;
    
       Receiver(int connsocket, TcpServThr* serv) {
          socket = connsocket;
          server = serv;
          }
       void run() { server->DealRecv(this); }
    };

    class Sender : public MyThread
    {
    public:
       int socket;
       TcpServThr* server;
    
       Sender(int connsocket, TcpServThr* serv) {
          socket = connsocket;
          server = serv;
          }
       void run() { server->DealSend(this); }
    };

};
