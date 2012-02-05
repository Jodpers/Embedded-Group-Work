/*
 * states.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "states.h"

int state = WAITING_LOGGED_OUT;	// State machine variable
int logged_in = FALSE;  		// Client connected to server

void * state_machine(void){
//  int i;
//  int ret;
  char *emergency="! EMERGENCY !";
  char *welcome="Welcome.";
  char *enter_pin="Please Enter PIN.";
  char button_read = FALSE;  // Local snapshot of Global 'Button'

  while(alive){
    if(state == EMERGENCY){
        display_string(emergency,PADDED,NOT_BLOCKING);
      continue;
    }
    switch(state){
      case WAITING_LOGGED_OUT:
        display_string(enter_pin,PADDED,NOT_BLOCKING);
        digits[0] = CURSOR_VAL;  // Set cursor position

        while(!button && alive && state == WAITING_LOGGED_OUT){ // Just Wait
         usleep(SLEEP);
        }
        if(state == EMERGENCY) break; // Get out if there's an emergency
        if(button >= '0' && button <= '9'){
          state = INPUTTING_PIN; // Fall through to next state
        }
        else{
          break;
        }

      case INPUTTING_PIN:
        if(button_read = button){ // Intentional Assignment
          input_pin(button_read);  // Sends a snapshot of button
        }
        cursor_blink();
        break;

      case WAITING_LOGGED_IN:
        display_string(welcome,PADDED,NOT_BLOCKING);
        display_string("Enter Track Number.",PADDED,NOT_BLOCKING);
        digits[0] = CURSOR_VAL;  // Set cursor position

        while(!button && alive && state == WAITING_LOGGED_IN){ // Just Wait
          usleep(SLEEP);
        }
        if(state == EMERGENCY) break; // Get out if there's an emergency
        if(button >= '0' && button <= '9'){
          state = INPUTTING_TRACK_NUMBER; // Fall through to next state
        }
        else if(button == ENTER_MENU){
          state = MENU_SELECT; // Fall through to next state
          break;
        }
        else{
          break;
        }

      case INPUTTING_TRACK_NUMBER:
        if(button_read = button){ // Intentional Assignment
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
