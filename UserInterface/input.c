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
//  printf("buffer: %s\ninput_len: %d\ncursor_pos: %d\n",buffer,input_len,cursor_pos);
    break;

  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(cursor_pos != input_len){  // Send cursor to the end
      cursor_pos = input_len;
    }
    if(input_len < PIN_MAX){
      display_string("PIN too short.",BLOCKING);
    }
    else{
      authentication = check_pin(input_buffer,strlen(input_buffer));
      printf("PIN: %s\n",input_buffer);
      if(authentication == TRUE){
    	printf("Authentication Passed\n");
        display_string("Logged In",NOT_BLOCKING);

    	pthread_mutex_lock(&state_Mutex);
        state = WAITING_LOGGED_IN;
    	pthread_mutex_unlock(&state_Mutex);
    	display_string("Welcome.",NOT_BLOCKING);
    	display_string("Enter Track Number.",NOT_BLOCKING);
    	digits[0] = CURSOR_VAL;
      }
      else{
    	printf("Authentication Failed\n");
        display_string("Invalid PIN.",NOT_BLOCKING);

    	pthread_mutex_lock(&state_Mutex);
        state = WAITING_LOGGED_OUT;
    	pthread_mutex_unlock(&state_Mutex);
    	display_string("Please Enter VALID PIN!",NOT_BLOCKING);
    	digits[0] = CURSOR_VAL;
      }
      //reset_buffer();
    }
    break;

  case CANCEL:
    //reset_buffer();

	pthread_mutex_lock(&state_Mutex);
    state = WAITING_LOGGED_OUT; // Go back to waiting
	pthread_mutex_unlock(&state_Mutex);
	display_string("Enter PIN.",NOT_BLOCKING);
	digits[0] = CURSOR_VAL;
    break;

  case FORWARD:
    move_cursor(RIGHT);
    break;

  case BACK:
    move_cursor(LEFT);
    break;

  case DELETE:
    delete_char();
//  printf("buffer: %s\ninput_len: %d\ncursor_pos: %d\n",buffer,input_len,cursor_pos);
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
//  printf("buffer: %s\ninput_len: %d\ncursor_pos: %d\n",buffer,input_len,cursor_pos);
    break;

  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(cursor_pos != input_len){  // Send cursor to the end
      cursor_pos = input_len;
    }

    if(input_len < 4){
      display_string("Invalid.",BLOCKING);
    }
    else{
      playing = TRUE;//play_track(buffer,strlen(buffer));
      if(playing == TRUE){
        printf("Track number: %s\n",input_buffer);
        display_string(" Track Number ",NOT_BLOCKING);
        display_string(input_buffer,NOT_BLOCKING);
        display_string(" Playing   ",NOT_BLOCKING);
      }
      else{
        display_string("Track not found.",NOT_BLOCKING);
      }

  	  pthread_mutex_lock(&state_Mutex);
      state = WAITING_LOGGED_IN;
  	  pthread_mutex_unlock(&state_Mutex);
  	  display_string("Enter Track Number.",NOT_BLOCKING);
  	  digits[0] = CURSOR_VAL;
  	  //display timing info
      //reset_buffer();
    }
    break;

  case CANCEL:
    //reset_buffer();
	pthread_mutex_lock(&state_Mutex);
    state = WAITING_LOGGED_IN; // Go back to waiting
	pthread_mutex_unlock(&state_Mutex);
	display_string("Enter Track Number.",NOT_BLOCKING);
	digits[0] = CURSOR_VAL;
    break;

  case FORWARD:  // Move the cursor forward 1 digit
    move_cursor(RIGHT);
    break;

  case BACK:
    move_cursor(LEFT);
    break;

  case DELETE:
    delete_char();
//  printf("buffer: %s\ninput_len: %d\ncursor_pos: %d\n",buffer,input_len,cursor_pos);
    break;
  default:
    break;
  }
}
