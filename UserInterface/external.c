/*
 * external.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */
#include "top.h"

BYTE playing = FALSE;

BYTE check_pin(char * buffer, int buf_len){
  if (buf_len == 4){
	return TRUE;
  }
  else{
	return FALSE;
  }
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
