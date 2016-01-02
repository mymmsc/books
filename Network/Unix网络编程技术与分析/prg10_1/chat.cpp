/* File : chat.cpp  */
#include <stdio.h>
#include <string.h>
#include <vector.h>
#include "pthread.h"

#include "Mysocket.h"
#include "MyThread.h"
#include "MySync.h"
#include "TcpServThr.h"

#include "Chat.h"

MessageQue::MessageQue(int len)
{
   lastSN = 1;
   queLen = len; 
   queue = new vector<Message*>;
   lock = new MyMutex;
}

MessageQue::~MessageQue()
{
   /* delete all messages */
   vector<Message*>::iterator it = queue->begin();
   while (it != queue->end()) {
      Message* m = (Message*)*it;
      delete m->message;
      delete m;
      it++;
      }
   delete queue;
   delete lock;
}

int MessageQue::Add(int conn, char* m)
{
   if (m == NULL) return -1;
   
   Message *mes;
   mes = new Message; 
   mes->connection = conn;
   mes->message = new char[strlen(m)];
   strcpy(mes->message,(const  char *) m);
   lock->Lock();
   mes->sn = lastSN;
   lastSN++;
   if (queue->size() == queLen) {
      vector<Message*>::iterator it = queue->begin();
      queue->erase(it);
      }
   queue->push_back(mes);
   lock->Unlock();
   return 0;
}

int MessageQue::Get(int conn, long* maxsn, char* m)
{
   int err;
   err = 1;
   lock->Lock();
   vector<Message*>::iterator it = queue->begin();
   while (it != queue->end()) {
      Message* mes = (Message*) *it;
      if ((mes->sn > *maxsn)&&(mes->connection != conn)) {
         strcpy(m,mes->message);
         *maxsn = mes->sn;
         m = m + strlen(mes->message) - 1;
         err = 0;
         }
      it++;
      }
   lock->Unlock();
   return  err;
}   

ChatServer::ChatServer()
{
    SetMaxConn(MAXCONN);
    max_usrs = MAX_USRS;
    msg = new MessageQue(QUE_LEN);
    con = new MyCondition;
}

ChatServer::ChatServer(int port, char *hostname): 
   TcpServThr(port,hostname)
{
   SetMaxConn(MAXCONN);
   max_usrs = MAX_USRS;
   msg = new MessageQue(QUE_LEN);
   con = new MyCondition;
}


ChatServer::ChatServer(int port, int max_conn, int maxusr, int len, char *hostname): 
   TcpServThr(port, max_conn,hostname)
{
   max_usrs = maxusr;
   queLen = len;
   msg = new MessageQue(queLen);
   con = new MyCondition;
}

ChatServer::~ChatServer()
{
   /* Wait until all threads exits */
   WaitAllThr();     // must execute it before ~TcpServThr
   delete msg;
   delete con;
}

   
void ChatServer::DealRecv(MyThread* thread)
{
   printf("Receiver is running!\n");
   char buf[MAX_PACKET_LEN];
   
   int socket = ((Receiver*) thread)->socket;
   int len = recv(socket,buf,MAX_NAME_LEN,0);
   buf[len - 1] = ':';
   buf[len] = '\0';
   printf("%s %d\n",buf,len);

   while (1) {
      int len1 = recv(socket,buf + len,MAX_PACKET_LEN,0);
      if (len1 < 1) break;
      buf[len1 + len] = '\0';
      printf("Recv:%s",buf);
      msg->Add(socket, buf);
      con->wakeAll();
      }
   DelThread(thread);
   
}

void ChatServer::DealSend(MyThread* thread)
{
   char buf[MAX_PACKET_LEN];
   long  maxsn;
   
   maxsn = msg->GetSN() - 1;
   int socket = ((Receiver*) thread)->socket;
   printf("Sender is running!\n");
   while(1) {
      con->wait();
      int err = msg->Get(socket,&maxsn,buf);
      if (err) continue;
      printf("Send:%s",buf);
      Send(socket,buf,strlen(buf),0);
      }
   DelThread(thread);
}


 
 
   
/************************************************************/



main()
{
   ChatServer  chat(1234);
 //  Server::Sender* s;
 //  s = new Server::Sender(1,NULL);
   chat.Init();
   chat.Run();
}
