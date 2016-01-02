/* File : mysync.cpp */

#include <iostream.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "MySync.h"

MyMutex::MyMutex()
{
   error = pthread_mutex_init(&a_mutex,  NULL);
}
 
MyMutex::~MyMutex()
{
   error = pthread_mutex_destroy(&a_mutex);
}



MyCondition::MyCondition()
{
	error = pthread_cond_init(&got_request,NULL);
	error += pthread_mutex_init(&a_mutex,NULL);

}

MyCondition::~MyCondition() 
{
	error = pthread_mutex_destroy(&a_mutex);
	int rc = pthread_cond_destroy(&got_request);
	while (rc == EBUSY) { /* some thread is still waiting on this condition variable */
   	sleep(1);
   	}
	error = error + rc;
}	
   

int MyCondition::wait(int second = 0)
{
	struct timeval  now;            /* time when we started waiting        */
	struct timespec timeout;        /* timeout value for the wait function */
	int             done;           /* are we done waiting?                */

	/* first, lock the mutex */
	int rc = pthread_mutex_lock(&a_mutex);
	if (rc) { /* an error has occurred */
   	perror("pthread_mutex_lock");
  	pthread_exit(NULL);
	}

	if (second) {
	   /* get current time */ 
	   gettimeofday(&now,NULL);
	   /* prepare timeout value */
	   timeout.tv_sec = now.tv_sec + second;
	   timeout.tv_nsec = now.tv_usec * 1000; /* timeval uses microseconds.         */
                                      /* timespec uses nanoseconds.         */
                                      /* 1 nanosecond = 1000 micro seconds. */
	
	  error = pthread_cond_timedwait(&got_request, &a_mutex, &timeout);
	}else error = pthread_cond_wait(&got_request, &a_mutex); 
	
	/* finally, unlock the mutex */
	pthread_mutex_unlock(&a_mutex);

    	switch(error) {
        	case 0:  /* we were awakened due to the cond. variable being signaled */
                 /* the mutex was now locked again by pthread_cond_timedwait. */
            	/* do your stuff here... */
            	return 0;
      
        case ETIMEDOUT: /* our time is up */
            	return 1;
       
        default:        /* some error occurred (e.g. we got a Unix signal) */
            return 2;      /* break this switch, but re-do the while loop.   */
       }
}

int MyCondition::wake()
{
	error = pthread_cond_signal(&got_request);
	return error;
}

int MyCondition::wakeAll()
{
	error = pthread_cond_broadcast(&got_request);
	return error;
}

