/*
 * display.h
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "top.h"

#define READ_MASK		0xF0
#define WRITE_MASK      0xF1
#define WRITTEN_MASK    0xF2

#define BLOCKING_MASK   0x1F
#define NOT_BLOCKING    0x0F

#define BLOCKING        1

#define PADDED          3
#define NOT_PADDED      0

#define DELAY 			70	// Key press delay
#define SCROLL_DELAY 	100 // LED scrolling
#define CUR_TRIGGER  	4	// Cursor blinking rate

#define PIN_MAX		  	4
#define TRACK_MAX	  	6
#define CURSOR_MAX		4

#define PIN				1
#define TRACK			2

#define ACCEPT_PLAY    'A'
#define BACK           'B'
#define CANCEL         'C'
#define DELETE         'D'
#define ENTER_MENU     'E'
#define FORWARD        'F'

/******************
  7-Seg hex map
    --1--
   20   2
     40
   10   4
    --8-- 80
*******************/
#define CURSOR_VAL		0x80
extern const BYTE segtab[];
extern const BYTE numtab[];
extern const BYTE uitab[];
extern const BYTE alphaU[];
extern const BYTE alphaL[];

extern char button;

extern BYTE digits[];

extern char input_buffer[BUFFER_SIZE];
extern char display_buffer[BUFFER_SIZE];
extern BYTE display_flag;
extern char cursor_position;

extern char buffer[BUFFER_SIZE];
extern char buffer_cnt;
extern char buffer_pos;
extern char cur_pos;
extern BYTE cursor;


void delay();			// Delay between button presses
void scroll_delay(BYTE);			// Delay of digits scrolling
void reset_buffer(void);
void clear_display();
void cursor_blink();
void move_cursor(BYTE);

void shift_digits();

void display_char(char,BYTE);
void display_string(char *,BYTE,BYTE);
void delete_char(void);
void insert_char(char,BYTE);

#endif /* DISPLAY_H_ */
