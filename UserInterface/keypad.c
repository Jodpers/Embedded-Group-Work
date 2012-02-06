/*
 * keypad.c
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#include "keypad.h"

char button = FALSE;  // Button pressed 1-16 or -1 for multiple buttons

/*------------------------------------------------------------------------------
 * keypad thread - this function continuously outputs to LEDs and reads buttons
 *------------------------------------------------------------------------------
 */
void * keypad(void){
  int i;
  int col;
  int out;
  char temp;
  char str[6];
  BYTE keypresses = 0;

  while(alive){
    for (col=0;col<COLSX;col++){
      write_to_port(C, 0);        // LEDS off
      write_to_port(A, (BYTE) (01 << col));     // select column
      write_to_port(C, digits[col]);  // next LED pattern

      write(fd_RS232,"@00P1?\r",7);             // Read the column
      usleep(SLEEP);
      read(fd_RS232,str,6);

      out = 0;
      if(str[4] > 0x40) {   // Convert output from ASCII to binary
        out |= (0x0F & (str[4]-0x07)); // A-F
      }
      else{
        out |= (0x0F & (str[4]));      // 0-9
      }

      for(i=0; i<ROWSX; i++){     // Scan the rows for key presses
        if((out >> i) & 0x01){
          keypresses++;
          temp = uitab[((col+1)+(i*4))];// Set the detected button
        }
      }

      if(col == COLSX-1){       // After reading all the columns
    	pthread_mutex_lock(&button_Mutex);
        switch(keypresses){
          case 0:
            button=FALSE; // No key press detected
            break;
          case 1:
            button=temp; // Write ASCII value from uitab
            pthread_cond_signal(&button_Signal);
            break;
          default:
            button=ERROR;       // Multiple keys pressed
            break;
        }
        pthread_mutex_unlock(&button_Mutex);

        keypresses = 0;
      }
    }
  }
  return 0;
}

