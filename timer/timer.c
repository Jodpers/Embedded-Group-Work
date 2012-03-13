/*
 ============================================================================
 Name        : Timer.c
 Author      : Joe Herbert
 Version     : 0.1
 Copyright   :
 Description : First attempt at timer function for iGep in uni
 ============================================================================
 */


/* time example */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

/* Thread Names */
pthread_t timer_thread;
pthread_t receiver_thread;

/* Thread Attributes */
pthread_attr_t timer_Attr;
pthread_attr_t receiver_Attr;

/* Mutex and Signals */
pthread_mutex_t timer_Mutex;
pthread_cond_t timer_Signal;

int msg = 0;
void * timer(void){
	time_t start_time, current_time;
	time_t gst_ping, wifi_ping;

	int gst_time = 100;
	int wifi_time = 1000;

	current_time = clock() / (CLOCKS_PER_SEC/1000);
	start_time = current_time;

	gst_ping = current_time + gst_time;
	wifi_ping = current_time + wifi_time;

	while (1)
	{
	  usleep(100000);

	  current_time = clock() / (CLOCKS_PER_SEC/1000);
	  printf("current_time: %d\n",current_time);

    if (current_time >= gst_ping){
      pthread_mutex_lock(&timer_Mutex);
      msg = 1;
			pthread_cond_signal(&timer_Signal); 
			pthread_mutex_unlock(&timer_Mutex);
    	gst_ping = current_time + gst_time;
  	  printf("gst_ping: %d\n",gst_ping);
    }

	  if(current_time >= wifi_ping)
	  {
			/*printf ("%ld \n ", five_second_flag);*/
			pthread_mutex_lock(&timer_Mutex);
      msg = 2;
			pthread_cond_signal(&timer_Signal); 
			pthread_mutex_unlock(&timer_Mutex);
			wifi_ping = current_time + wifi_time;
  	  printf("\twifi_ping: %d\n",wifi_ping);
	  }
	}
	return 0;
}

void * receiver(void){
int local_msg = 0;
	while(1){
		pthread_mutex_lock(&timer_Mutex); 
		pthread_cond_wait(&timer_Signal, &timer_Mutex);
    local_msg = msg;
		pthread_mutex_unlock(&timer_Mutex);
		printf("%d Receiver Awake!\n", local_msg);
	}
	return 0;
}

int main ()
{

  /* Setup Mutex */
  pthread_mutex_init(&timer_Mutex, NULL);
  /* Setup Conditions */
  pthread_cond_init(&timer_Signal, NULL);
  /* Setup Threads */
  pthread_attr_init(&timer_Attr);
  pthread_attr_init(&receiver_Attr);

  if((pthread_create( &timer_thread, &timer_Attr, (void *)timer, NULL)) != 0){
  	  perror("Timer thread failed to start\n");
  	  exit(EXIT_FAILURE);
    }
  if((pthread_create( &receiver_thread, &receiver_Attr, (void *)receiver, NULL)) != 0){
  	  perror("Receiver thread failed to start\n");
  	  exit(EXIT_FAILURE);
    }


  while(1){
	  /* Waiting */
  }

  return 0;
}
