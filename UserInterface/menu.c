/*
 * @file menu.c
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //threads
#include <pthread.h>

#include "top.h"
#include "menu.h"
#include "states.h"
#include "threads.h"
#include "display.h"
#include "debug.h"


/**
 *  @brief Menu selection routine.
 *
 *    This function is called by pressing ENTER_MENU (button E)
 *    while state machine is in 'WAITING_LOGGED_IN' state and no
 *    track number has been input.
 *
 *    Options are navigated using the FORWARD (F) and BACK (B) buttons
 *    or pressing the relevant option number on the keypad.
 *
 *    Options are selected by pressing ENTER_MENU (E) or ACCEPT (A).
 *
 *  @param Void.
 *  @return Void.
 */

#define MENU_STR_NUM  7


/*  MENU SELECTION */
void menu_select(void){
  enum {zero, VOLUME, LOCATION, SCROLL, PLAYBACK, LOG_OUT, EXIT_PROG};
  int choice = 1;
  char button_read = 0;
  int state_read;

  set_menu(TRUE); // in display.c
  show_choice(choice);

  pthread_mutex_lock(&state_Mutex);
  state_read = state; // Initialise the copy of the current state.
  pthread_mutex_unlock(&state_Mutex);

  while(alive && (state_read == MENU_SELECT)){

    pthread_mutex_lock(&button_Mutex);
    pthread_cond_wait(&button_Signal, &button_Mutex); // Wait for press
    button_read = button;               // Read the button pressed
    pthread_mutex_unlock(&button_Mutex);

    /* If button has been pressed, but an emergency packet has arrived,
     * check for it and respond by breaking out of the loop.
     */
	  pthread_mutex_lock(&state_Mutex);
	  state_read = state;
	  pthread_mutex_unlock(&state_Mutex);
    
    if(state_read == EMERGENCY || alive == FALSE){
      set_menu(FALSE); // in display.c
      break; // Get out if there's an emergency
    }

/* Button has been pressed. Now what? */
    switch(button_read){
      case '0' + VOLUME: // Volume
      case '0' + LOCATION: // Location
      case '0' + SCROLL: // Scroll Speed Settings
      case '0' + PLAYBACK: // Playback Settings
      case '0' + LOG_OUT: // Log Out
      case '0' + EXIT_PROG: // Log Out
        choice = button_read - '0';
        show_choice(choice);
        break;
        
      case ACCEPT_PLAY:
        break;

      case ENTER_MENU:
  	    switch(choice){
		  case VOLUME:
		    pthread_mutex_lock(&state_Mutex);
            state = SUBMENU_SELECT;
            state_read = state;
            pthread_mutex_unlock(&state_Mutex);
            printd("Volume Selected\n");
	           volume(); 
            show_choice(choice); // After return, display correct choice again
		    break;

		  case LOCATION:
		    //Request Location Information
	        //wifi_scan();
		    break;

          case SCROLL:
            break;

          case PLAYBACK:
            break;

		  case LOG_OUT:
		    {
          set_menu(FALSE);
          reset_buffers();
  	      display_string(" Goodbye ",BLOCKING);
	
		      pthread_mutex_lock(&state_Mutex);
	        logged_in = FALSE;
	        already_logged_in = FALSE;
          state = INIT_STATE;
          state_read = state;
          pthread_mutex_unlock(&state_Mutex);
  
          printd("Logging Out\n");
        }
		    break;

		  case EXIT_PROG:
		    printf("Exiting\n");
		    exit(1);
          default:

	        break;
	    }
	    printd("Choice: %d\n",choice);
        break;

      case CANCEL:
        pthread_mutex_lock(&state_Mutex);
        state = WAITING_LOGGED_IN; // Go back to waiting
        state_read = state;
        pthread_mutex_unlock(&state_Mutex);
        set_menu(FALSE);
        break;

      case FORWARD:
        if(++choice == MENU_STR_NUM){
          choice = 1;
        }
        show_choice(choice);
        break;

      case BACK:
        if(--choice == 0){
          choice = MENU_STR_NUM - 1;
        }
        show_choice(choice);
        break;

      case DELETE:
      default:
        break;
    }
  }
}

/**
 *  @brief Displays the selected choice from the menu on the 7-Segment
 *			LED Display.
 *
 *    Takes the choice from the parameter,
 *    sends the relevant string to the display_string routine.
 *
 *  @param [in] choice integer value of current choice option to display.
 *  @return Void.
 */
void show_choice(int choice){
  char *menu_strings[MENU_STR_NUM] = {
    "",
    "1.Volume.",
    "2.Location.",
    "3.Scroll Settings.",
    "4.Playback Settings.",
    "5.Log out.",
    "6.Exit Program"
  };

  if (choice > 0 && choice < MENU_STR_NUM){
    display_string(menu_strings[choice],NOT_BLOCKING);
  }

  return;
}
