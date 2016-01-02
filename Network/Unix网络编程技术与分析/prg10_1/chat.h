/* File : chat.h  */

/* Define constant */
   const int MAX_USRS = 10;
   const int QUE_LEN = 10;
   const  int MAX_PACKET_LEN = 1000;
   const  int MAX_NAME_LEN = 100;

struct Message 
{
   long sn;
   int connection;
   char* message;
};

/****************************************************
class name : MessageQue
Function: A queue for message
******************************************************/
class MessageQue
{
   long lastSN;
   int queLen;  // The length of the queue
   vector<Message*> *queue;
   MyMutex* lock;
public:
   MessageQue(int len);
   ~MessageQue();
   
   int Add(int conn, char* m);
   int Get(int conn, long* maxsn, char* m);
   long GetSN() {return lastSN;}
     void SetSN(int sn) { lastSN = sn;}
     int GetQueLen() { return queLen;}
     void SetQueLen(int l) { queLen = l;}
};

/****************************************************
class name : ChatServer
Function: chat with multiple users
******************************************************/
class ChatServer : public TcpServThr
{
protected:
   int max_usrs;
   int queLen;
   MessageQue *msg;
   MyCondition *con;
public:
   ChatServer();
   ChatServer(int port, char *hostname = NULL); 
   ChatServer(int port, int max_conn, int maxusers, int len, char *hostname = NULL);
   virtual ~ChatServer();
   
   void SetMaxUser(int num) {max_usrs = num;}
   int GetMaxUser() { return max_usrs;}
   void DealRecv(MyThread* thread);
   void DealSend(MyThread* thread);
};
