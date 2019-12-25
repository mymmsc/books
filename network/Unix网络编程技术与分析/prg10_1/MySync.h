/****************************************************
class name : MyMutex
Function: support mutex
******************************************************/

class MyMutex 
{
  pthread_mutex_t a_mutex;
  int error;
   
public:
   MyMutex(); 
   ~MyMutex();
   
   int Lock() { error = pthread_mutex_lock(&a_mutex);return error;}
   int Trylock() { error = pthread_mutex_trylock(&a_mutex); return error;}
   int Unlock() {error = pthread_mutex_unlock(&a_mutex); return error;}
   int Error() {return error;}
};


/****************************************************
class name : MyCondition
Function: support MyCondition variable
******************************************************/

class MyCondition 
{
  pthread_mutex_t a_mutex;
   
  pthread_cond_t got_request;
   
  int error;
   
public:
   MyCondition(); 
   ~MyCondition();
   
   int wait(int second = 0);    // wait until signal by other thread
   int wake();           // wake a thread waiting for this condition
   int wakeAll();     // wake all threads waiting for this conditoin
   int Error() {return error;}
};
