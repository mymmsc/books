#include <stdio.h>
#include <vector.h>
#include "pthread.h"
#include "Mysocket.h"
#include "MyThread.h"
#include "TcpServThr.h"




TcpServThr::TcpServThr()
{
    max_connections = MAXCONN;
    ThrSet = new vector<MyThread*>();
}

TcpServThr::TcpServThr(int port, char *hostname): 
   MySocket(AF_INET, SOCK_STREAM,0,port, hostname)
{
   max_connections = MAXCONN;
   ThrSet = new vector<MyThread*>();
}


TcpServThr::TcpServThr(int port, int maxconn, char *hostname): 
   MySocket(AF_INET, SOCK_STREAM,0,port, hostname)
{
   max_connections = maxconn;
   ThrSet = new vector<MyThread*>();
}

TcpServThr::~TcpServThr()
{
   /* Wait until all threads exits */
   WaitAllThr();
   /* free all memory of threads */
   vector<MyThread*>::iterator it = ThrSet->begin();
   while (it != ThrSet->end()) {
      MyThread* thr = (MyThread*)(*it);
      delete thr;
      it++;
      }
   
   delete ThrSet;
   
}

int TcpServThr::Init()
{   
   int opt = SO_REUSEADDR;
   setsockopt(Mysocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
   if (Bind() == -1) return socket_error;
   if (Listen(max_connections) == -1) return socket_error;
   return 0;
}


int TcpServThr::Run()
{
   while (Accept() != -1) {
      /*  Create new thread */
      MyThread *Rthread, *Wthread;
      if(CreateThr(&Rthread,&Wthread) == -1) return -1;
      AddThread(Rthread);
      AddThread(Wthread);
      if (Rthread->Start()) return -1;
      if (Wthread->Start()) return -1;
      }
   return -1;
}

int TcpServThr::CreateThr(MyThread** Rthread, MyThread** Wthread)
{
   printf("Accept Conn\n");
   *Rthread = new Receiver(conn_socket, this);
   *Wthread = new Sender(conn_socket,this);
   return 0;
}
   
void TcpServThr::DealRecv(MyThread* thread)
{
   printf("Receiver is running!\n");
}

void TcpServThr::DealSend(MyThread* thread)
{
   printf("Sender is running!\n");
}

void TcpServThr::AddThread(MyThread* thread)
{   
   if (thread == NULL) return;
   ThrSet->push_back(thread);
}

void TcpServThr::DelThread(MyThread* thread)
{   
   vector<MyThread*>::iterator it = ThrSet->begin();
   while (it != ThrSet->end()) {
      if (((MyThread *)*it) == thread) {
         ThrSet->erase(it);
         break;
         }
      it++;
      }
}

int TcpServThr::WaitAllThr()
{
   vector<MyThread*>::iterator it = ThrSet->begin();
   while (it != ThrSet->end()) {
      MyThread* thr = (MyThread*)(*it);
      pthread_join(thr->getId(),NULL);
      it++;
      }
   return 0;
}
 
 
   
/************************************************************/


class Server : public TcpServThr
{
public:
   Server(int port) : TcpServThr(port) {}
   ~Server() {WaitAllThr();}
   void DealRecv(MyThread* thread) {
      char buf[100];
      int len = recv(conn_socket,buf,100,0);
      buf[len] = '\0';
      printf("Recv:%s\n",buf);
   }
   
   void DealSend(MyThread* thread) {
   sleep(3);
      char* buf = "Good Client";
      Send(conn_socket,buf,strlen(buf),0);
      }
};

/*
main()
{
   Server server(1234);
   
   server.Init();
   server.Run();
}
*/