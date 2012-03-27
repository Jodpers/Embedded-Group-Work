/*
 * @file keypad.h
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#define BUTTON_WAIT_DELAY   4
#define BUTTON_PRESS_DELAY  8
#define SCROLL_DELAY        10

void * keypad(void);
void read_button(int, char);

extern char button;
extern BYTE digits[];

#endif /* KEYPAD_H_ */
