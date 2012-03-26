/*
 * external.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include <string.h>
#include "top.h"
#include "threads.h"
#include "network.h"

BYTE check_pin(char * buffer, int buf_len){

	BYTE valid = FALSE;

	pthread_mutex_lock(&network_Mutex);
	strncpy(data,buffer,buf_len);
	task = PIN;
	pthread_cond_signal(&network_Signal);
	pthread_mutex_unlock(&network_Mutex);

	pthread_mutex_lock(&request_Mutex);
	pthread_cond_wait(&request_Signal, &request_Mutex);
	printf("data[0] = %c", data[0]);
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
  printf("track buffer:%s\n, buffer_len: %d\n %d",buffer, buf_len,strlen(buffer));
  printf("data before bzero:%s\n", data);
  printf("data after bzero= %s\n", data);
  strncpy(data,buffer,buf_len);
  data[buf_len] = 0;
  printf("data after strcpy= %s\n", data);
  task = PLAY;
  pthread_cond_signal(&network_Signal);
  pthread_mutex_unlock(&network_Mutex);
  
  pthread_mutex_lock(&request_Mutex);
  pthread_cond_wait(&request_Signal, &request_Mutex);
  printf("data = %s", data);
  valid = data[0];
  pthread_mutex_unlock(&request_Mutex);
  return valid;	
}

char * check_time(BYTE flashing){
  char * time = "04:20";
  return time;
}

char * closest_MAC(void){

  return;
}
