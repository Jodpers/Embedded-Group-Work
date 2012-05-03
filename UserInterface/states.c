/*
 * @file states.c
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //threads
#include <pthread.h>

#include "top.h"
#include "display.h"
#include "threads.h"
#include "states.h"
#include "debug.h"

int state = INIT_STATE;	// State machine variable
int logged_in = FALSE;  		// Client connected to server

//int state = WAITING_LOGGED_IN; // State machine variable
//int logged_in = TRUE;          // Client connected to server


extern int getFollower();

/**
 *  @brief State Machine - controls the user interface.
 *
 *    This function is the top level state machine.
 *    From here the user navigates the states define
 *    in 'enum ui_states' in states.h.
 *
 *  @param Void.
 *  @return Void.
 */
void * state_machine(void){
  char *emergency="! EMERGENCY !";
  char button_read = FALSE;  // Local snapshot of Global 'Button'
  int state_read;
  int pause = FALSE;

  while(alive){
	  pthread_mutex_lock(&state_Mutex);
	  state_read = state;
	  pthread_mutex_unlock(&state_Mutex);

      if(state_read == EMERGENCY_STATE){ // Display Blocking Message
        reset_buffers();
        display_string(emergency,BLOCKING);
        continue;
      }

      if(state_read != INIT_STATE && state_read != MENU_SELECT){
        pthread_mutex_lock(&button_Mutex);	// Wait for a button press
        pthread_cond_wait(&button_Signal, &button_Mutex);
        button_read = button;
        pthread_mutex_unlock(&button_Mutex);
        if(!alive) continue;                     // Check for kill signal
	    
		  /* Check for Emergency since we've been waiting */
        pthread_mutex_lock(&state_Mutex);
        state_read = state;
        pthread_mutex_unlock(&state_Mutex);

        if(state_read == EMERGENCY_STATE){ // Display Blocking Message
          reset_buffers();
          continue;
        }
      }

	  switch(state_read){
	    case EMERGENCY_STATE:  // Emergency since we've been waiting
	      break;         // Drop out and catch it next round
	      
	    case INIT_STATE:
	      reset_buffers();
	      pthread_mutex_lock(&state_Mutex);
	      state = WAITING_LOGGED_OUT;
	      pthread_mutex_unlock(&state_Mutex);
	      
	    case WAITING_LOGGED_OUT:
	      if(button_read >= '0' && button_read <= '9'){
	        pthread_mutex_lock(&state_Mutex);
	        state = INPUTTING_PIN; // Fall through to next state
	        pthread_mutex_unlock(&state_Mutex);
	      }
	      else{
	        if(button_read == 'C' && input_len == 0){
	          //alive = FALSE; // NEEDS WORK
	        }
	        display_string("Please Enter PIN.",NOT_BLOCKING);
	        break;
	      }
	      
	    case INPUTTING_PIN:
	      if(button_read){
	        input_pin(button_read);  // Sends a snapshot of button pressed
	      }
	      break;
	      
	    case WAITING_LOGGED_IN:
	      switch(button_read){
	      
          case ACCEPT_PLAY:
            pause=~pause;

            if (pause != FALSE)
            {
              pauseGst();
            }
            else
            {
              playGst();
            }
            printf("pause = %d\n",pause);
            break;

          case ENTER_MENU:
            pthread_mutex_lock(&state_Mutex);
            state = MENU_SELECT;
            pthread_mutex_unlock(&state_Mutex);
            break;

          default:
            if (getFollower() == 1)
            {
              printd("For you are a follower, you may not request a track. You have to listen to what i want.\n");
            }
            else
            {
              if(button_read >= '0' && button_read <= '9'){
                pthread_mutex_lock(&state_Mutex);
                state = INPUTTING_TRACK_NUMBER;
                pthread_mutex_unlock(&state_Mutex);
              }
              else{
                display_string("Enter Track Number.",NOT_BLOCKING);
              }
            }
            break;
        }
        break;
	
    case INPUTTING_TRACK_NUMBER:
      if(button_read){
        input_track_number(button_read);  // Sends a snapshot of button
      }
      break;

    case MENU_SELECT:
      display_string("MENU.",BLOCKING);
      menu_select();
      break;

    case SUBMENU_SELECT:
      
    default:
      break;
	  }
  }
  pthread_exit(0);
}

void set_emergency(int in_state)
{
  static int prev_state;

  if (in_state == TRUE)
  {
      prev_state = state;
      
      pthread_mutex_lock(&state_Mutex);
      state = EMERGENCY_STATE;
      pthread_cond_broadcast(&state_Signal);
      pthread_mutex_unlock(&state_Mutex);
  }
  else if (in_state == FALSE)
  {
      pthread_mutex_lock(&state_Mutex);
      state = prev_state;
      pthread_cond_broadcast(&state_Signal);
      pthread_mutex_unlock(&state_Mutex);
      
  }
}
