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
 * keypad thread - this function continuously outputs to LEDs and reads buttons
 *------------------------------------------------------------------------------
 */
void * keypad(void){
  int col;
  int timeout = DELAY;
  char str[6];
  
  while(alive){
    if(display_flag == CHANGED){
      timeout = 1; //Trigger an update
    }
    if(--timeout == 0){
      update_display();
      timeout = DELAY;
    }
    for(col=0;col<COLSX;col++){
      write_to_port(C, 0);        // LEDS off
      write_to_port(A, (BYTE) (01 << col));     // select column
      write_to_port(C, digits[col]);  // next LED pattern  

      write(fd_RS232,"@00P1?\r",7);             // Read the column
      usleep(SLEEP);
      read(fd_RS232,str,6);
      
      read_button(col,str[4]);
    }
  }
  return 0;
}

void read_button(int col, char in){
  int row;
  int out = 0;
  static char temp;
  static BYTE keypresses = 0;
  
  if(in > 0x40) {   // Convert output from ASCII to binary
    out |= (0x0F & (in-0x07)); // A-F
  }
  else{
    out |= (0x0F & (in));      // 0-9
  }

  for(row=0; row<ROWSX; row++){     // Scan the rows for key presses
    if((out >> row) & 0x01){
      keypresses++;
      temp = uitab[((col+1)+(row*4))];// Set the detected button
    }
  }

  if(col == COLSX-1){       // After reading all the columns
    switch(keypresses){
      case 0:
        button=FALSE; // No key press detected
        break;
      case 1:
        button=temp; // Write ASCII value from uitab
        break;
      default:
        button=ERROR;       // Multiple keys pressed
        break;
    }
    keypresses = 0;
  }
  return;
} 

