/*
 * input.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "top.h"
#include "states.h"

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
    insert_char(button_read,PIN);
//  printf("buffer: %s\nbuffer_cnt: %d\ncur_pos: %d\n",buffer,buffer_cnt,cur_pos);
    break;

  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(cur_pos != buffer_cnt){  // Send cursor to the end
      cur_pos = buffer_cnt;
    }
    if(buffer_cnt < PIN_MAX){
      display_string("PIN too short.",PADDED,BLOCKING);
    }
    else{
      authentication = check_pin(buffer,strlen(buffer));
      printf("PIN: %s\n",buffer);
      if(authentication == TRUE){
    	printf("Authentication Passed\n");
        display_string("Logged In",PADDED,NOT_BLOCKING);
        state = WAITING_LOGGED_IN;
      }
      else{
    	printf("Authentication Failed\n");
        display_string("Invalid PIN.",PADDED,NOT_BLOCKING);
        state = WAITING_LOGGED_OUT;
      }
      reset_buffer();
    }
    break;

  case CANCEL:
    reset_buffer();
    state = WAITING_LOGGED_OUT; // Go back to waiting
    break;

  case FORWARD:  // Move the cursor forward 1 digit
    if(buffer_cnt){
      if(++cur_pos>buffer_cnt){ // Scroll as far as 1 digit past last input
        cur_pos = buffer_cnt;
      }
    }
    cursor_blink(); // Update Cursor Position
//    printf("cursor_position: %d\n",cur_pos);
    break;

  case BACK:
    if(--cur_pos<=0){
      cur_pos=0;
    }
    cursor_blink();
//    printf("cursor_position: %d\n",cur_pos);
    break;

  case DELETE:
    delete_char();
//  printf("buffer: %s\nbuffer_cnt: %d\ncur_pos: %d\n",buffer,buffer_cnt,cur_pos);
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
    insert_char(button_read,TRACK);
//  printf("buffer: %s\nbuffer_cnt: %d\ncur_pos: %d\n",buffer,buffer_cnt,cur_pos);
    break;

  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(cur_pos != buffer_cnt){  // Send cursor to the end
      cur_pos = buffer_cnt;
    }

    if(buffer_cnt < 4){
      display_string("Invalid.",PADDED,BLOCKING);
    }
    else{
      playing = TRUE;//play_track(buffer,strlen(buffer));
      if(playing == TRUE){
        printf("Track number: %s\n",buffer);
        display_string(" Track Number ",NOT_PADDED,NOT_BLOCKING);
        display_string(buffer,NOT_PADDED,NOT_BLOCKING);
        display_string(" Playing   ",PADDED,NOT_BLOCKING);
      }
      else{
        display_string("Track not found.",PADDED,NOT_BLOCKING);
      }
      state = WAITING_LOGGED_IN;
      reset_buffer();
    }
    break;

  case CANCEL:
    reset_buffer();
    state = WAITING_LOGGED_IN; // Go back to waiting
    break;

  case FORWARD:  // Move the cursor forward 1 digit
    if(buffer_cnt){
      if(++cur_pos>buffer_cnt){ // Scroll as far as 1 digit past last input
        cur_pos = buffer_cnt;
      }
    }
    cursor_blink(); // Update Cursor Position
//    printf("cursor_position: %d\n",cur_pos);
    break;

  case BACK:
    if(--cur_pos<=0){
      cur_pos=0;
    }
    cursor_blink();
//    printf("cursor_position: %d\n",cur_pos);
    break;

  case DELETE:
    delete_char();
//  printf("buffer: %s\nbuffer_cnt: %d\ncur_pos: %d\n",buffer,buffer_cnt,cur_pos);
    break;
  default:
    break;
  }
}
