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

void * timer(void){
	time_t start_time, current_time, five_second_flag, ping;

	start_time = clock() / (CLOCKS_PER_SEC);
	current_time = clock() / (CLOCKS_PER_SEC/1000);
	five_second_flag = current_time - start_time;


	printf ("start: %ld\n current: %ld\n five_sec:%ld \n ", start_time, current_time, five_second_flag);

	ping = current_time - five_second_flag;

	while (1)
	{
	  usleep(100000);

	  current_time = clock() / (CLOCKS_PER_SEC/1000);
	  ping = current_time - five_second_flag;

	  if(ping >= 500)
	  {
			printf ("%ld \n ", five_second_flag);
			pthread_mutex_lock(&timer_Mutex);
			pthread_cond_signal(&timer_Signal); // Signal UI to wake
			pthread_mutex_unlock(&timer_Mutex);
			start_time = clock() / (CLOCKS_PER_SEC);
			five_second_flag = current_time - start_time;
	  }
	}
	return 0;
}

void * receiver(void){
	while(1){
		pthread_mutex_lock(&timer_Mutex); // Wait for a button press
		pthread_cond_wait(&timer_Signal, &timer_Mutex);
		printf("Receiver Awake!\n");
		pthread_mutex_unlock(&timer_Mutex);
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
