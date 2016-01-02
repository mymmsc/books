// don't use static condition variable

/* File : mythread.cpp */
#include <iostream.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "mythread.h"
#include "MySync.h"

// MyThread
MyThread::MyThread(Thread_interface& w)
{
   worker = &w;
   error =0;
}

void* MyThread::run(void* w)
{
   ((Thread_interface *)w)->run();
}

int MyThread::Start()
{
   error = pthread_create(&id,NULL,run,(void *)worker);
   return error;
}

/*************************************/




/* Creating a thread:
       1) Create a class extends from MyThread class;
       2) implement void run(){};
       3) run Start();
 */


/* Creating a thread:
       1) Create a class extends from Thread_interface class;
       2) implement void run(){};
       3) Create a object of Thread. (named thread)
       4) thread.Start();
 */


