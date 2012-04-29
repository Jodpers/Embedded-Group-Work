#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define FALSE 0
#define TRUE 1

//#define MULTI


enum {STOPPED, PLAYING, EOS, ERROR};

#ifndef MULTI
extern void * gst(void);
extern void set_ip_and_port(char *ip_in, int port_in);
#else
extern void * gst_multicast(void);
extern void set_multicast_ip_and_port(char *ip_in, int port_in);
#endif

extern int gst_playing;

void main(void)
{
  int status = 0;
  int port = 4444;
  void *res;
  
  
  while(1)
  {
    pthread_t gst_thread;
    pthread_attr_t gst_Attr;
      
    pthread_attr_init(&gst_Attr);
    pthread_attr_setdetachstate(&gst_Attr, PTHREAD_CREATE_JOINABLE);
    
#ifndef MULTI
    if(pthread_create( &gst_thread, &gst_Attr, 
    											(void *)gst, NULL) != 0){
      perror("gst_multicast thread failed to start\n");
      exit(EXIT_FAILURE);
    }
#else
   if(pthread_create( &gst_thread, &gst_Attr, 
    											(void *)gst_multicast, NULL) != 0){
      perror("gst_multicast thread failed to start\n");
      exit(EXIT_FAILURE);
    }
#endif
    
    /* Destroy the thread attributes object, since it is no longer needed */

    status = pthread_attr_destroy(&gst_Attr);
    if (status != 0)
    {
      errno = status;
      perror("pthread_attr_destroy");
    }
    
    
    /* do something */
#ifndef MULTI
    set_ip_and_port("127.0.0.1",port);
    //getTimeGst();
#else
    set_multicast_ip_and_port("224.0.0.2",port);
#endif
  
    printf("hi port:%d\n",port);
    status = pthread_join(gst_thread, &res);
    
    if (status != 0)
    {
      errno = status;
      perror("pthread_join");
    }
    
    printf("Joined with thread; returned value was %s\n", (char *) res);
    free(res);      /* Free memory allocated by thread */
  }
  return ;
}

