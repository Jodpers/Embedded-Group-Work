/*
 * @file threads.c
 *
 *  Created on 6 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //threads
#include <pthread.h>

#include "top.h"
#include "pio_term.h"
#include "threads.h"

/* Thread Names */
pthread_t keypad_thread;
pthread_t state_machine_thread;

pthread_t network_thread;
pthread_t receive_thread;

pthread_t timer_thread;
pthread_t receiver_thread;

/* Thread Attributes */
pthread_attr_t keypad_Attr;
pthread_attr_t state_machine_Attr;

pthread_attr_t network_Attr;
pthread_attr_t receive_Attr;

pthread_attr_t timer_Attr;
pthread_attr_t receiver_Attr;

/* Mutexs and Signals */
pthread_mutex_t button_Mutex;
pthread_cond_t button_Signal;

pthread_mutex_t state_Mutex;
pthread_cond_t state_Signal;

pthread_mutex_t display_Mutex;
pthread_cond_t display_Signal;

pthread_mutex_t network_Mutex;
pthread_cond_t network_Signal;

pthread_mutex_t request_Mutex;
pthread_cond_t request_Signal;

pthread_mutex_t timer_Mutex;
pthread_cond_t timer_Signal;

int button_thread_state;

/**
 *  @brief Initialises Mutexs, Signals and Attributes.
 *
 *  @param Void.
 *  @return Void.
 */
void setup_threads(void){

	  /* Setup Mutex */
	  pthread_mutex_init(&button_Mutex, NULL);
	  pthread_mutex_init(&state_Mutex, NULL);
      pthread_mutex_init(&display_Mutex, NULL);
	  pthread_mutex_init(&network_Mutex, NULL);
	  pthread_mutex_init(&request_Mutex, NULL);

	  /* Setup Signals */
	  pthread_cond_init(&button_Signal, NULL);
	  pthread_cond_init(&state_Signal, NULL);
      pthread_cond_init(&display_Signal, NULL);
	  pthread_cond_init(&network_Signal, NULL);
	  pthread_cond_init(&request_Signal, NULL);

	  /* Setup Thread Attributes */
	  pthread_attr_init(&keypad_Attr);
	  pthread_attr_init(&state_machine_Attr);
	  pthread_attr_init(&network_Attr);
	  pthread_attr_init(&receive_Attr);

	  return;
}

/**
 *  @brief Launches threads.
 *    Each thread in the system is launched in this function.
 *    If any thread cannot start the program exits with an error.
 *
 *  @param Void.
 *  @return Void.
 */

void start_threads(void){
  extern void * keypad(void);
  extern void * state_machine(void);
  extern void * networkingFSM(void);
  extern void * receive(void);
  extern void * timer(void);

  if(pthread_create( &keypad_thread, &keypad_Attr, (void *)keypad, NULL) != 0){
    perror("Keypad thread failed to start\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_create( &state_machine_thread, &state_machine_Attr, (void *)state_machine, NULL) != 0){
    perror("State Machine thread failed to start\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_create( &network_thread, &network_Attr, (void *)networkingFSM, NULL) != 0){
    perror("Network thread failed to start\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_create( &receive_thread, &receive_Attr, (void *)receive, NULL) != 0){
    perror("Network Receive thread failed to start\n");
    exit(EXIT_FAILURE);
  }
  if((pthread_create( &timer_thread, &timer_Attr, (void *)timer, NULL)) != 0){
    perror("Timer thread failed to start\n");
    exit(EXIT_FAILURE);
  }


  return;
}

/**
 *  @brief Exit Subroutine.
 *
 *    This function is called when the program is signalled to close.
 *    It performs tidy-up operations, such as killing threads and
 *    clearing the last value on the 7 Segment display.
 *
 *  @param Void.
 *  @return Void.
 */
void closing_time(void){
  alive = FALSE;

  pthread_mutex_lock(&state_Mutex);
  button_thread_state = STATE_KILL;
  pthread_cond_signal(&state_Signal);
  pthread_mutex_unlock(&state_Mutex);

  pthread_mutex_lock(&button_Mutex);
  pthread_cond_signal(&button_Signal);
  pthread_mutex_unlock(&button_Mutex);

  pthread_join(keypad_thread, NULL);
  pthread_join(state_machine_thread, NULL);
  printf("Signalled threads to close\n");

  pthread_join(network_thread, NULL);
  pthread_join(receive_thread, NULL);


  write_to_port(C, 0);      /* Last LED off */
  close_term();

  pthread_attr_destroy(&keypad_Attr);
  pthread_attr_destroy(&state_machine_Attr);
  pthread_attr_destroy(&network_Attr);
  pthread_attr_destroy(&receive_Attr);

  pthread_mutex_destroy(&button_Mutex);
  pthread_mutex_destroy(&state_Mutex);
  pthread_mutex_destroy(&display_Mutex);

  pthread_mutex_destroy(&network_Mutex);
  pthread_mutex_destroy(&request_Mutex);

  pthread_cond_destroy(&button_Signal);
  pthread_cond_destroy(&state_Signal);
  pthread_cond_destroy(&display_Signal);

  pthread_cond_destroy(&network_Signal);
  pthread_cond_destroy(&request_Signal);
  printf("Closing\n");
}

