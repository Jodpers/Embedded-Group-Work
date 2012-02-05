/*
 * display.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "display.h"

const BYTE segtab[] = {0x00,0x06,0x5B,0x4F,0x71,0x66,0x6D,0x7D,0x79,
		       0x07,0x7F,0x6F,0x5E,0x77,0x3F,0x7C,0x39};
const BYTE numtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};//0-9
const BYTE uitab[] = {0x00,'1','2', '3',FORWARD,
                           '4','5', '6',ENTER_MENU,
                           '7','8', '9',DELETE,
                   ACCEPT_PLAY,'0',BACK,CANCEL};
/*                        A,   B,   C,   d,   E,   F,   g,   H,   I,*/
const BYTE alphaU[] = {0x77,0x7F,0x39,0x5E,0x79,0x71,0x6F,0x76,0x30,
/*                        J,   K,   L,   M,   n,   O,   P,   Q,   r,*/
                        0x1E,0x76,0x38,0x15,0x54,0x3F,0x73,0x67,0x50,
/*                        S,   t,   U,   V,   W,   X,   Y,   Z*/
                        0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};
/*                        A,   b,   c,   d,   E,   F,   g,   h   i,*/
const BYTE alphaL[] = {0x77,0x7C,0x58,0x5E,0x79,0x71,0x6F,0x74,0x04,
/*                        J,   K,   L,   M,   n,   o,   P,   Q,   r,*/
                        0x1E,0x76,0x38,0x15,0x54,0x5C,0x73,0x67,0x50,
/*                        S,   t,   U,   V,   W,   X,   Y,   Z*/
                        0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};

BYTE digits[COLSX] = {0x00,0x00,0x00,0x00};
int digits_begin = 0;

char buffer[BUFFER_SIZE] = {0};
char buffer_cnt = 0;
char buffer_pos = 0;
char cur_pos = 0;
BYTE cursor = 0;

/*------------------------------------------------------------------------------
 * Delay Routines - Used in printing chars and strings to the LEDs
 *------------------------------------------------------------------------------
 */

void delay(){        // Delay between button presses
  int i = DELAY;
  while(--i && alive)
    usleep(SLEEP);
}
void scroll_delay(BYTE blocking){ // Delay of text moving across the display
  int i = SCROLL_DELAY;
  if(blocking){
    while(--i && alive)
      usleep(SLEEP);
  }
  else{
    while(--i && alive && !button) // Non-blocking method
      usleep(SLEEP);
  }
}
/*------------------------------------------------------------------------------
 * Clear the buffer, counter and cursor position on reset
 *------------------------------------------------------------------------------
 */
void reset_buffer(void){ // Reset everything
  int i;
  for(i=0;i<BUFFER_SIZE;i++){
    buffer[i]=0;
  }
  buffer_cnt = 0;
  cur_pos = 0;

  for(i=0;i<4;i++){ // Clear the LEDs
    digits[i]=0;
  }
}
/*------------------------------------------------------------------------------
 * Clear Display
 *------------------------------------------------------------------------------
 */
void clear_display(void){
  int i;
  for(i=0;i<4;i++)
    digits[i] = 0;
}

/*------------------------------------------------------------------------------
 * Cursor Blinking Routine - Toggles the dot on the displayed char
 *------------------------------------------------------------------------------
 */
void cursor_blink(void){
  static DWORD timer = CUR_TRIGGER;
  static BYTE prev_pos = 0;

  if(cur_pos != prev_pos){ // Cursor has moved
    digits[prev_pos] &= (BYTE) 0x7F; // Clear the old cursor
    if(cur_pos < 4){  // Unless pointing off the edge of the display
      digits[(BYTE)cur_pos] ^= CURSOR_VAL; // Toggle the cursor
    }
    prev_pos = cur_pos; // Remember where cursor is
    timer = CUR_TRIGGER;
  }

  timer--;      // Only toggle every few loops
  if(!timer){   // Gives the blinking effect
    timer = CUR_TRIGGER;
    if(cur_pos < 4){ // Unless pointing off the edge of the display
      digits[(BYTE)cur_pos] ^= CURSOR_VAL;
    }
  }
}

/*------------------------------------------------------------------------------
 * Display Digit Shifting Routines
 *------------------------------------------------------------------------------
 */
void shift_digits_left(void){
  int i;
  int buf_len;

  for(i=0;i<SCROLL_DELAY;i++); // Delay of text moving across the display
  digits[(BYTE)cur_pos] &= 0x7F;  // Clear the cursor
  buf_len = strlen(buffer);
  if(buf_len < BUFFER_SIZE){
    buffer_pos++;
    if(buffer_pos == buf_len){
      buffer_pos--;
    }
    if(buffer_pos > 3){
      for(i=0;i<4;i++){
        digits[i] = numtab[buffer[buffer_pos-(3-i)]-'0'];
      }
    }
  }
  cursor_blink();
}

void shift_digits_right(void){
}

void shift_digits(){
  int i;
  digits[(BYTE)cur_pos] &= 0x7F;
  for(i=0;i<3;i++){
    digits[i] = digits[i+1];
  }
  digits[3] = 0;
}
/*------------------------------------------------------------------------------
 * Display Char Routine
 *------------------------------------------------------------------------------
 */
void display_char(char key, BYTE blocking){

  if(key == '.' || key == ':'){// Add a full stop on the right char
    digits[3] |= CURSOR_VAL;
    return;
  }

  shift_digits();
  switch(key){
    case ' ':
      digits[3] = 0x00;
      break;
    case '_':
      digits[3] = 0x08;
      break;
    case '-':
      digits[3] = 0x40;
      break;
    case '=':
      digits[3] = 0x48;
      break;
    /*case '.':
      digits[3] = CURSOR_VAL;
      break;*/
    case '!':
      digits[3] = 0x82;
      break;
    case '?':
      digits[3] = 0x83;
      break;
    case '(':
    case '<':
    case '[':
    case '{':
      digits[3] = 0x39;
      break;
    case ')':
    case '>':
    case ']':
    case '}':
      digits[3] = 0x0F;
      break;
    default:
      if((key >= '0') && (key <= '9')){      // Numbers
        digits[3] = numtab[key-'0'];
      }
      else if((key >= 'A') && (key <= 'Z')){ // "Upper case" alphabet
        digits[3] = alphaU[key-'A'];
      }
      else if((key >= 'a') && (key <= 'z')){ // "Lower case" alphabet
        digits[3] = alphaL[key-'a'];
      }
      else{
        digits[3] = 0x00;
      }
      break;
  }
  scroll_delay(blocking);
}
/*------------------------------------------------------------------------------
 * Display String Routines - Blocking and Non-Blocking
 *------------------------------------------------------------------------------
 */

void display_string(char *in, BYTE padded, BYTE blocking){
  int i;
  BYTE temp[4];
  static BYTE prev_padded = TRUE;

  if(button && !blocking){  // Routine called by a button press?
    while(button && alive){  // Wait until not pressed
      usleep(SLEEP);
    }
  }

  for(i=0;i<4;i++){
    temp[i]=digits[i]; // Save current digits
  }

  if(padded && prev_padded){
    for(i=0;i<PADDED;i++){
      shift_digits();
      scroll_delay(blocking);
    }
  }

  if(blocking){
    while(*in!='\0' && alive){
      display_char(*in,blocking); // Print chars in the string
      in++;
    }
  }
  else{
    while(*in!='\0' && alive && !button){
      display_char(*in,blocking); // Print chars in the string
      in++;
    }
  }

  if(button && !blocking){
    for(i=0;i<4;i++){
      digits[i]=temp[i]; // Replace saved digits
    }
  }
  else{   // Uninterrupted
    if(padded){
      for(i=0;i<PADDED;i++){ // Finish Scrolling the message off
        shift_digits();
        scroll_delay(blocking);
      }
    }
    if(blocking){
      for(i=0;i<4;i++){
        shift_digits();
        digits[3]=temp[i]; // Replace saved digits
        scroll_delay(blocking);
      }
    }
  }
  prev_padded = padded;
  return;
}

void display_time(void){};


void delete_char(void){
  int i;
  if (cur_pos == CURSOR_MAX || cur_pos == buffer_cnt){
    buffer[(BYTE)--buffer_cnt] = 0;
    digits[(BYTE)cur_pos] &= 0x7F;
    digits[(BYTE)--cur_pos] = 0;
    cursor_blink();
  }
  else{
	digits[(BYTE)cur_pos] &= 0x7F;
	for(i=cur_pos;i<4;i++){
      digits[i] = digits[i+1];
	  buffer[i] = buffer[i+1];
	}
	buffer[(BYTE)buffer_cnt--] = 0;
	digits[3] = 0;
	}
	if (!buffer_cnt || cur_pos < 0){
	  cur_pos = 0;
	  buffer_cnt = 0;
	  buffer[(BYTE)buffer_cnt] = 0;
	}
  return;
}
/*void insert_char(char button_read, BYTE TrackOrPIN){

  return;
}*/
void insert_char(char button_read, BYTE TrackOrPIN){
  if(TrackOrPIN == PIN){
  if(buffer_cnt != cur_pos){  // If cursor isn't at the end
	  buffer_cnt--;  // Decrement it before next line, lest we wrongly add 1
	}
	if(++buffer_cnt > PIN_MAX){ // Button has been pressed, let's up the buffer count
	  buffer_cnt = PIN_MAX; // Limit PIN to 4 digits
	  cur_pos = CURSOR_MAX; // Cursor is off the edge of the array and won't blink
	  buffer[(BYTE)cur_pos] = 0; // Null terminate string containing PIN
	}
	else{   // Less than 4 digits already? Well, get it in there!
	  digits[(BYTE)cur_pos] = numtab[button_read-'0'];// Display the digit pressed
	  buffer[(BYTE)cur_pos] = button_read; // Copy ASCII button press
	  cur_pos = buffer_cnt; // Point at next empty location
	  cursor_blink();  // Clear last cursor and move to next empty space
	  delay();  // Wait to make sure button has stopped being pressed
	}
  }
  else if(TrackOrPIN == TRACK){
	  if(buffer_cnt != cur_pos){  // If cursor isn't at the end
	        buffer_cnt--;  // Decrement it before next line, lest we wrongly add 1
	      }
	      if(++buffer_cnt > TRACK_MAX){ // Button has been pressed, let's up the buffer count
	        buffer_cnt = TRACK_MAX; // Limit number of digits in Track Number
	        cur_pos = CURSOR_MAX; // Cursor is off the edge of the array and won't blink
	        buffer[(BYTE)cur_pos] = 0; // Null terminate string containing PIN
	      }
	      else{   // Less than 4 digits already? Well, get it in there!
	        digits[(BYTE)cur_pos] = numtab[button_read-'0'];// Display the digit pressed
	        buffer[(BYTE)cur_pos] = button_read; // Copy ASCII button press
	        cur_pos = buffer_cnt; // Point at next empty location
	        cursor_blink();  // Clear last cursor and move to next empty space
	        delay();  // Wait to make sure button has stopped being pressed
	      }
  }
  return;
}
