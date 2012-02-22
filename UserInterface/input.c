/*
 * input.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "top.h"
#include "states.h"
#include "threads.h"

BYTE authentication = FALSE;
char temp_string[BUFFER_SIZE] = {0};

BYTE playing = FALSE;

/*------------------------------------------------------------------------------
 * User Interface State Machines
 *------------------------------------------------------------------------------
 */

/*  INPUTTING_PIN */

void input_pin(char button_read){
  extern BYTE check_pin(char *, int);

  switch(button_read){
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':

    insert_char(button_read);
    break;

  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(input_len < PIN_MAX){
      display_string("PIN too short.",BLOCKING);
    }
    else{
      authentication = check_pin(input_buffer,strlen(input_buffer));
      printf("PIN: %s\n",input_buffer);

      reset_buffers();

      if(authentication == TRUE){
    	printf("Authentication Passed\n");

    	pthread_mutex_lock(&state_Mutex);
    	logged_in = TRUE;
        state = WAITING_LOGGED_IN;
    	pthread_mutex_unlock(&state_Mutex);

    	display_string("Welcome.",BLOCKING);
    	display_string("Enter Track Number.",NOT_BLOCKING);
      }
      else{
    	printf("Authentication Failed\n");
        pthread_mutex_lock(&state_Mutex);
    	logged_in = FALSE;
        pthread_mutex_unlock(&state_Mutex);
    	display_string("Please Enter VALID PIN!",NOT_BLOCKING);


        pthread_mutex_lock(&state_Mutex);
        state = WAITING_LOGGED_OUT;
        pthread_mutex_unlock(&state_Mutex);
      }
    }
    break;

  case CANCEL:
    reset_buffers();

	pthread_mutex_lock(&state_Mutex);
    state = WAITING_LOGGED_OUT; // Go back to waiting
	pthread_mutex_unlock(&state_Mutex);
	display_string("Enter PIN.",NOT_BLOCKING);
    break;

  case FORWARD:
    move_cursor(RIGHT);
    break;

  case BACK:
    move_cursor(LEFT);
    break;

  case DELETE:
    delete_char();
    break;
  default:
    break;
  }
}

/*  INPUTTING_TRACK_NUMBER */

void input_track_number(char button_read){

  switch(button_read){
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    insert_char(button_read);
    break;

  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(input_len < TRACK_MIN || input_len >= TRACK_MAX){
      display_string("Invalid.",NOT_BLOCKING);
    }
    else{
      playing = TRUE;//play_track(buffer,strlen(buffer));
      if(playing == TRUE){
        printf("Track number: %s\n",input_buffer);
        sprintf(temp_string,"Track Number %s Playing",input_buffer);
        reset_buffers();
        display_string(temp_string,BLOCKING);
      }
      else{
        display_string("Track not found.",BLOCKING);
      }

  	  pthread_mutex_lock(&state_Mutex);
      state = WAITING_LOGGED_IN;
  	  pthread_mutex_unlock(&state_Mutex);

      reset_buffers();
  	  //display timing info
    }
    break;

  case CANCEL:
    reset_buffers();

	pthread_mutex_lock(&state_Mutex);
    state = WAITING_LOGGED_IN; // Go back to waiting
	pthread_mutex_unlock(&state_Mutex);
	display_string("Enter Track Number.",NOT_BLOCKING);
    break;

  case FORWARD:  // Move the cursor forward 1 digit
    move_cursor(RIGHT);
    break;

  case BACK:
    move_cursor(LEFT);
    break;

  case DELETE:
    delete_char();
    break;
  default:
    break;
  }
}
