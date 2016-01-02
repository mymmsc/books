

/*************************************************
class name : Thread_interface
function: It is base class of all threads. used by class Mythread.
****************************************************/

class Thread_interface
{
public:
virtual    void run(){}
};
  
/*******************************************************
class name : MyThread
function: Create thread and run it.
***************************************************************/
      

class MyThread : public Thread_interface
{
   Thread_interface*  worker;
   int error;
   pthread_t id;
   
static    void* run(void*);   
   
public:
   MyThread(Thread_interface& w);
   MyThread() {worker = this;}
     
   int Start();
   void Exit(void *value_ptr) { ::pthread_exit(value_ptr);}
   int Join(pthread_t thread, void **value_ptr) { error = ::pthread_join(thread, value_ptr); return error;}
   int Cancel(pthread_t target_thread){ error = ::pthread_cancel(target_thread); return error;}
   int Detach() {error = ::pthread_detach(id); return error;}
   int Detach(pthread_t thread) {error = ::pthread_detach(thread); return error;}   
   pthread_t getId() {return id;}
   int Error(){return error;}
};

