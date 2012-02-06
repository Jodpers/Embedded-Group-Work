/*
 * states.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "states.h"

int state = INIT_STATE;	// State machine variable
int logged_in = FALSE;  		// Client connected to server

void * state_machine(void){
  char *emergency="! EMERGENCY !";
  char button_read = FALSE;  // Local snapshot of Global 'Button'
  int state_read;

  pthread_mutex_lock(&state_Mutex);
  //pthread_cond_wait(&state_Signal, &state_Mutex);
  state_read = state;
  pthread_mutex_unlock(&state_Mutex);

  while(alive){

	pthread_mutex_lock(&state_Mutex);
	state_read = state;
	pthread_mutex_unlock(&state_Mutex);

    if(state_read == EMERGENCY){ // Continue within the switch may not work
    	/*pthread_mutex_lock(&state_Mutex);
    	//pthread_cond_wait(&state_Signal, &state_Mutex);
    	state_read = state;
        pthread_mutex_unlock(&state_Mutex);*/
        display_string(emergency,PADDED,NOT_BLOCKING);
      continue;
    }

    if(state_read != INIT_STATE){
      pthread_mutex_lock(&button_Mutex);	// Wait for a button press
      pthread_cond_wait(&button_Signal, &button_Mutex);
      button_read = button;
      pthread_mutex_unlock(&button_Mutex);

      pthread_mutex_lock(&state_Mutex); 	// Check for Emergency since we've been waiting
      state_read = state;
      pthread_mutex_unlock(&state_Mutex);
    }

	switch(state_read){
      case EMERGENCY:	// Emergency since we've been waiting
    	display_string(emergency,PADDED,NOT_BLOCKING);
//    	continue;
    	break;

      case INIT_STATE:
   	    pthread_mutex_lock(&state_Mutex);
    	state = WAITING_LOGGED_OUT;
    	pthread_mutex_unlock(&state_Mutex);

      case WAITING_LOGGED_OUT:
        digits[0] = CURSOR_VAL;  // Set cursor position

        if(button_read >= '0' && button_read <= '9'){
          pthread_mutex_lock(&state_Mutex);
          state = INPUTTING_PIN; // Fall through to next state
      	  pthread_mutex_unlock(&state_Mutex);
        }
        else{
          display_string("Please Enter PIN.",PADDED,NOT_BLOCKING);
          digits[0] = CURSOR_VAL;
          break;
        }

      case INPUTTING_PIN:
        if(button_read){
          input_pin(button_read);  // Sends a snapshot of button pressed
        }
        cursor_blink();//TODO move to somewhere not blocked?
        break;

      case WAITING_LOGGED_IN:
        digits[0] = CURSOR_VAL;  // Set cursor position
        if(button_read >= '0' && button_read <= '9'){
          pthread_mutex_lock(&state_Mutex);
          state = INPUTTING_TRACK_NUMBER; // Fall through to next state
      	  pthread_mutex_unlock(&state_Mutex);
        }
        else if(button_read == ENTER_MENU){
          pthread_mutex_lock(&state_Mutex);
          state = MENU_SELECT; // Don't know how to jump 2 states below
      	  pthread_mutex_unlock(&state_Mutex);
          break; // Go Round Again         T_T
        }
        else{
          display_string("Enter Track Number.",PADDED,NOT_BLOCKING);
          digits[0] = CURSOR_VAL;
          break;
        }

      case INPUTTING_TRACK_NUMBER:
        if(button_read){
          input_track_number(button_read);  // Sends a snapshot of button
        }
        cursor_blink();
        break;

      case MENU_SELECT:
        display_string("MENU.",PADDED,NOT_BLOCKING);
        menu_select();
        break;

      default:
        break;
    }
    delay();
  }
  return 0;
}
