/*
 * main.c
 *
 *  created on: 5 feb 2012
 *      author: pete hemery
 */

#include "main.h"

BYTE alive = TRUE;  // exit condition of while loops

/*------------------------------------------------------------------------------
 * main
 *------------------------------------------------------------------------------
 */
int main (void) {
  int ret;
  int prev_state = state;
//  sighandler_t


  printf("press 'e' and 'enter' to toggle emergency state\n");
  printf("press 'x' and 'enter' to exit\n");

  ret = networkSetup();
  if (ret != 0 )
    {
      printd("Socket failed to init, error no: %d\n", ret); 
    }
  else
    {
      /* error handling - reset leds */
      atexit(closing_time);
      signal(SIGINT, (void *)closing_time);
      
      setup_term();
      start_threads();
      
      while((ret = getchar()) != 'x' && alive){
	if (ret == 'e'){
	  pthread_mutex_lock(&state_Mutex);
	  if (state == EMERGENCY){
	    state = prev_state;
	    if(state == SUBMENU_SELECT){
	      state = MENU_SELECT;
	    }
	  }
	  else {
	    prev_state = state;
	    state = EMERGENCY;
	  }

	  pthread_mutex_lock(&button_Mutex);  // unlock state machine
	  pthread_cond_signal(&button_Signal);
	  pthread_mutex_unlock(&button_Mutex);
	  
	  pthread_cond_signal(&state_Signal);
	  pthread_mutex_unlock(&state_Mutex);
	}
      }
    }
  return 0;
}
