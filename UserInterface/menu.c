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

  show_choice(choice);

  while(alive && state == MENU_SELECT){
    while(!button && alive && state == MENU_SELECT){ // Just Wait
      usleep(SLEEP);
    }
    if(state == EMERGENCY) break; // Get out if there's an emergency
    button_read = button;

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
      break;

    case CANCEL:
      reset_buffer();
      state = WAITING_LOGGED_IN; // Go back to waiting
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

  reset_buffer();
  display_string(NULL,NOT_PADDED,NOT_BLOCKING);
  display_string(menu_strings[choice],PADDED,NOT_BLOCKING);

  for(i=0;i<5;i++){
    display_char(menu_strings[choice][i],NOT_BLOCKING);
  }
  return;
}
