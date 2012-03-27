/*
 * @file threads.h
 *
 *  Created on 6 Feb 2012
 *     @author Pete Hemery
 */

#ifndef THREADS_H_
#define THREADS_H_

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

/* Display Buffers */
extern pthread_mutex_t display_Mutex;
extern pthread_cond_t display_Signal;

/* Network Request State */
extern pthread_mutex_t network_Mutex;
extern pthread_cond_t network_Signal;

/* Request Signals */
extern pthread_mutex_t request_Mutex;
extern pthread_cond_t request_Signal;

/* Timer Signals */
extern pthread_mutex_t timer_Mutex;
extern pthread_cond_t timer_Signal;

/* Local Function Prototypes */

void setup_threads(void);
void start_threads(void);
void closing_time(void);

#endif /* THREADS_H_ */
