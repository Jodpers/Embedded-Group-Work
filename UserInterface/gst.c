/*
 * gst.c
 *
 *  Created on: 22 Feb 2012
 *      Author: Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "top.h"
#include "threads.h"
#include "display.h"
#include "gstClient.h"
#include "network.h"


extern int gst_playing;
extern int getFollower();
extern int playCode;

extern int continous();
//#define MULTI 1


void * gst_control(void){

  int status = 0;
  void *res;

  while(alive && logged_in)
  {
    pthread_t gst_thread;
    pthread_attr_t gst_Attr;

    pthread_attr_init(&gst_Attr);
    pthread_attr_setdetachstate(&gst_Attr, PTHREAD_CREATE_JOINABLE);

    if(getFollower() != -1)
	  {
	    if(pthread_create( &gst_thread, &gst_Attr,
			       (void *)gst, NULL) != 0){
	      perror("gst thread failed to start\n");
	      exit(EXIT_FAILURE);
	    }
	  }
	  
    /* Destroy the thread attributes object, since it is no longer needed */

    status = pthread_attr_destroy(&gst_Attr);
    if (status != 0)
    {
      errno = status;
      perror("pthread_attr_destroy");
    }

    status = pthread_join(gst_thread, &res);
    
    if (status != 0)
    {
      errno = status;
      perror("pthread_join");
    }
    if (res == (void *)-1)
    {
      printf("gst thread exited abnormally!\n");
    }
    else
    {
      printf("Joined with thread; returned value was %s\n", (char *) res);
      free(res);      /* Free memory allocated by thread */
    }

    pthread_mutex_lock(&network_Mutex);
    task = PLAY;
    if (continous())
      {
      	playCode = FIN_PLAYLIST_TRACK;
      }
    else
      {
      	playCode = FIN_INDIV_TRACK;
      }
    pthread_mutex_unlock(&network_Mutex);
  }
  pthread_exit(0);
}
