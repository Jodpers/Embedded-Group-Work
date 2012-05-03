/*
 * @file external.c
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "top.h"
#include "threads.h"
#include "network.h"
#include "debug.h"

extern int continous();
extern char playCode;

BYTE check_pin(char * buffer, int buf_len){

	BYTE valid = FALSE;

	pthread_mutex_lock(&network_Mutex);
	strncpy(data,buffer,buf_len);
	task = PIN;
	pthread_cond_signal(&network_Signal);
	pthread_mutex_unlock(&network_Mutex);

	pthread_mutex_lock(&request_Mutex);
	pthread_cond_wait(&request_Signal, &request_Mutex);
	printd("data[0] = %c\n", data[0]);
	valid = data[0];
	bzero(data,PACKETLEN);
	pthread_mutex_unlock(&request_Mutex);

	return valid - '0';

}

BYTE play_track(char * buffer,int buf_len){

  BYTE valid = FALSE;
  pthread_mutex_lock(&network_Mutex);
  printd("track buffer:%s\nbuffer_len: %d\n %u\n",buffer, buf_len,strlen(buffer));
  strncpy(data,buffer,buf_len);
  data[buf_len] = '\0';
  printd("data after strcpy= %s\n", data);
  task = PLAY;

  if (continous())
    {
      playCode = PLAYLIST;
    }
  else
    {
      playCode = PLAY_TRACK_ONLY;
    }

  pthread_cond_signal(&network_Signal);
  pthread_mutex_unlock(&network_Mutex);
  
  pthread_mutex_lock(&request_Mutex);
  pthread_cond_wait(&request_Signal, &request_Mutex);
  printd("PLAY TRACK VALID?  %c\n", data[0]);
  valid = data[0];
  pthread_mutex_unlock(&request_Mutex);

  return valid - '0';
}
