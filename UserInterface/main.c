/*
 * @file main.c
 *
 *  created on 5 Feb 2012
 *     @author Pete Hemery
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>  //threads
#include <pthread.h>

#include "top.h"
#include "main.h"
#include "threads.h"
#include "states.h"
#include "network.h"
#include "pio_term.h"
#include "debug.h"

BYTE alive = TRUE;  // exit condition of while loops

/**
 *  @brief Main.
 *
 *    This function starts everything.
 *    It initialises the Network and Keypad.
 *    If either of these fail then the program exits with an error.
 *
 *    If not, then threads are initialised and the
 *    function 'closing_time' is associated with a signal to exit.
 *    This allows the threads of be killed safely and the
 *    7 segment display to be cleared.
 *
 *    After initialisation, the user is asked to toggle the
 *    emergency state with the letter 'e', or to e'x'it the program.
 *
 *  @param Void.
 *  @return Void.
 */
int main (void) {
  int ret;
  int prev_state = state;
//  sighandler_t

  if(ret = networkSetup() != 0){
    printd("Socket failed to init, error no: %d\n", ret);
    printf("Network setup failed\n");
    return 1;
  }

  printd("Setting up USB ACM terminal\n");
  setup_term();

  /* error handling - reset leds */
  atexit(closing_time);
  signal(SIGINT, (void *)closing_time);

  printd("Setting up threads\n");
  start_threads();

  printf("press 'e' and 'enter' to toggle emergency state\n");
  printf("press 'x' and 'enter' to exit\n");

  while((ret = getchar()) != 'x' && alive){
    if (ret == 'e'){
      pthread_mutex_lock(&state_Mutex);
      if (state == EMERGENCY_STATE){
        state = prev_state;
        if(state == SUBMENU_SELECT){
          state = MENU_SELECT;
        }
      }
      else {
        prev_state = state;
        state = EMERGENCY_STATE;
      }
  	  pthread_cond_broadcast(&state_Signal);
	  pthread_mutex_unlock(&state_Mutex);

      pthread_mutex_lock(&button_Mutex);  // Unlock state machine
      pthread_cond_broadcast(&button_Signal);
      pthread_mutex_unlock(&button_Mutex);

    }
  }
  alive = FALSE; // fallen out by pressing 'x'
  return 0;
}
