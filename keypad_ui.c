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
#include <fcntl.h>   //file descriptors
#include <ctype.h>
#include <termios.h>  //terminal
#include <assert.h>
#include <time.h>
#include <sys/types.h>  //threads
#include <pthread.h>
#include "keypad_ui.h"

pthread_t keypad_thread;
BYTE alive = TRUE;  // Exit condition of while loops
char button = FALSE;  // Button pressed 1-16 or -1 for multiple buttons
int fd_RS232;   // Terminal File descriptor
int state = WAITING_LOGGED_OUT;  // State machine variable
int logged_in = FALSE;  // Client connected to server
BYTE digits[COLSX] = {0x00,0x00,0x00,0x00};
char buffer[BUF_SIZE] = {0};
char buffer_cnt = 0;
char buffer_pos = 0;
char cur_pos = 0;
BYTE cursor = 0;
char authentication = FALSE;


/*----------------------------------------------------------------
 * main -- Open terminal and launch keypad thread.
 *         Check for button input and respond appropriately
 *----------------------------------------------------------------
 */

int main (void) {
//  int i;
  int ret;
  char *emergency="! EMERGENCY !";
  char *welcome="Welcome.";
  char *enter_pin="Please Enter PIN.";
  char button_read = FALSE;  // Local snapshot of Global 'Button'

  /* error handling - reset LEDs */
  atexit(closing_time);
  signal(SIGINT, closing_time);

  term_initio();
  rs232_open();
  
  setup_ports();
  ret = pthread_create( &keypad_thread, NULL, keypad, NULL);

  display_string_nblock(welcome);

  while(alive){
    if(state == EMERGENCY){
        display_string_block(emergency);
      continue;
    }
    switch(state){
      case WAITING_LOGGED_OUT:
        display_string_nblock(enter_pin);
        digits[0] = 0x80;  // Set cursor position
        while(!button && alive && state == WAITING_LOGGED_OUT); // Just Wait
        
        if(button >= '0' && button <= '9'){
          state = INPUTTING_PIN; // Fall through to next state
        }
        else{
          break;
        }
      case INPUTTING_PIN:
        if(button_read = button){ // Intentionally Assignment
          input_pin(button_read);  // Sends a snapshot of button
        }
        cursor_blink();
        break;
      case WAITING_LOGGED_IN:
        display_string_block("Enter Track Number.");
        digits[0] = 0x80;  // Set cursor position
        while(!button && alive && state == WAITING_LOGGED_IN); // Just Wait
        if(button >= '0' && button <= '9'){
          state = INPUTTING_TRACK_NUMBER; // Fall through to next state
        }
        else if(button == ENTER_MENU){
          state = MENU_SELECT; // Fall through to next state
          break;
        }
        else{
          break;
        }
      case INPUTTING_TRACK_NUMBER:
        display_string_block("INPUTTING =)");
        cursor_blink();
        state = WAITING_LOGGED_IN;
        break;
      case MENU_SELECT:
        display_string_block("MENU.");
        state = WAITING_LOGGED_IN;
        break;
      default:
        break;
    }
    delay();
  }


  pthread_join(keypad_thread, NULL);

  term_exitio();
  rs232_close();
  return 0;
}
/*----------------------------------------------------------------
 * User Interface State Machines
 *----------------------------------------------------------------
 */

/*  INPUTTING_PIN */

void input_pin(char button_read){
  int i;
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
    if(buffer_cnt != cur_pos){  // If cursor isn't at the end
      buffer_cnt--;  // Decrement it before next line, lest we wrongly add 1
    }
    if(++buffer_cnt > 4){ // Button has been pressed, let's up the buffer count
      buffer_cnt = 4; // Limit PIN to 4 digits
      cur_pos = 4; // Cursor is off the edge of the array and won't blink
      buffer[cur_pos] = 0; // Null terminate string containing PIN
    }
    else{   // Less than 4 digits already? Well, get it in there!
      digits[cur_pos] = numtab[button_read-'0'];// Display the digit pressed
      buffer[cur_pos] = button_read; // Copy ASCII button press
      cur_pos = buffer_cnt; // Point at next empty location
      cursor_blink();  // Clear last cursor and move to next empty space
      delay();  // Wait to make sure button has stopped being pressed
    }
//  printf("buffer: %s\nbuffer_cnt: %d\ncur_pos: %d\n",buffer,buffer_cnt,cur_pos);
    break;
    
  case ACCEPT_PLAY:
  case ENTER_MENU:
    if(cur_pos != buffer_cnt){  // Ignore key if cursor isn't at the end
      cur_pos = buffer_cnt;
    }
    if(buffer_cnt < 4){
      display_string_block("PIN too short.");
    }
    else{
      authentication = TRUE;//check_pin(buffer,strlen(buffer));
      if(authentication == TRUE){
        display_string_nblock("Logged In.");
        display_string_nblock(buffer);
      }
      else{
        display_string_nblock("Invalid PIN.");
      }
      state = WAITING_LOGGED_IN;
      reset_buffer();
    }
    break;

  case CANCEL:
    reset_buffer();
    state = WAITING_LOGGED_OUT; // Go back to waiting
    break;

  case FORWARD:  // Move the cursor forward 1 digit
    if(buffer_cnt){
      if(++cur_pos>buffer_cnt){ // Scroll as far as 1 digit past last input
        cur_pos = buffer_cnt;
      }
    }
    cursor_blink(); // Update Cursor Position
//    printf("cursor_position: %d\n",cur_pos);
    break;
    
  case BACK:
    if(--cur_pos<=0){
      cur_pos=0;
    }
    cursor_blink();
//    printf("cursor_position: %d\n",cur_pos);
    break;
    
  case DELETE:
    if (cur_pos == 4 || cur_pos == buffer_cnt){
      buffer[--buffer_cnt] = 0;
      digits[cur_pos] &= 0x7F;
      digits[--cur_pos] = 0;
      cursor_blink();
    }
    else{
      digits[cur_pos] &= 0x7F;
      for(i=cur_pos;i<4;i++){
        digits[i] = digits[i+1];
        buffer[i] = buffer[i+1];
      }
      buffer[buffer_cnt--] = 0;
      digits[3] = 0;
    }
    if (!buffer_cnt || cur_pos < 0){
      cur_pos = 0;
      buffer_cnt = 0;
      buffer[buffer_cnt] = 0;
    }
//  printf("buffer: %s\nbuffer_cnt: %d\ncur_pos: %d\n",buffer,buffer_cnt,cur_pos);
    break;
  default:
    break;
  }
}

/*----------------------------------------------------------------
 * keypad thread - this function continiously outputs to LEDs and reads buttons
 *----------------------------------------------------------------
 */
void * keypad(){
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

void reset_buffer(void){ // Reset everything
  int i;
  for(i=0;i<BUF_SIZE;i++){
    buffer[i]=0;
  }
  buffer_cnt = 0;
  cur_pos = 0;

  for(i=0;i<4;i++){ // Clear the LEDs
    digits[i]=0;
  }
}

void cursor_blink(){
  static DWORD timer = CUR_TRIGGER;
  static BYTE prev_pos = 0;

  if(cur_pos != prev_pos){ // Cursor has moved
    digits[prev_pos] &= 0x7F; // Clear the old cursor
    if(cur_pos < 4){  // Unless pointing off the edge of the display
      digits[cur_pos] ^= 0x80; // Toggle the cursor
    }
    prev_pos = cur_pos; // Remember where cursor is
    timer = CUR_TRIGGER; 
  }

  timer--;      // Only toggle every few loops
  if(!timer){   // Gives the blinking effect
    timer = CUR_TRIGGER;
    if(cur_pos < 4){ // Unless pointing off the edge of the display
      digits[cur_pos] ^= 0x80;
    }
  }
}

/*------------------------------------------------------------
 * Display Digit Shifting Routines
 *------------------------------------------------------------
 */
void shift_digits_left(){
  int i;
  int buf_len;
  
  for(i=0;i<SCROLL_DELAY;i++); // Delay of text moving across the display
  digits[cur_pos] &= 0x7F;  // Clear the cursor
  buf_len = strlen(buffer);
  if(buf_len < BUF_SIZE){
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

void shift_digits_right(){
}

void shift_digits(){
  int i;
  digits[cur_pos] &= 0x7F;
  for(i=0;i<3;i++){
    digits[i] = digits[i+1];
  }
  digits[3] = 0;
}
/*------------------------------------------------------------
 * Display Char Routine
 *------------------------------------------------------------
 */
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
/*------------------------------------------------------------
 * Display String Routines - Blocking and Non-Blocking
 *------------------------------------------------------------
 */
void display_string_block(char *in){
  int i;
  BYTE temp[4];
  
  for(i=0;i<4;i++){
   temp[i]=digits[i]; // Save current digits
   digits[i]=0;  // Clear digits before displaying message
  }
  
  while(*in!='\0' && alive){
    display_char(*in);
    in++;
    scroll_delay(); // Blocking method
  }
  
  for(i=0;i<4;i++){ // Finish Scrolling the message off
    shift_digits();
    scroll_delay();
  }
  
  for(i=0;i<4;i++){
    digits[i]=temp[i]; // Replace saved digits
  }
}

char display_string_nblock(char *in){
  int i;
  BYTE temp[4];

  if(button)  // Routine called by a button press
    while(button && alive);  // Wait until not pressed
        
  for(i=0;i<4;i++){
   temp[i]=digits[i]; // Save current digits
   digits[i]=0;  // Clear digits before displaying message
  }
    
  while(*in!='\0' && alive && !button){
    display_char(*in);
    in++;
    i = SCROLL_DELAY; // Non-blocking method
    while(--i && alive && !button); // Delay of text moving across the display
  }
  
  if(button){
    for(i=0;i<4;i++){
      digits[i]=temp[i]; // Replace saved digits
    }
    return TRUE;  // Interrupted
  }
  
  for(i=0;i<4;i++){ // Finish Scrolling the message off
    shift_digits();
    scroll_delay(); // Blocking method, just for the end bit
  }
  
  for(i=0;i<4;i++){
    digits[i]=temp[i]; // Replace saved digits
  }
  return FALSE;   // Uninterrupted
}

/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios  savetty;

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
  char   *name;
  int   result;  
  struct termios tty;
  
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
  int  result;
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
  write_to_port(C, 0);      // Last LED off
}
