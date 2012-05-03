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
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "top.h"
#include "display.h"
#include "threads.h"
#include "gstClient.h"
#include "debug.h"

#define GST_TIME    1
#define WIFI_TIME   6

#define TIMEOUT 1



enum gst_states {STOPPED, READY, PAUSED, PLAYING} gst_state;

extern void wifi_scan(void);

int count = 0;

void show_time(void)
{
  char str[6];
  sprintf(str,"%02d%02d",((count / 60) % 99), count % 60);
  display_time(str);
}

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
  struct timespec timeToWait;
  struct timeval now;
  int err, init = FALSE;
  int paused_blink = FALSE;


  //extern char closest_mac[];

  gettimeofday(&now,NULL);
  timeToWait.tv_sec = now.tv_sec+1;
  timeToWait.tv_nsec = 0;


  while(alive && logged_in)
  {

    long long int time = 0;

    pthread_mutex_lock(&timer_Mutex);
    err = pthread_cond_timedwait(&timer_Signal, &timer_Mutex, &timeToWait);
/*    if (err == ETIMEDOUT) {
      printd("timer timed out\n");
    }
    else
    {
      printd("timer called\n");
    }
*/
    pthread_mutex_unlock(&timer_Mutex);

    gettimeofday(&now,NULL);

    if (time = getTimeGst()) //nanoseconds into current stream
    {
      if (init == FALSE)
      {
        long long int nano_offset = (1000000000UL - time);
        timeToWait.tv_sec = now.tv_sec + (nano_offset / 1000000000UL);
        timeToWait.tv_nsec = nano_offset % 1000000000UL;
        init = TRUE;
      }
      else
      {
        timeToWait.tv_sec = now.tv_sec + TIMEOUT;
        timeToWait.tv_nsec = 0;
      }
      count = time / (1000000000UL);
      show_time();
      paused_blink = FALSE;
    }
    else
    {
      timeToWait.tv_sec = now.tv_sec;//+1;
      timeToWait.tv_nsec = 500000000UL;//0;
      //display_time("    ");
      paused_blink = TRUE;
      init = FALSE;
    }

    /*
    if (strlen(closest_mac) != 0)
    {
      printd("timer seeing mac: %s\n",closest_mac);
    }
    */
  }
  pthread_exit(0);
}
