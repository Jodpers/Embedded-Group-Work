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
const BYTE numtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};//0-9
const BYTE uitab[] = {0x00,'1', '2', '3', FORWARD,      // Keypad button assignment
                           '4', '5', '6', ENTER_MENU,
                           '7', '8', '9', DELETE,
                   ACCEPT_PLAY, '0',BACK, CANCEL};
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

BYTE reset_flag = FALSE;
BYTE menu_set = FALSE;
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
  static BYTE saved_digits[COLSX] = {0};//{0x73,0x79,0x78,0x79};
  static int prev_cursor_pos = 0;

  switch(display_flag){
    case CHANGED:
      if(finished == FALSE){ // If not finished showing last string
        for(i=0;i<COLSX;i++){ // clear current digits
          digits[i] = 0;
        }
      }
      cursor_blink = FALSE;
      finished = FALSE;

      block = blocking; // Make local copy of global
      offset = 0;
      display_flag = WRITING;
      
    case WRITING:
      switch(finished){
        case FALSE:
          if(offset == 0){
            for(i=0;i<COLSX;i++){       // Shift old chars by 1
              digits[i] = digits[i+1];
            }
            digits[3] = 0;                 // Make space for new string
          }

          if(display_buffer[offset] != 0){
            while(display_buffer[offset] == '.' && display_buffer[offset] != 0 && alive){
              digits[3] |= CURSOR_VALUE; // Only display single '.' on relevant digit
              offset++;                  // even if there are many in the string
            }
            for(i=0;i<3;i++){           // Scroll everything left
              digits[i] = digits[i+1];
            }
            digits[3] = display_char(display_buffer[offset]);
            offset++;  // Display the relevant Hex value from the string

            /* If we're in the menu state then copy the currently digits
             * so they can displayed after the string has finished scrolling
             */
            if(menu_set == TRUE && offset == 5){
              for(i=0;i<4;i++){
                saved_digits[i] = digits[i];
              }
            }
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

            if(reset_flag == FALSE || menu_set == TRUE){
              digits[3] = saved_digits[COLSX-pad]; // Copy saved display
            }
            else{
              digits[3] = 0; // If its been reset then restore
            }

            pad--;
          }
          else{
            pthread_cond_signal(&display_Signal);
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
      }
      started_waiting = TRUE;
      display_flag = WAITING;
      
    case WAITING:
      if(started_waiting){
        for(i=0;i<COLSX;i++){
          if(reset_flag == FALSE){
            saved_digits[i] = digits[i]; // Copy current display
          }
          else{
            saved_digits[i] = 0; // If its been reset then prevent restore
          }
        }
        reset_flag = FALSE;
        started_waiting = FALSE;
        cursor_blink = TRUE;
      }
      
      if(cursor_blink == TRUE){
        if(prev_cursor_pos != cursor_pos){ // If the cursor position has changed
            digits[prev_cursor_pos] &= NO_CURSOR; // Mask out the old cursor
            prev_cursor_pos = cursor_pos;   // Update the currently held value
          }
          
        if(cursor_pos <= DIGITS_MAX){ // only display the cursor on digits 0-3
          digits[cursor_pos] ^= CURSOR_VALUE;
        }
      }
      break;
    default:
      break;
  }  
}

/*------------------------------------------------------------------------------
 * Insert character into the buffer and move the cursor
 *------------------------------------------------------------------------------
 */
void insert_char(char in_char){
  char temp_buffer[BUFFER_SIZE] = {0};
  int input_ptr;
  
  input_ptr = (cursor_pos + cursor_offset);
  if(input_ptr == input_len){
    switch(logged_in){
      case FALSE: // Inputting PIN Number
        if(input_len < PIN_MAX){
          input_len++;
          input_buffer[input_ptr] = in_char;
          move_cursor(RIGHT);
        }
        break;

      case TRUE: // Inputting Track Number
        if(input_len < TRACK_MAX){
          input_len++;
          input_buffer[input_ptr] = in_char;
          move_cursor(RIGHT);
        }
        break;
      default:
        break;
    }
  }
  else if(input_ptr < input_len){ // Cursor isn't at the end of the line
    switch(logged_in){
    case FALSE:
      if(input_len < PIN_MAX){
        strncpy(temp_buffer,input_buffer,input_ptr);
        temp_buffer[input_ptr] = in_char;
        temp_buffer[input_ptr+1] = 0;
        strcat(temp_buffer,&input_buffer[input_ptr]);
        strcpy(input_buffer,temp_buffer);
        input_len++;
        move_cursor(RIGHT);
      }
      break;
    case TRUE: // Inputting Track Number
      if(input_len < TRACK_MAX){
        strncpy(temp_buffer,input_buffer,input_ptr);
        temp_buffer[input_ptr] = in_char;
        temp_buffer[input_ptr+1] = 0;
        strcat(temp_buffer,&input_buffer[input_ptr]);
        strcpy(input_buffer,temp_buffer);
        input_len++;
        if((cursor_pos < DIGITS_MAX) && input_len < TRACK_MAX){
          move_cursor(RIGHT);
        }
        else{
          if(cursor_pos < TRACK_MAX-1){
            cursor_offset++;
          }
        }
      }
      break;
    default:
      break;
    }
  }
  display_input_buffer();
}
/*------------------------------------------------------------------------------
 * Delete Character from the buffer and move the cursor
 *------------------------------------------------------------------------------
 */
void delete_char(void){
  int i;
  int input_ptr;

  if(input_len){ // Only delete if there is something already
    input_ptr = (cursor_pos + cursor_offset);
    if(input_ptr < input_len){  // Only if the cursor isn't at the end
      for(i=input_ptr;i<input_len;i++){ // Shift items in the buffer
        input_buffer[i] = input_buffer[i+1];
      }

    }
    input_buffer[--input_len] = 0; // NULL terminate string

    if(input_len < input_ptr){  // Deleting before the end string
      if(--cursor_pos < 0){
        cursor_pos = 0;
        if(cursor_offset){
          cursor_offset--;
        }
      }
      else if(input_len >= DIGITS_MAX && cursor_offset){
        cursor_offset--;
        cursor_pos++;
      }
    }
    //printf("cursor_offset: %d\n cursor_pos: %d\n input_len: %d\n",cursor_offset,cursor_pos,input_len);
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
        else if(input_len >= DIGITS_MAX && cursor_offset){
          cursor_offset--;
          cursor_pos++;
        }
      }
      break;
    case RIGHT:
      if(input_len){ // Only go right if there is somewhere to go
        if((cursor_pos + cursor_offset) < input_len){
          if(++cursor_pos > DIGITS_MAX){
            cursor_pos = DIGITS_MAX;
            if(logged_in == TRUE){ // Inputting Track Number
              if((cursor_pos + cursor_offset) < TRACK_MAX-1){ // limit digits displayed
                cursor_offset++;
              }
            }
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
  int i;
  pthread_mutex_lock(&display_Mutex);
  bzero(display_buffer,BUFFER_SIZE);
  strcpy(display_buffer,in);
  blocking = blocked;
  display_flag = CHANGED;
  if(blocking == TRUE){
    pthread_cond_wait(&display_Signal,&display_Mutex);
  }
  else{
    for(i=0;i<=DIGITS_MAX;i++){ // Clear the LEDs
      digits[i]=0;
    }
  }
  pthread_mutex_unlock(&display_Mutex);
}

void display_input_buffer(void){
  pthread_mutex_lock(&display_Mutex);
  strcpy(display_buffer,&input_buffer[cursor_offset]);
  //printf("input_buffer: %s\n",input_buffer);
  display_flag = INPUTTING;
  pthread_mutex_unlock(&display_Mutex);
}

void display_time(void){};


/*------------------------------------------------------------------------------
 * Clear the buffer, counter and cursor position on reset
 *------------------------------------------------------------------------------
 */
void reset_buffers(void){ // Reset everything
  int i;

  pthread_mutex_lock(&display_Mutex);

  bzero(input_buffer,BUFFER_SIZE);
  bzero(display_buffer,BUFFER_SIZE);
  /*
  for(i=0;i<BUFFER_SIZE;i++){
    input_buffer[i]='\0';
    display_buffer[i]='\0';
  }
   */
  input_len = 0;
  input_ptr = 0;
  cursor_pos = 0;
  cursor_offset = 0;

  for(i=0;i<4;i++){ // Clear the LEDs
    digits[i]=0;
  }
  reset_flag = TRUE;
  pthread_mutex_unlock(&display_Mutex);
}
/*------------------------------------------------------------------------------
 * Set the menu flag TRUE or FALSE from menu.c
 *
 * Flag used in the WRITING state of display to copy the first 4 chars
 * to display after the selected choice string has finished scrolling.
 * e.g. "1.Vol"
 *------------------------------------------------------------------------------
 */
void set_menu(BYTE in){
  pthread_mutex_lock(&display_Mutex);
  menu_set = in;
  pthread_mutex_unlock(&display_Mutex);

  if(!in){
    reset_buffers();
  }
}
