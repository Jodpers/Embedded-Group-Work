/*
 * @file external.c
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "top.h"
#include "threads.h"
#include "network.h"
#include "debug.h"

BYTE check_pin(char * buffer, int buf_len){

	BYTE valid = FALSE;

	pthread_mutex_lock(&network_Mutex);
	strncpy(data,buffer,buf_len);
	task = PIN;
	pthread_cond_signal(&network_Signal);
	pthread_mutex_unlock(&network_Mutex);

	pthread_mutex_lock(&request_Mutex);
	pthread_cond_wait(&request_Signal, &request_Mutex);
	printd("data[0] = %c", data[0]);
	valid = data[0];
	pthread_mutex_unlock(&request_Mutex);

	return valid;

}

BYTE play_track(char * buffer,int buf_len){
  /*	if (buf_len == 4){
		return TRUE;
			  }
	  else{
		return FALSE;
		}*/
  BYTE valid = FALSE;
  pthread_mutex_lock(&network_Mutex);
  printd("track buffer:%s\n, buffer_len: %d\n %d",buffer, buf_len,strlen(buffer));
  strncpy(data,buffer,buf_len);
  data[buf_len] = '\0';
  printd("data after strcpy= %s\n", data);
  task = PLAY;
  pthread_cond_signal(&network_Signal);
  pthread_mutex_unlock(&network_Mutex);
  
  pthread_mutex_lock(&request_Mutex);
  pthread_cond_wait(&request_Signal, &request_Mutex);
  printd("data = %s", data);
  valid = data[0];
  pthread_mutex_unlock(&request_Mutex);
  return valid;	
}

char * closest_MAC(void){

  return;
}
