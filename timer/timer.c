


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h> 

#include "top.h"

#define GST_TIME    1
#define WIFI_TIME   6

int alive = TRUE;

/* Thread Names */
pthread_t timer_thread;
pthread_t wifi_thread;

/* Thread Attributes */
pthread_attr_t timer_Attr;
pthread_attr_t wifi_Attr;

/* Mutex and Signals */
pthread_mutex_t timer_Mutex;
pthread_cond_t timer_Signal;

/* Wifi Signals */
pthread_mutex_t wifi_Mutex;
pthread_cond_t wifi_Signal;


/*void * timer(void){
	time_t start_time, current_time;

  char time_string [10];

	time_t gst_ping;
	time_t wifi_ping;

	time(&current_time);
	start_time = current_time;

	gst_ping = current_time + GST_TIME;
	wifi_ping = current_time + WIFI_TIME;

	while (alive)
	{
	  usleep(10000);

	  time(&current_time);
	  
	  printf("* ");//current_time: %ld\n",current_time);

    if (current_time >= gst_ping){
      pthread_mutex_lock(&timer_Mutex);
			pthread_cond_signal(&timer_Signal); 
			pthread_mutex_unlock(&timer_Mutex);
    	gst_ping = current_time + GST_TIME;
  	  printf("gst_ping: %ld\n",gst_ping);
    }

	  if(current_time >= wifi_ping)
	  {
			pthread_mutex_lock(&wifi_Mutex);
			pthread_cond_signal(&wifi_Signal); 
			pthread_mutex_unlock(&wifi_Mutex);
			wifi_ping = current_time + WIFI_TIME;
  	  printf("\twifi_ping: %ld\n",wifi_ping);
	  }
	}
	return 0;
}
*/

#define TIMEOUT 1

extern void * wifi_scan(void);

void * timer(void)
{
  struct timespec timeToWait;
  struct timeval now;
  int err;
  
  extern char closest_mac[];
  
  gettimeofday(&now,NULL);

  while(alive)
  {
    gettimeofday(&now,NULL);  
    timeToWait.tv_sec = now.tv_sec + TIMEOUT;
    timeToWait.tv_nsec = now.tv_usec+(1500UL*1500UL);
    pthread_mutex_lock(&wifi_Mutex);
    err = pthread_cond_timedwait(&wifi_Signal, &wifi_Mutex, &timeToWait);
    if (err == ETIMEDOUT) {
      printf("timed out\n");
    }
    else
    {
      printf("called\n");
    }
    
    if (strlen(closest_mac) != 0)
    {
      printf("timer seeing mac: %s\n",closest_mac);
    }
    pthread_mutex_unlock(&wifi_Mutex);
  }
}


//int setup_timer()
int main(void)
{

  /* Setup Mutex */
  pthread_mutex_init(&timer_Mutex, NULL);
  pthread_mutex_init(&wifi_Mutex, NULL);
  /* Setup Conditions */
  pthread_cond_init(&timer_Signal, NULL);
  pthread_cond_init(&wifi_Signal, NULL);
  /* Setup Threads */
  pthread_attr_init(&timer_Attr);
  pthread_attr_init(&wifi_Attr);

  pthread_attr_setdetachstate(&timer_Attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setdetachstate(&wifi_Attr, PTHREAD_CREATE_JOINABLE);    

  if((pthread_create( &timer_thread, &timer_Attr, (void *)timer, NULL)) != 0){
  	  perror("Timer thread failed to start\n");
  	  exit(EXIT_FAILURE);
    }
  if((pthread_create( &wifi_thread, &wifi_Attr, (void *)wifi_scan, NULL)) != 0){
  	  perror("Wifi thread failed to start\n");
  	  exit(EXIT_FAILURE);
    }
    
  
  pthread_join(timer_thread, NULL);
  printf("timer dead\n");
  pthread_join(wifi_thread, NULL);
  printf("wifi dead\n");

  return 0;
}
