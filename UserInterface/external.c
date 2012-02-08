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

BYTE playing = FALSE;

BYTE check_pin(char * buffer, int buf_len){

	BYTE valid = FALSE;

	pthread_mutex_lock(&network_Mutex);
	strncpy(data,buffer,buf_len);
	task = PIN;
	pthread_cond_signal(&network_Signal);
	pthread_mutex_unlock(&network_Mutex);

	pthread_mutex_lock(&request_Mutex);
	pthread_cond_wait(&request_Signal, &request_Mutex);
	valid = data[0];
	pthread_mutex_unlock(&request_Mutex);

	return valid;

}

BYTE play_track(char * buffer,int buf_len){
	if (buf_len == 4){
		return TRUE;
	  }
	  else{
		return FALSE;
	  }
}

char * check_time(BYTE flashing){
  char * time = "04:20";
  return time;
}

char * closest_MAC(void){

}
