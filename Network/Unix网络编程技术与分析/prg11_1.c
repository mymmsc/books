/* File : prg11_1.cpp  */
#include <stdio.h>
#include <string.h>
#include <vector.h>
#include "pthread.h"
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "Mysocket.h"
#include "MyThread.h"
#include "MySync.h"
#include "TcpServThr.h"
#include "Chat.h"


class ChatServerd : public ChatServer
{
   int MsgPriority;  
   void closeall(int fd);

public:
   ChatServerd(int port, char *hostname = NULL);
   ChatServerd(int port, int max_conn, int maxusr, int len, char *hostname);
   
   int daemon(char* name,int facility = 0);
   void DealRecv(MyThread* thread);
   void DealSend(MyThread* thread);
   void SetPriority(int p) {MsgPriority = p;}
   int GetPriority() {return MsgPriority;}
};
   
ChatServerd::ChatServerd(int port, char *hostname): 
   ChatServer(port,hostname)
{
   MsgPriority = LOG_NOTICE|LOG_LOCAL0;
}

ChatServerd::ChatServerd(int port, int max_conn, int maxusr, int len, char *hostname): 
   ChatServer(port,max_conn, maxusr, len, hostname)
{

MsgPriority = LOG_NOTICE|LOG_LOCAL0;
}
   
void ChatServerd::DealRecv(MyThread* thread)
{
   syslog(MsgPriority,"Receiver is running!\n");
   char buf[MAX_PACKET_LEN];
   
   int socket = ((Receiver*) thread)->socket;
   int len = recv(socket,buf,MAX_NAME_LEN,0);
   buf[len - 1] = ':';
   buf[len] = '\0';
   syslog(MsgPriority,"%s %d\n",buf,len);

   while (1) {
      int len1 = recv(socket,buf + len,MAX_PACKET_LEN,0);
      if (len1 < 1) break;
      buf[len1 + len] = '\0';
      syslog(MsgPriority,"Recv:%s",buf);
      msg->Add(socket, buf);
      con->wakeAll();
      }
   DelThread(thread);
   
}

void ChatServerd::DealSend(MyThread* thread)
{
   char buf[MAX_PACKET_LEN];
   long  maxsn;
   
   maxsn = msg->GetSN() - 1;
   int socket = ((Receiver*) thread)->socket;
   syslog(MsgPriority,"Sender is running!\n");
   while(1) {
      con->wait();
      int err = msg->Get(socket,&maxsn,buf);
      if (err) continue;
      syslog(MsgPriority,"Send:%s",buf);
      Send(socket,buf,strlen(buf),0);
      }
   DelThread(thread);
}

int ChatServerd::daemon(char* name,int facility)
{
    switch (fork())
    {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);          // exit the original process 
    }

    if (setsid() < 0)               
      return -1;

    signal(SIGHUP,SIG_IGN);  // ignore SIGHUP

    switch (fork())
    {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);
    }

    chdir("/");


    closeall(0);
    open("/dev/null",O_RDWR);
    dup(0); 
    dup(0);


   openlog(name,LOG_PID,facility);

   
    return 0;
}

void ChatServerd::closeall(int fd)
{
    int fdlimit = sysconf(_SC_OPEN_MAX);

    while (fd < fdlimit)
{
      if (fd != Mysocket) close(fd);
      fd++;
      }
}


/*************************************************************/
int main(int argc, char** argv)
{
  FILE* file = fopen("chatd.conf","r");
  if (file == NULL) {
     perror("chatd.conf:");
     exit(1);
     }
  
  char  input[100];
  int port = 1234;
  int max_conn = 20;
  int maxusr = 20;
  int len = 20;
  char *hostname = "127.0.0.1";
  
  while(1) {
     char* s = fgets(input,100,file);
     if (s == NULL) break;
     
     if (!strncasecmp(input,"PORT",4)) port = atoi(strrchr(input,' '));
     if (!strncasecmp(input,"MAX_USRS",8)) maxusr = atoi(strrchr(input,' '));
     if (!strncasecmp(input,"QUELEN",6)) len = atoi(strrchr(input,' '));
     if (!strncasecmp(input,"MAX_CONN",8)) max_conn = atoi(strrchr(input,' '));
     if (!strncasecmp(input,"HOSTNAME",8)) hostname = strrchr(input,' ');
     } 
  
  ChatServerd  chatd(port, max_conn,maxusr, len, hostname);

  if (chatd.daemon(argv[0]) < 0) {
        perror("daemon");
        exit(2);
        }

   chatd.Init(); 
   chatd.Run();

   return 0;
}

