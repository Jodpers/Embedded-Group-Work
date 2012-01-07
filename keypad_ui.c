/*****
 terminal code used to send ASCII to USB PIO cable. From:
 http://www.st.ewi.tudelft.nl/~gemund/Courses/In4073/Resources/myterm.c

 Keypad handling routines for Embedded Systems Design coursework
 22/12/2011 - Pete Hemery
 0.2 - User Interface state machine
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>			//file descriptors
#include <ctype.h>
#include <termios.h>		//terminal
#include <assert.h>
#include <time.h>
#include <sys/types.h>		//threads
#include <pthread.h>
#include "keypad_ui.h"

pthread_t keypad_thread;
BYTE alive = TRUE;		// Exit condition of while loops
BYTE cur_blink = TRUE;
short button = FALSE;		// Button pressed or -1 for multiple buttons
int fd_RS232;			// Terminal File descriptor
int state = WAITING_LO;		// State machine variable
int logged_in = FALSE;		// Client connected to server
BYTE digits[COLSX] = {0x00,0x00,0x00,0x00};
char pin[5] = {0};
char pin_cnt = 0;
char cur_pos = 0;
BYTE cursor = 0;


/*----------------------------------------------------------------
 * main -- Open terminal and launch keypad thread.
 *         Check for button input and respond appropriately
 *----------------------------------------------------------------
 */

int main () {
  int i;
  char *welcome="Welcome. Please Enter PIN Number.    ";
  char *emergency="EMERGENCY!  !";
  BYTE button_read = 0;  
  int ret;

  /* error handling - reset LEDs */
  atexit(closing_time);
  signal(SIGINT, closing_time);

  term_initio();
  rs232_open();
  
  setup_ports();
  ret = pthread_create( &keypad_thread, NULL, keypad, NULL);

//  display_string_nblock(welcome);

  while(alive){
    delay();

    if(state == EMERGENCY){
      display_string_block(emergency);
      continue;
    }

    cursor_blink();

    button_read = button;
    if(button_read){
      if(state == WAITING_LO){
	switch(uitab[button_read]){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
          if(pin_cnt != cur_pos){
            pin_cnt--;
          }
          if(++pin_cnt > 4){
            pin_cnt = 4;
            cur_pos = 4;
          }
          else{
            digits[cur_pos] = segtab[button_read];
            pin[cur_pos] = uitab[button_read]+'0';
            cur_pos = pin_cnt;
            digits[cur_pos] ^= 0x80;
          }
printf("pin: %s\npin_cnt: %d\ncur_pos: %d\n",pin,pin_cnt,cur_pos);
	  break;
	case ACCEPT_PLAY:
	  display_string_block("Play");
	  break;
	case BACK:
	  if(--cur_pos<=0){
            cur_pos=0;
	  }
printf("pin: %s\npin_cnt: %d\ncur_pos: %d\n",pin,pin_cnt,cur_pos);
	  break;
	case CANCEL:
	  pin_cnt = 0;
          cur_pos = 0;
	  for(i=0;i<4;i++){
	    digits[i]=0;
	    pin[i]=0;
	  }
printf("pin: %s\npin_cnt: %d\ncur_pos: %d\n",pin,pin_cnt,cur_pos);
          //cur_blink=FALSE;
	  break;
	case DELETE:
          if (cur_pos == 4 || cur_pos == pin_cnt){
            pin[--pin_cnt] = 0;
            digits[--cur_pos] = 0;
          }
          else{
//            digits[cur_pos] &= 0x7F;
            for(i=cur_pos;i<4;i++){
              digits[i] = digits[i+1];
              pin[i] = pin[i+1];
            }
            pin[pin_cnt--] = 0;
            //digits[cur_pos] = 0;
            digits[3] = 0;
          }
          if (!pin_cnt || cur_pos < 0){
            cur_pos = 0;
            pin_cnt = 0;
            pin[pin_cnt] = 0;
          }

printf("pin: %s\npin_cnt: %d\ncur_pos: %d\n",pin,pin_cnt,cur_pos);
	  break;
	case ENTER_MENU:
	  break;
	case FORWARD:
	  if(pin_cnt){
            if(++cur_pos>pin_cnt){
              cur_pos = pin_cnt;
	    }
	  }
printf("pin: %s\npin_cnt: %d\ncur_pos: %d\n",pin,pin_cnt,cur_pos);
	  break;
	default:
          display_char('.');
	  break;
	}
      }
    }
  }

  pthread_join(keypad_thread, NULL);

  term_exitio();
  rs232_close();
  return 0;
}

/*----------------------------------------------------------------
 * keypad thread - this function continiously outputs to LEDs and reads buttons
 *----------------------------------------------------------------
 */
void * keypad(){
  int i;
  int col;
  char str[6];
  int out;
  BYTE keypresses = 0;


  while(alive){
    for (col=0;col<COLSX;col++){
      write_to_port(C, 0);     			// LEDS off
      write_to_port(A, (BYTE) (01 << col));     // select column
      write_to_port(C, digits[col]);		// next LED pattern  

      write(fd_RS232,"@00P1?\r",7);             // Read the column
      usleep(SLEEP);
      read(fd_RS232,str,6);

      out = 0;
      if(str[4] > 0x40) {	  // Convert output from ASCII to binary
        out |= (0x0F & (str[4]-0x07)); // A-F
      }
      else{
        out |= (0x0F & (str[4]));      // 0-9
      }

      for(i=0; i<ROWSX; i++){     // Scan the rows for key presses
        if((out >> i) & 0x01){
          keypresses++;
	  button = ((col+1)+(i*4));// Set the detected button
        }
      }

      if(col == COLSX-1){         // After reading all the columns
        if(keypresses){           // Check how many buttons were pressed
//      printf("\n %s\n",str);
          if(keypresses > 1){     // More than one is an error
            button=ERROR;         // Otherwise button has correct value
          }
        }
	else{
	  button=FALSE;           // No key press detected
	}
	keypresses=FALSE;         // Ready for the next loop
      }
    }
  }
}
/*----------------------------------------------------------------
 * display routines - printing chars and strings to the LEDs
 *----------------------------------------------------------------
 */

void delay(){        // Delay between button presses
  int i;
  for(i=0;i<DELAY;i++);
}

void scroll_delay(){ // Delay of text moving across the display
  int i;
  for(i=0;i<SCROLL_DELAY;i++);
}

void cursor_blink(){
  static DWORD timer = CUR_TRIGGER;
  static prev_pos = 0;

  if(cur_blink){
    timer--;
    if(!timer){
      timer = CUR_TRIGGER;
      if(cur_pos != prev_pos){
        digits[prev_pos] &= 0x7F;
        prev_pos = cur_pos;
      }
      digits[cur_pos] ^= 0x80;
    }
  }
}

void shift_digits(){
  scroll_delay();
  digits[cur_pos] &= 0x7F;
  digits[0] = digits[1];
  digits[1] = digits[2];
  digits[2] = digits[3];
}

void display_char(char key){
  shift_digits();

  switch(key){
    case ' ':
      digits[3] = 0x00;
      break;
    case '-':
      digits[3] = 0x40;
      break;
    case '.':
      digits[3] = 0x80;
      break;
    case '=':
      digits[3] = 0x48;
      break;
    case '?':
      digits[3] = 0x83;
      break;
    case '!':
      digits[3] = 0x82;
      break;
    case '_':
      digits[3] = 0x08;
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
      if((key >= 0) && (key <= 0x10)){
       digits[3] = segtab[key];
      }
      else if((key >= '0') && (key <= '9')){ // Numbers
        digits[3] = numtab[key-0x30];
      }
      else if((key >= 'A') && (key <= 'Z')){ // "Upper case" alphabet
        digits[3] = alphaU[key-0x41];
      }
      else if((key >= 'a') && (key <= 'z')){
        digits[3] = alphaL[key-0x61];
      }
      else{
        digits[3] = 0x00;
      }
      break;
  }
}

void display_string_block(char *in){
  while(*in!='\0' && alive){
    display_char(*in);
    in++;
  }
}
void display_string_nblock(char *in){
  int i;
  while(*in!='\0' && alive && !button){
    display_char(*in);
    in++;
  }
  if(button){
    for(i=0;i<4;i++){
      digits[i] = 0;
    }
  }
}

/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios 	savetty;

void term_initio(){
  struct termios tty;

  tcgetattr(0, &savetty);
  tcgetattr(0, &tty);
  tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;
  tcsetattr(0, TCSADRAIN, &tty);
}

void term_exitio(){
  tcsetattr(0, TCSADRAIN, &savetty);
}

/*------------------------------------------------------------
 * serial I/O (8 bits, 1 stopbit, no parity, 38,400 baud)
 *------------------------------------------------------------
 */


int rs232_open(void){
  char 		*name;
  int 		result;  
  struct termios	tty;
  
  fd_RS232 = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
  assert(fd_RS232>=0);
  result = isatty(fd_RS232);
  assert(result == 1);
  name = ttyname(fd_RS232);
  assert(name != 0);
  result = tcgetattr(fd_RS232, &tty);
  assert(result == 0);

  tty.c_iflag = IGNBRK; /* ignore break condition */
  tty.c_oflag = 0;
  tty.c_lflag = 0;
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
  tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */
  cfsetospeed(&tty, B38400); /* set output baud rate */
  cfsetispeed(&tty, B38400); /* set input baud rate */
  tty.c_cc[VMIN]  = 0;
  tty.c_cc[VTIME] = 0;
  tty.c_iflag &= ~(IXON|IXOFF|IXANY);
  result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */
  tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}

int rs232_close(void){
  int 	result;
  result = close(fd_RS232);
  assert (result==0);
}
/*----------------------------------------------------------------
 * USB-PIO specific functions
 *----------------------------------------------------------------
 */
void setup_ports(){
  char str[4];
  write(fd_RS232,"@00D000\r",8); // Port A input
  usleep(SLEEP);                 // Needs time for reply
  usleep(SLEEP);
  read(fd_RS232,str,4);

  write(fd_RS232,"@00D1FF\r",8); // Port B output
  usleep(SLEEP);
  read(fd_RS232,str,4);

  write(fd_RS232,"@00D200\r",8); // Port C input
  usleep(SLEEP);
  read(fd_RS232,str,4);
}

void write_to_port(int port, BYTE bits){
  char str[8];

  snprintf(str,10,"@00P%d%02x\r",port,bits);
  write(fd_RS232,str,8);
  usleep(SLEEP);
  read(fd_RS232,str,4);
}
/*----------------------------------------------------------------
 * exit subroutine
 *----------------------------------------------------------------
 */
void closing_time() {
  alive = FALSE;
  pthread_join(keypad_thread, NULL);
  write_to_port(C, 0);     	// Last LED off
}
