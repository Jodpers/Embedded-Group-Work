/*
 * states.h
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#ifndef STATES_H_
#define STATES_H_

#include "display.h"

#define PIN_LEN	4

/* State Table */
enum e_states{
	EMERGENCY,
	WAITING_LOGGED_OUT,
	INPUTTING_PIN,
	WAITING_LOGGED_IN,
	INPUTTING_TRACK_NUMBER,
	MENU_SELECT,
	MAX_STATES
} current_state;

extern int state;
extern BYTE playing;

void * state_machine(void);
extern void input_pin(char);
extern void input_track_number(char);
extern void menu_select(void);

#endif /* STATES_H_ */
