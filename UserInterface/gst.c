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
extern int follower;

//#define MULTI 1


void * gst_control(void){

  int status = 0;
  void *res;

  int port = 5000;

  while(alive && logged_in)
  {
    pthread_t gst_thread;
    pthread_attr_t gst_Attr;

    pthread_attr_init(&gst_Attr);
    pthread_attr_setdetachstate(&gst_Attr, PTHREAD_CREATE_JOINABLE);

    //#ifndef MULTI

      if(getFollower == 0)
	{
	  if(pthread_create( &gst_thread, &gst_Attr,
			     (void *)gst, NULL) != 0){
	    perror("gst thread failed to start\n");
	    exit(EXIT_FAILURE);
	  }
	}
    //#else
      else
	{
	  if(pthread_create( &gst_thread, &gst_Attr,
			     (void *)gst_multicast, NULL) != 0){
	    perror("gst_multicast thread failed to start\n");
	    exit(EXIT_FAILURE);
	  }
	}

   //#endif

    /* Destroy the thread attributes object, since it is no longer needed */

    status = pthread_attr_destroy(&gst_Attr);
    if (status != 0)
    {
      errno = status;
      perror("pthread_attr_destroy");
    }


    /* do something */
    //#ifndef MULTI
    //    set_ip_and_port("127.0.0.1",port);
    
    printf("hi port:%d\n",port);
    status = pthread_join(gst_thread, &res);
    
    if (status != 0)
      {
	errno = status;
	perror("pthread_join");
      }
    
    printf("Joined with thread; returned value was %s\n", (char *) res);
    free(res);      /* Free memory allocated by thread */
  
    //#else
    //  set_multicast_ip_and_port("224.0.0.2",port);
    
    /*	while(alive)
	  {
	    char trackTime[12] = {0};
	    sleep(1);
	    getTimeGst(trackTime);
	    printf("track time: %s\n",trackTime);
	  }
	  }*/
    //#endif

    pthread_mutex_lock(&network_Mutex);
    task = PLAY;
    if (continous())
      {
	data[0] = FIN_PLAYLIST_TRACK;
      }
    else
      {
	data[0] = FIN_INDIV_TRACK;
      }
  }
  pthread_exit(0);
}
