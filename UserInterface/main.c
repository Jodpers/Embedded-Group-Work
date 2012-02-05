/*
 * main.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "main.h"

pthread_t keypad_thread;
pthread_t state_machine_thread;

BYTE alive = TRUE;  // Exit condition of while loops
extern char button;


/*------------------------------------------------------------------------------
 * Main
 *------------------------------------------------------------------------------
 */
int main (void) {
  extern void closing_time(void);
  extern void * keypad(void);
  extern void * state_machine(void);
  extern void setup_term();

  int ret;
  int prev_state = state;
//  sighandler_t

  /* error handling - reset LEDs */
  atexit(closing_time);
  signal(SIGINT, (void *)closing_time);
  printf("Press 'x' and 'Enter' to exit\n");
  printf("Press 'e' and 'Enter' to toggle Emergency state\n");
  setup_term();
  ret = pthread_create( &keypad_thread, NULL, (void *)keypad, NULL); /* TODO: Error Checking */
  ret = pthread_create( &state_machine_thread, NULL, (void *)state_machine, NULL); /* TODO: Error Checking */
  while((ret = getchar()) != 'x'){
	if (ret == 'e'){
	  if (state == EMERGENCY){
        state = prev_state;
      }
      else {
        prev_state = state;
        state = EMERGENCY;
      }
    }
  }
  printf("Closing\n");
  return 0;
}

