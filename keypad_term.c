/*****
 Keypad handling routines for Embedded Systems Design coursework
 22/12/2011 - Pete Hemery

 Terminal code from:
 http://www.st.ewi.tudelft.nl/~gemund/Courses/In4073/Resources/myterm.c
 used to send ASCII to USB PIO keypad
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
#define SLEEP 1400		// Lowest value needed between write and read
#define DELAY 50000000		// Key press delay
#define SCROLL_DELAY 60000000   // LED scrolling

#define PLAY    'A'
#define BACK    'B'
#define CANCEL  'C'
#define MENU    'D'
#define ENTER   'E'
#define FORWARD 'F'

typedef unsigned char BYTE;

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
const BYTE uitab[] = {0x00,'1','2', '3',FORWARD,
                           '4','5', '6',ENTER,
                           '7','8', '9',MENU,
                          PLAY,'0',BACK,CANCEL};

BYTE digits[COLSX] = {0x73,0x79,0x78,0x79};
pthread_t keypad_thread;
BYTE alive = TRUE;  // Used to exit while loops when Ctrl+C is caught
char button = FALSE;

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

  return result;
}

int rs232_close(void){
  int 	result;
  result = close(fd_RS232);
  assert (result==0);
  return result;
}
/*----------------------------------------------------------------
 * USB-PIO specific functions
 *----------------------------------------------------------------
 */
void setup_ports(){
  char str[4];
  write(fd_RS232,"@00D000\r",8); //Port A output
  usleep(SLEEP);                // Needs time for reply
  usleep(SLEEP);
  read(fd_RS232,str,4);

  write(fd_RS232,"@00D1FF\r",8); //Port B input
  usleep(SLEEP);
  read(fd_RS232,str,4);

  write(fd_RS232,"@00D200\r",8); //Port C output
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
  char str[6];
  char temp;
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
        out |= (0x0F & (str[4]-0x07));  // A-F
      }
      else{
        out |= (0x0F & (str[4]));       // 0-9
      }

      for(i=0; i<ROWSX; i++){           // Scan the rows for key presses
        if((out >> i) & 0x01){
          keypresses++;
          temp = uitab[((col+1)+(i*4))];// Set the detected button
        }
      }

      if(col == COLSX-1){ // After reading all the columns
        switch(keypresses){
          case 0:
            button=FALSE; // No key press detected
            break;
          case 1:
            button=temp;  // Write ASCII value from uitab
            break;
          default:
            button=ERROR; // Multiple keys pressed
            break;
        }
        keypresses = 0;
      }
    }
  }
  return 0;
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

void shift_digits(){
  scroll_delay();

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
  char *welcome="  Hello World..    ";
  int ret;
  BYTE button_read = 0;


  /* error handling - reset LEDs */
  atexit(closing_time);
  signal(SIGINT, closing_time);

  term_initio();
  rs232_open();
  
  setup_ports();
  ret = pthread_create( &keypad_thread, NULL, keypad, NULL);

//  delay();
//  display_string(welcome);

  while(alive){
    delay();
    button_read = button;
    if(button_read){
      printf("button: %c\n",button_read);
      switch(button_read){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
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
    }
  }

  pthread_join(keypad_thread, NULL);

  term_exitio();
  rs232_close();
  return 0;
}

