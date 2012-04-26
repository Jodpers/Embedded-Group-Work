


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

enum gst_state_t { STOPPED, PLAYING, PAUSED, RESETTING };

extern int alive;

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

extern enum gst_state_t gst_state;
extern void display_time(char * in);
extern void clear_time(void);

int count = 0;

#define BLOCKING        1
#define NOT_BLOCKING    0

void show_time(void)
{
  char str[6];
  sprintf(str,"%02d%02d",((count / 60) % 99), count % 60);
  display_time(str);
}

void * timer(void)
{
//  time_t start_time, current_time, pause_time;
  struct timespec timeToWait;
  struct timeval now, start_time, pause_time;
  int err;
  int paused_blink = FALSE;
  int prev_state = gst_state;
  
  //extern char closest_mac[];
  
  gettimeofday(&now,NULL);
  timeToWait.tv_sec = now.tv_sec+1;
  timeToWait.tv_nsec = 0;


  while(alive)
  {

    
    pthread_mutex_lock(&timer_Mutex);
    err = pthread_cond_timedwait(&timer_Signal, &timer_Mutex, &timeToWait);
    if (err == ETIMEDOUT) {
      printf("timed out\n");
    }
    else
    {
      printf("called\n");
    }
    
    pthread_mutex_unlock(&timer_Mutex);

    gettimeofday(&now,NULL);
        
    switch(gst_state)
    {
      case STOPPED:
        printf("gst_stopped\n");
        
        timeToWait.tv_sec = now.tv_sec+3;
        timeToWait.tv_nsec = 0;
        break;
        
      case PLAYING:
        printf("gst_playing\n");

        switch(prev_state)
        {
          case STOPPED:
            {
              start_time.tv_sec = now.tv_sec;
              start_time.tv_usec = now.tv_usec;
              timeToWait.tv_sec = now.tv_sec+1;
              timeToWait.tv_nsec = (now.tv_usec * 1000UL) % (1000000000UL);          
            }
            break;
            
          case PAUSED:
            {
              start_time.tv_sec += (now.tv_sec - pause_time.tv_sec);
              start_time.tv_usec += (now.tv_usec - pause_time.tv_usec);
              timeToWait.tv_sec = now.tv_sec + ((start_time.tv_usec + pause_time.tv_usec) / (1000000UL));
              timeToWait.tv_nsec = (start_time.tv_usec * 1000UL) % (1000000000UL);
            }
            break;
            
          default:
            timeToWait.tv_sec = now.tv_sec+1;
            break;
        }
        count = now.tv_sec - start_time.tv_sec;
        
        show_time();

//        timeToWait.tv_nsec = (start_time.tv_usec * 1000UL);
        printf("now usec %ld\n",now.tv_usec);
        printf("timeToWait.tv_nsec %ld\n",timeToWait.tv_nsec);
        break;
        
      case PAUSED:
        printf("gst_paused\n");
        
        if (prev_state == PLAYING)
        {
          pause_time = now;
          count = now.tv_sec - start_time.tv_sec;
          paused_blink = FALSE;
          timeToWait.tv_sec = now.tv_sec+1;
          timeToWait.tv_nsec = (now.tv_usec * 1000UL) % (1000000000UL);
        }
        
        printf("paused_blink = %d\n",paused_blink);
        if (paused_blink == TRUE)
        {
          show_time();
          paused_blink = FALSE;
        }
        else
        {
          display_time("    ");
          paused_blink = TRUE;
        }
        timeToWait.tv_sec = now.tv_sec+1;
        break;
        
      case RESETTING:
        printf("gst_resetting\n");
        timeToWait.tv_sec = now.tv_sec+1;
        gst_state = STOPPED;
        count = 0;
        clear_time();
        break;
        
      default:
        timeToWait.tv_sec = now.tv_sec+1;
        break;
    }
    prev_state = gst_state;
    
    /*
    if (strlen(closest_mac) != 0)
    {
      printf("timer seeing mac: %s\n",closest_mac);
    }
    */
  }
}


int setup_timer()
//int main(void)
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
/*  if((pthread_create( &wifi_thread, &wifi_Attr, (void *)wifi_scan, NULL)) != 0){
  	  perror("Wifi thread failed to start\n");
  	  exit(EXIT_FAILURE);
    }
 */   
/*  
  pthread_join(timer_thread, NULL);
  printf("timer dead\n");
  pthread_join(wifi_thread, NULL);
  printf("wifi dead\n");
*/
  return 0;
}

void close_timer(void)
{
  pthread_join(timer_thread, NULL);
}
