/*****
 terminal code from:  http://www.st.ewi.tudelft.nl/~gemund/Courses/In4073/Resources/myterm.c
 used to send ASCII to USB PIO keypad
 20/12/2011 - Pete Hemery
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>	//file descriptors
#include <ctype.h>
#include <termios.h>	//terminal
#include <assert.h>
#include <time.h>
#include <sys/types.h> 	//threads
#include <pthread.h>

#define FALSE 0
#define TRUE 1
#define ERROR -1

#define COLSX 4
#define ROWSX 4

#define SERIAL_DEVICE 	"/dev/ttyACM0"
#define A 0
#define B 1
#define C 2
#define SLEEP 1300
#define DELAY 50000000
#define SCROLL_DELAY 40000000

#define PLAY    'A'
#define BACK    'B'
#define CANCEL  'C'
#define MENU    'D'
#define ENTER   'E'
#define FORWARD 'F'

typedef unsigned char BYTE;
typedef unsigned int DWORD;

/******************
  7-Seg hex map
    --1--
   20   2
     40  
   10   4
    --8-- 80
*******************/
BYTE digits[COLSX] = {0x73,0x79,0x78,0x79};
const BYTE segtab[] = {0x00,0x06,0x5B,0x4F,0x71,0x66,0x6D,0x7D,0x79,
		       0x07,0x7F,0x6F,0x5E,0x77,0x3F,0x7C,0x39};
const BYTE keytab[] = {1,4,7,10, 2,5,8,0, 3,6,9,11, 15,14,13,12};
const BYTE uitab[] = {0x00,1, 2, 3, FORWARD, 4, 5, 6, ENTER, 7, 8, 9, MENU, PLAY, 0, BACK, CANCEL};
/*                        A,   B,   C,   d,   E,   F,   g,   H,   I,   J,   K*/
const BYTE alphaU[] = {0x77,0x7F,0x39,0x5E,0x79,0x71,0x6F,0x76,0x30,0x1E,0x76,
/*                        L,   M,   n,   O,   P,   Q,   r,   S,   t,   U,   V,   W,   X,   Y,   Z*/
		       0x38,0x15,0x54,0x3F,0x73,0x67,0x50,0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};
/*                        A,   b,   c,   d,   E,   F,   g,   h   i,   J,   K*/
const BYTE alphaL[] = {0x77,0x7C,0x58,0x5E,0x79,0x71,0x6F,0x74,0x04,0x1E,0x76,
/*                        L,   M,   n,   o,   P,   Q,   r,   S,   t,   U,   V,   W,   X,   Y,   Z*/
		       0x38,0x15,0x54,0x5C,0x73,0x67,0x50,0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};

pthread_t keypad_thread;
BYTE alive = TRUE;
short button = FALSE;

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
int fd_RS232;  // Terminal File descriptor

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
  char out[4];
  write(fd_RS232,"@00D000\r",8); //Port A output
  usleep(SLEEP);                // Needs time for reply
  usleep(SLEEP);
  read(fd_RS232,out,4);

  write(fd_RS232,"@00D1FF\r",8); //Port B input
  usleep(SLEEP);
  read(fd_RS232,out,4);

  write(fd_RS232,"@00D200\r",8); //Port C output
  usleep(SLEEP);
  read(fd_RS232,out,4);
}

void write_to_port(int port, BYTE bits){
  char str[8];

  snprintf(str,10,"@00P%d%02x\r",port,bits);
  write(fd_RS232,str,8);
  usleep(SLEEP);
  read(fd_RS232,str,4);
}
/*----------------------------------------------------------------
 * error handling - exit subroutine
 *----------------------------------------------------------------
 */
void closing_time() {
  alive = FALSE;
  pthread_join(keypad_thread, NULL);
  write_to_port(C, 0);     			// LEDS off
}
/*----------------------------------------------------------------
 * keypad thread - this function continiously outputs to LEDs and reads buttons
 *----------------------------------------------------------------
 */
void * keypad(){
  int i;
  int col;
  char str[5];
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

void delay(){
  int i;
  for(i=0;i<DELAY;i++);
}

void scroll_delay(){
  int i;
  for(i=0;i<SCROLL_DELAY;i++);
}

void shift_digits(){
  scroll_delay();

  digits[0] = digits[1];
  digits[1] = digits[2];
  digits[2] = digits[3];
}

void display_char(char key){
  if((key >= 0) && (key <= 0x10)){
    shift_digits();
    digits[3] = segtab[key];
  }
  else if(key == 0x20){                  //space
    shift_digits();
    digits[3] = 0x00;
  }
  else if(key == 0x2D){                  // hyphen -
    shift_digits();
    digits[3] = 0x40;
  }
  else if(key == 0x2E){                  // full stop .
    shift_digits();
    digits[3] = 0x80;
  }
  else if(key == 0x3D){                  // =
    shift_digits();
    digits[3] = 0x48;
  }
  else if(key == 0x3F){                  // ?
    shift_digits();
    digits[3] = 0xD3;
  }
  else if(key == 0x5F){                  // _
    shift_digits();
    digits[3] = 0x08;
  }
  else if((key == 0x28)||(key == 0x5B)||(key == 0x7B)){ // [
    shift_digits();
    digits[3] = 0x39;
  }
  else if((key == 0x29)||(key == 0x5D)||(key == 0x7D)){ // ]
    shift_digits();
    digits[3] = 0x0F;
  }
  else if((key > 0x40) && (key < 0x5B)){ // "Upper case" alphabet
    shift_digits();
    digits[3] = alphaU[key-0x41];
  }
  else if((key > 0x60) && (key < 0x7B)){ // "Lower case" alphabet
    shift_digits();
    digits[3] = alphaL[key-0x61];
  }
}

void display_string(char *in){
  while(*in!='\0' && alive){
    display_char(*in);
    in++;
  }
}

/*----------------------------------------------------------------
 * main -- Open terminal and launch keypad thread.
 *         Check for button input and respond appropriately
 *----------------------------------------------------------------
 */

int main () {
  char key;
  int i, l;

  BYTE know;
  DWORD keyflags;
  BYTE colsel=-1;
  char *menu="menu";
  char *info="info";
  char *track="track";
  char *time="time";
  char *welcome="Hello World? =] R0FL .-_-.\0";
  char* ptr;
  int ret;
  BYTE button_read = 0;

  ptr = welcome;
  l = 0;

  /* error handling - reset led's and close file descriptor & terminal io */
  atexit(closing_time);
  signal(SIGINT, closing_time);

  term_initio();
  rs232_open();
  
  setup_ports();
  ret = pthread_create( &keypad_thread, NULL, keypad, NULL);

  while(alive){
//    display_string(welcome);
    delay();

    button_read = button;
    if(button_read){
      if(button_read > 0x11){
	display_char('.');
//	printf("button = %d\n",button_read);
      }
      else{
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
	    display_char(button_read);
	    break;
	  case PLAY:
	    display_string("Play");
	    break;
	  case BACK:
	    display_string("Back");
	    break;
	  case CANCEL:
	    display_string("Canl");
	    break;
	  case MENU:
	    display_string("Menu");
	    break;
	  case ENTER:
	    display_string("Entr");
	    break;
	  case FORWARD:
	    display_string("Ford");
	    break;
	  default:
	    break;
	}
//	printf("button_read = %d\n",button_read);
//	printf("uitab = %d\n",uitab[button_read]);
      }
    }
  }

  pthread_join(keypad_thread, NULL);

  term_exitio();
  rs232_close();
  return 0;
}

