#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define FALSE 0
#define TRUE 1

//#define MULTI 1


enum {STOPPED, PLAYING, EOS, ERROR};

#ifndef MULTI
extern void * gst(void);
extern void set_ip_and_port(char *ip_in, int port_in);
#else
extern void * gst_multicast(void);
extern void set_multicast_ip_and_port(char *ip_in, int port_in);
#endif

extern int getTimeGst(char * trackTime);

extern void killGst();
extern void playGst();
extern void pauseGst();

extern int gst_playing;

int alive = TRUE;

void * check_time_thread(void)
{
  int count = 0;
  while(alive)
  {
      char trackTime[12] = {0};
      int time = 0;
      sleep(1);
      time = getTimeGst(trackTime);
      if (time != 0)
      {
        printf("seconds: %d\ttrack time: %s",time,trackTime);
     
        switch(++count)
        {
          case 5:
            pauseGst();
            break;
          case 7:
            playGst();
            break;
          case 10:
            killGst();
            count=0;
          default:
            break;
      }
    }
  }
}

void main(void)
{
  int status = 0;
  int port = 4444;
  void *res;
  
  pthread_t time_thread;
  pthread_attr_t time_Attr;

  pthread_attr_init(&time_Attr);
  pthread_attr_setdetachstate(&time_Attr, PTHREAD_CREATE_JOINABLE);
    
  if(pthread_create( &time_thread, &time_Attr, 
  											(void *)check_time_thread, NULL) != 0){
    perror("gst thread failed to start\n");
    exit(EXIT_FAILURE);
  }
  status = pthread_attr_destroy(&time_Attr);
  if (status != 0)
  {
    errno = status;
    perror("pthread_attr_destroy");
  }
  
  while(alive)
  {
    pthread_t gst_thread;
    pthread_attr_t gst_Attr;
      
    pthread_attr_init(&gst_Attr);
    pthread_attr_setdetachstate(&gst_Attr, PTHREAD_CREATE_JOINABLE);
    
#ifndef MULTI
    if(pthread_create( &gst_thread, &gst_Attr, 
    											(void *)gst, NULL) != 0){
      perror("gst thread failed to start\n");
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
    printf("hi port:%d\n",port);
    status = pthread_join(gst_thread, &res);
    
    if (status != 0)
    {
      errno = status;
      perror("pthread_join");
    }
    
    printf("Joined with thread; returned value was %s\n", (char *) res);
    free(res);      /* Free memory allocated by thread */
#else
    set_multicast_ip_and_port("224.0.0.2",port);
    
    while(alive)
    {
      char trackTime[12] = {0};
      sleep(1);
      getTimeGst(trackTime);
      printf("track time: %s\n",trackTime);
    }
#endif

  }
  return ;
}

