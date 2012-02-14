/*
 * keypad.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "keypad.h"

char button = FALSE;  // Button pressed 1-16 or -1 for multiple buttons

BYTE digits[COLSX] = {0x00,0x00,0x00,0x00};

/*------------------------------------------------------------------------------
 * keypad thread - Continuously outputs to 7 Segment display and reads buttons
 *------------------------------------------------------------------------------
 */
void * keypad(void){
  int col;
  int timeout = SCROLL_DELAY;
  char str[6];
  
  while(alive){
    pthread_mutex_lock(&display_Mutex);
    if(display_flag == CHANGED){
      timeout = 1; //Trigger an update
    }

    if(--timeout == 0){
      update_display(); // display.c
      timeout = SCROLL_DELAY;
    }
    pthread_mutex_unlock(&display_Mutex);

    for(col=0;col<COLSX;col++){
      write_to_port(C, 0);                  // LEDS off
      write_to_port(A, (BYTE) (01 << col)); // select column
      write_to_port(C, digits[col]);        // next LED pattern

      write(fd_RS232,"@00P1?\r",7);         // Read the column
      usleep(SLEEP);
      read(fd_RS232,str,6);
      
      pthread_mutex_lock(&button_Mutex);
      read_button(col,str[4]);
      pthread_mutex_unlock(&button_Mutex);
    }
  }
  return 0;
}

/*------------------------------------------------------------------------------
 * Read Button - Translates the ASCII value read from the serial line.
 *               Single Hex digit represents which buttons are pressed:
 *        E.g. Column Selected | Row 8 4 2 1     Bottom Right button pressed
 *                        0    |     1 0 1 0     Hex: 0x0A
 *
 *             Only one key press is allowed per complete scan.
 *------------------------------------------------------------------------------
 */
void read_button(int col, char in){
  int row;
  int out = 0; // Used to decode ASCII to binary
  static char temp; // Stores the HEX value to be sent to display routines
  static BYTE keypresses = 0;
  static int timeout = BUTTON_WAIT_DELAY;
  
  if(in > 0x40) {   // Convert output from ASCII to binary
    out |= (0x0F & (in-0x07)); // A-F
  }
  else{
    out |= (0x0F & (in));      // 0-9
  }

  for(row=0; row<ROWSX; row++){     // Scan the rows for key presses
    if((out >> row) & 0x01){
      keypresses++;     // Keep track of how many times keys have been pressed
      temp = uitab[((col+1)+(row*4))]; // Set the detected button (in display.c)
    }
  }

  if(col == COLSX-1){       // After reading all the columns
    switch(keypresses){
      case 0:
        button=FALSE; // No key press detected
        timeout = BUTTON_WAIT_DELAY;
        break;
      case 1:
        if(--timeout <= 0){ // Slow down button presses
          button=temp; // Write ASCII value from uitab in display.c
          timeout = BUTTON_PRESS_DELAY;  // Reset button press delay
          pthread_cond_signal(&button_Signal); // Signal UI to wake
        }
        break;
      default:
        button=ERROR;       // Multiple keys pressed
        timeout = BUTTON_WAIT_DELAY;
        break;
    }
    keypresses = 0;     // Reset counter for next loop of all columns
  }
  return;
} 

