/*
 * keypad.h
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <string.h>
#include "top.h"
#include "pio_term.h"
#include "display.h"
#include "threads.h"

#define BUTTON_WAIT_DELAY   4
#define BUTTON_PRESS_DELAY  8
#define SCROLL_DELAY        6

void * keypad(void);
void read_button(int, char);

#endif /* KEYPAD_H_ */
