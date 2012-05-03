/*
 * @file input.c
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  //threads
#include <pthread.h>

#include "top.h"
#include "states.h"
#include "threads.h"
#include "display.h"
#include "debug.h"
#include "gstClient.h"
#include "network.h"

BYTE playing = FALSE;
BYTE authentication = FALSE;
char temp_string[BUFFER_SIZE] = {0};

BYTE pause = FALSE;

BYTE play_track(char * buffer,int buf_len);


int already_logged_in = FALSE;

/*------------------------------------------------------------------------------
 * User Interface State Machines
 *------------------------------------------------------------------------------
 */

/**
 *  @brief Inputting Pin Number.
 *
 *    This function allows the user to input a Pin number
 *    using the keypad and using the 7-Segment display for output.
 *
 *    As numbers are entered the cursor moves position. The user
 *    can navigate the cursor using 'F' and 'B' buttons.
 *    The 'D' button will delete a character from the display
 *    and the cursor position shall update accordingly.
 *
 *  @param [in] button_read used to determine button pressed.
 *  @return Void.
 */
/*  INPUTTING_PIN */

void input_pin(char button_read){
  extern BYTE check_pin(char *, int);

  switch(button_read){

  case ENTER_MENU:
  case ACCEPT_PLAY:
    if(input_len < PIN_MAX){
      display_string("PIN too short.",BLOCKING);
    }
    else{
      /* Send pin to the server and block until response received */
      authentication = check_pin(input_buffer,strlen(input_buffer));
      printd("PIN: %s\n",input_buffer);

      reset_buffers();

      if(authentication == TRUE){
        printd("Authentication Passed\n");

    	  pthread_mutex_lock(&state_Mutex);
    	  logged_in = TRUE;
        state = WAITING_LOGGED_IN;
      	pthread_mutex_unlock(&state_Mutex);
      	
      	/* Launch threads on log in */
      	if (already_logged_in == FALSE)
	      {
	        start_logged_in_threads();
	        already_logged_in = TRUE;
	      }

      	display_string("Welcome.",BLOCKING);
      	display_string("Enter Track Number.",NOT_BLOCKING);
      }
      else{
      	printd("Authentication Failed\n");
        pthread_mutex_lock(&state_Mutex);
      	logged_in = FALSE;
      	already_logged_in = FALSE;
        state = WAITING_LOGGED_OUT;
        pthread_mutex_unlock(&state_Mutex);
      	display_string("Please Enter VALID PIN!",NOT_BLOCKING);


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
    if (button_read >= '0' && button_read <= '9')   /* 0-9*/
      insert_char(button_read);
    break;
  }
}

/**
 *  @brief Inputting Track Number.
 *
 *    This function allows the user to input a Track number
 *    using the keypad and using the 7-Segment display for output.
 *
 *    As numbers are entered the cursor moves position. The user
 *    can navigate the cursor using 'F' and 'B' buttons.
 *    The 'D' button will delete a character from the display
 *    and the cursor position shall update accordingly.
 *
 *  @param [in] button_read used to determine button pressed.
 *  @return Void.
 */
void input_track_number(char button_read){

  switch(button_read){


  case ENTER_MENU:
    if (input_len == 0)
    {
      pthread_mutex_lock(&state_Mutex);
      state = MENU_SELECT;
      pthread_mutex_unlock(&state_Mutex);
      break;
    }


  case ACCEPT_PLAY:
    printf("input_len: %d\n",input_len);
    if(input_len < TRACK_MIN || input_len >= TRACK_MAX){
      display_string("Invalid.",NOT_BLOCKING);
    }
    else{
      printd("input buffer to play track: %s\n", input_buffer);
      playing = play_track(input_buffer,strlen(input_buffer));
      printd("playing:%c\n", playing);
      if(playing == TRUE){
        printd("Track number: %s\n",input_buffer);
        sprintf(temp_string,"Track Number %s Playing",input_buffer);
        reset_buffers();
        display_string(temp_string,BLOCKING);
	      //playGst();
      }
      else if(playing == END_OF_PLAYLIST)
	    {
	      display_string("End of playlist.",BLOCKING); //to james, only 50 chars!!
	    }
      else
	    {
	      display_string("Track not found.",BLOCKING);
	    }

  	  pthread_mutex_lock(&state_Mutex);
      state = WAITING_LOGGED_IN;
  	  pthread_mutex_unlock(&state_Mutex);

      reset_buffers();
  	  //display timing info
    }
    break;


  case DELETE:
    delete_char();
    if (input_len)
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

  default:
    if (button_read >= '0' && button_read <= '9')   /* 0-9*/
      insert_char(button_read);
    break;
  }
}
