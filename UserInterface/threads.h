/*
 * threads.h
 *
 *  Created on: 6 Feb 2012
 *      Author: student
 */

#ifndef THREADS_H_
#define THREADS_H_

#include <sys/types.h>  //threads
#include <pthread.h>

#include "pio_term.h"

enum thread_states{
	STATE_RUNNING,
	STATE_PAUSED,
	STATE_KILL
} thr_state;

extern int button_thread_state;

/* Button Press State */
extern pthread_mutex_t button_Mutex;
extern pthread_cond_t button_Signal;

/* Thread State */
extern pthread_mutex_t state_Mutex;
extern pthread_cond_t state_Signal;


void closing_time(void);

#endif /* THREADS_H_ */
