/*
 ============================================================================
 Name        : Timer.c
 Author      : Joe Herbert
 Version     : 0.1
 Copyright   :
 Description : First attempt at timer function for iGep in uni
 ============================================================================

 Modified by Pete Hemery on 23/03/2012 to integrate with client code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "top.h"
#include "threads.h"

extern void wifi_scan(void);

/**
 *  @brief Timer Thread - used to aid time specific tasks.
 *
 *    Continuously updates the time information on the 7 Seg display
 *    and checks for the closest Wifi beacon (every 10 seconds).
 *
 *  @param Void.
 *  @return Void.
 */
void * timer(void){
    time_t start_time, current_time;
    time_t gst_ping, wifi_ping;

    int gst_time = 100;
    int wifi_time = 1000;

    current_time = clock() / (CLOCKS_PER_SEC/1000);
    start_time = current_time;

    gst_ping = current_time + gst_time;
    wifi_ping = current_time + wifi_time;

    while (alive)
    {
      usleep(100000);

      current_time = clock() / (CLOCKS_PER_SEC/1000);
//      printf("current_time: %ld\n",current_time);

    if (current_time >= gst_ping){
      pthread_mutex_lock(&timer_Mutex);
      pthread_cond_signal(&timer_Signal);
      pthread_mutex_unlock(&timer_Mutex);
      gst_ping = current_time + gst_time;
//      printf("gst_ping: %ld\n",gst_ping);
    }

      if(current_time >= wifi_ping)
      {
        pthread_mutex_lock(&timer_Mutex);

        pthread_cond_signal(&timer_Signal);
        pthread_mutex_unlock(&timer_Mutex);
        wifi_ping = current_time + wifi_time;
//        printf("\twifi_ping: %ld\n",wifi_ping);
      }
    }
    return 0;
}
