/*
 * @file display.h
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_


/* Display Flag States */
enum display_states{
	WAITING,
	CHANGED,
	INPUTTING,
    DISPLAYING_TIME,
    CLEARING_TIME,
	WRITING
} display_state;

#define CURSOR_VALUE    0x80
#define NO_CURSOR       0x7F

#define LEFT            0
#define RIGHT           1

#define NOT_BLOCKING    0
#define BLOCKING        1

#define PIN_MAX		  	4
#define TRACK_MIN     1
#define TRACK_MAX	  	8
#define DIGITS_MAX		3

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

extern const BYTE numtab[];
extern const BYTE uitab[];
extern const BYTE alphaU[];
extern const BYTE alphaL[];

extern char button;

extern BYTE digits[];

extern char input_buffer[BUFFER_SIZE];
extern char display_buffer[BUFFER_SIZE];

extern BYTE display_flag;
extern BYTE cursor_blink;

extern BYTE reset_flag;

extern BYTE blocking;
extern BYTE padding;

extern int cursor_pos;
extern int cursor_offset;
extern int input_len;
extern int input_ptr;

extern int logged_in; // (states.c)
extern int already_logged_in;

void update_display(void);

void insert_char(char);
void delete_char(void);
void move_cursor(int);

BYTE display_char(char);
void display_string(char *,BYTE);
void display_input_buffer(void);
void display_volume(long);
void display_time(char *in);
void clear_time(void);

void set_menu(BYTE);
extern void reset_buffers(void);

int set_scroll_delay(int);
int get_scroll_delay(void);
#endif /* DISPLAY_H_ */
