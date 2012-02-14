/*
 * menu.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "menu.h"

/*  MENU SELECTION */
void menu_select(void){
  int choice = 1;
  char button_read = 0;
  int state_read;

  show_choice(choice);

  pthread_mutex_lock(&state_Mutex);
  state_read = state;
  pthread_mutex_unlock(&state_Mutex);

  while(alive && state_read == MENU_SELECT){

    pthread_mutex_lock(&button_Mutex);
    pthread_cond_wait(&button_Signal, &button_Mutex);
    button_read = button;
    pthread_mutex_unlock(&button_Mutex);

	pthread_mutex_lock(&state_Mutex);
	state_read = state;
	pthread_mutex_unlock(&state_Mutex);
    if(state_read == EMERGENCY || alive == FALSE) break; // Get out if there's an emergency


    switch(button_read){
      case '1':
      case '2':
      case '3':
      case '4':
        choice = button_read - '0';
        show_choice(choice);
        break;
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0':
        break;
      case ACCEPT_PLAY:
        break;

      case ENTER_MENU:
  	    switch(choice){
		  case '1':
		    break;
		  case '2':
	        //wifi_scan();
		    break;
		  case '3':
		    break;
		  case '4':
		    break;
          default:
	        break;
	    }
	    printf("Choice: %d\n",choice);
	    break;

      case CANCEL:
        //reset_buffer();
        pthread_mutex_lock(&state_Mutex);
        state = WAITING_LOGGED_IN; // Go back to waiting
        pthread_mutex_unlock(&state_Mutex);
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

void show_choice(int choice){
  int i;
  char *menu_strings[MENU_STR_NUM] = {
    "",
    "1.Volume.",
    "2.Location.",
    "3.Settings.",
    "4.Log out."
  };

  //reset_buffer();
  display_string("",NOT_BLOCKING);
  display_string(menu_strings[choice],NOT_BLOCKING);

  for(i=0;i<5;i++){
    display_char(menu_strings[choice][i]);
  }
  return;
}
