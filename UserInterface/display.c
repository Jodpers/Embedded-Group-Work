/*
 * display.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "display.h"

/******************
  7-Seg hex map
    --1--
   20   2
     40
   10   4
    --8-- 80
*******************/
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

char input_buffer[BUFFER_SIZE] = {0};
char display_buffer[BUFFER_SIZE] = {0};

int display_flag = WAITING;
int cursor_blink = FALSE;

BYTE blocking = FALSE;
BYTE padding = TRUE;

int cursor_pos = 0;
int cursor_offset = 0;
int input_len = 0;
int input_ptr = 0;

/*------------------------------------------------------------------------------
 * Display State Machine
 *------------------------------------------------------------------------------
 */
void update_display(void){
  int i;
  static int started_waiting = TRUE;
  static int block = FALSE;
  static int offset = 0;
  static int finished = TRUE;
  static int pad = 0;
  static BYTE saved_digits[COLSX] = {0x73,0x79,0x78,0x79};
  static int prev_cursor_pos = 0;
  
  switch(display_flag){
    case CHANGED:
      cursor_blink = FALSE;
      switch(finished){
        case FALSE:
          switch(block){
            case TRUE:
              printf("Blocked\n");
              break;
            case FALSE:
              display_flag = WRITING;
              for(i=0;i<COLSX;i++){
                digits[i] = 0;
              }
              block = blocking;
              offset = 0;
              break;
            default:
              break;
          }
          break;
        case TRUE:
          display_flag = WRITING;
          finished = FALSE;
          block = blocking;
          offset = 0;
          break;
        default:
          break;
      }
      break;
      
    case WRITING:
      switch(finished){
        case FALSE:
          if(offset == 0){
            for(i=0;i<COLSX;i++){
              digits[i] = digits[i+1];
            }
            digits[3] = 0;                 // Make space for new string
          }
          if(display_buffer[offset] != 0){
            while(display_buffer[offset] == '.' && display_buffer[offset] != 0){
              digits[3] |= CURSOR_VALUE;
              offset++;
            }
            for(i=0;i<3;i++){
              digits[i] = digits[i+1];
            }
            digits[3] = display_char(display_buffer[offset]);
            offset++;
          }
          else{
            finished = TRUE;
            for(i=0;i<COLSX;i++){
              digits[i] = digits[i+1];
            }
            digits[3] = 0;  // Space between end of string and restored digits
           
            if(padding == TRUE){
              pad = 4;
            }
          }
          break;
        
        case TRUE:
          if(pad){
            for(i=0;i<COLSX;i++){
              digits[i] = digits[i+1];
            }
            digits[3] = saved_digits[COLSX-pad];
            pad--;
          }
          else{
            blocking = FALSE;
            block = blocking;
            offset = 0;
            started_waiting = TRUE;
            display_flag = WAITING;
          }
          break;
        default:
          break;
        }
      break;
      
    case INPUTTING:
      for(i=0;i<COLSX;i++){
        digits[i] = display_char(input_buffer[i+cursor_offset]);
        saved_digits[i] = digits[i];
      }
      cursor_blink = TRUE;    
      block = FALSE;
      display_flag = WAITING;
      break;
      
    case WAITING:
      if(started_waiting){
        for(i=0;i<COLSX;i++){
          saved_digits[i] = digits[i]; // Copy current display
        }
        started_waiting = FALSE;
        cursor_blink = TRUE;
      }
      
      if(cursor_blink == TRUE){
        if(prev_cursor_pos != cursor_pos){
            digits[prev_cursor_pos] &= NO_CURSOR;
            prev_cursor_pos = cursor_pos;
          }
          
        if(cursor_pos < COLSX){
          digits[cursor_pos] ^= CURSOR_VALUE;
        }
      }
      break;
    default:
      break;
  }  
}

/*------------------------------------------------------------------------------
 * Char Insert and Delete
 *------------------------------------------------------------------------------
 */
void insert_char(char in_char){
  char temp_buffer[BUFFER_SIZE] = {0};
  int input_ptr;
  
  input_ptr = (cursor_pos + cursor_offset);
  if(input_ptr == input_len){
    if(input_len < BUFFER_SIZE){
      input_len++;
      input_buffer[input_ptr] = in_char;
      move_cursor(RIGHT);
    }
  }
  else if(input_ptr < input_len){
    strncpy(temp_buffer,input_buffer,input_ptr);
    temp_buffer[input_ptr] = in_char;
    temp_buffer[input_ptr+1] = 0;
    strcat(temp_buffer,&input_buffer[input_ptr]);
    strcpy(input_buffer,temp_buffer);
    input_len++;
    if((cursor_pos < COLSX-1) && input_len < COLSX){
      move_cursor(RIGHT);
    }
    else{
      cursor_offset++;
    }
  }
  display_input_buffer();
}

void delete_char(void){
  int i;
  int input_ptr;

  if(input_len > 0){  
    input_ptr = (cursor_pos + cursor_offset);
    if(input_ptr >= input_len){
      input_buffer[input_ptr-1] = 0;
      
    }
    else{
      for(i=input_ptr;i<input_len;i++){
        input_buffer[i] = input_buffer[i+1];
      }
      input_buffer[input_len] = 0;
    }
    input_len--;
    
    if(cursor_offset){
      if(--cursor_offset < 0){
        cursor_offset = 0;
      }
      if(cursor_pos < input_len){
        if(++cursor_pos >= COLSX-1){
          cursor_pos = COLSX-1;
        }
      }
    }
    else{
      if(cursor_pos){
        if(--cursor_pos < 0){
          cursor_pos = 0;
        }
      }
    }
  }
  display_input_buffer();
}
/*------------------------------------------------------------------------------
 * Cursor movement along the display buffer
 *------------------------------------------------------------------------------
 */

void move_cursor(int direction){
  switch(direction){
    case LEFT:
      if(input_len){ // Only go left if there is somewhere to go
        if(--cursor_pos < 0){
          cursor_pos = 0;
          if(cursor_offset){
            cursor_offset--;
          }
        }
        else if(cursor_pos < 3 && cursor_offset){
          cursor_offset--;
          cursor_pos++;
        }
      }
      break;
    case RIGHT:
      if(input_len){ // Only go right if there is somewhere to go
        if((cursor_pos + cursor_offset) < input_len){
          if(++cursor_pos >= COLSX){
            cursor_pos = COLSX-1;
            cursor_offset++;
          }
        }
      }
      break;
    default:
      break;
  }
  display_input_buffer();
  return;
}


/*------------------------------------------------------------------------------
 * Display Char Routine
 *------------------------------------------------------------------------------
 */
BYTE display_char(char key){
  BYTE character = 0;
  switch(key){
    case ' ':
      character = 0x00;
      break;
    case '-':
      character = 0x40;
      break;
    case '=':
      character = 0x48;
      break;
    case '.':
      character = 0x80;
      break;
    case '?':
      character = 0x83;
      break;
    case '!':
      character = 0x82;
      break;
    case '_':
      character = 0x08;
      break;
    case '(':
    case '<':
    case '[':
    case '{':
      character = 0x39;
      break;
    case ')':
    case '>':
    case ']':
    case '}':
      character = 0x0F;
      break;
    default:
      if((key >= '0') && (key <= '9')){      // Numbers
        character = numtab[key-'0'];
      }
      else if((key >= 'A') && (key <= 'Z')){ // "Upper case" alphabet
        character = alphaU[key-'A'];
      }
      else if((key >= 'a') && (key <= 'z')){ // "Lower case" alphabet
        character = alphaL[key-'a'];
      }
      else{
        character = 0;
      }
      break;
  }
  return character;
}
/*------------------------------------------------------------------------------
 * Display String Routines - Blocking and Non-Blocking
 *------------------------------------------------------------------------------
 */
void display_string(char * in, BYTE blocked){
  strcpy(display_buffer,in);
  blocking = blocked;
  display_flag = CHANGED;
}

void display_input_buffer(void){
  strcpy(display_buffer,&input_buffer[cursor_offset]);
  printf("input_buffer: %s\n",input_buffer);
  display_flag = INPUTTING;
}

void display_time(void){};


