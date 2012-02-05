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

#define DELAY 			70	// Key press delay
#define SCROLL_DELAY 	100 // LED scrolling
#define CUR_TRIGGER  	4	// Cursor blinking rate
#define BUFFER_SIZE  	10
#define BUFFER_MAX  	BUF_SIZE - 1
#define PIN_MAX		  	4
#define TRACK_MAX	  	6
#define CURSOR_MAX		4

#define PADDED          3
#define BLOCKING        1
#define NOT_PADDED      0
#define NOT_BLOCKING    0

#define PIN				1
#define TRACK			2

#define ACCEPT_PLAY    'A'
#define BACK           'B'
#define CANCEL         'C'
#define DELETE         'D'
#define ENTER_MENU     'E'
#define FORWARD        'F'

#define CURSOR_VAL		0x80
/******************
  7-Seg hex map
    --1--
   20   2
     40
   10   4
    --8-- 80
*******************/
extern const BYTE segtab[];
extern const BYTE numtab[];
extern const BYTE uitab[];
extern const BYTE alphaU[];
extern const BYTE alphaL[];

extern char button;

extern BYTE digits[];
extern int digits_begin;

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
