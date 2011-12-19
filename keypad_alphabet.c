/*****
 keypad strace tester to determine the calls to the ACM device
 18/12/2011 - Pete Hemery
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libad.h>
#include <sys/termios.h>
#include <ctype.h>      // gcc keypad.c -lad4

#define FALSE 0
#define TRUE 1

#define DIOA (AD_CHA_TYPE_DIGITAL_IO|1)
#define DIOB (AD_CHA_TYPE_DIGITAL_IO|2)
#define DIOC (AD_CHA_TYPE_DIGITAL_IO|3)

#define COLSX 4
#define ROWSX 4

#define BIGLOOP   4

typedef unsigned char BYTE;
typedef unsigned int DWORD;

BYTE portAstatus = 0;
BYTE portBstatus = 0;
BYTE portCstatus = 0;

int pio;

int inithw(void) {
  if((pio = ad_open("usb-pio"))==-1){
    printf("failed to open USB driver\n");
    exit(1); 
  };
  ad_set_line_direction(pio, 1, 0x00);  // A 4 col sel output
  ad_set_line_direction(pio, 2, 0xFF);  // B 4 row sense input
  ad_set_line_direction(pio, 3, 0x00);  // C 8 7-nseg output

  return;
}

void writeFAB (int np, BYTE wd) {
 
  ad_digital_out(pio, np, wd);
}

BYTE readFAB (int np) {
  int x;
  ad_digital_in(pio, np, &x);
  return (BYTE) x;
}

/******************
  7-Seg hex map
    --1--
   20   2
     40  
   10   4
    --8-- 80
*******************/

BYTE digits[COLSX] ={0,4,2,0};
const BYTE numtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,
		       0x07,0x7F,0x6F,0x77,0x7C,0x58,0x5E,0x79,0x71,0x40,0xD3};
const BYTE keytab[] = {1,4,7,10, 2,5,8,0, 3,6,9,11, 15,14,13,12};
/*                        A,   B,   C,   D,   E,   F,   G,   H,   I,   J,   K*/
const BYTE alphaU[] = {0x77,0x7C,0x39,0x5E,0x79,0x71,0x6F,0x76,0x30,0x1E,0x76,
/*    L,   M,   N,   O,   P,   Q,   R,   S,   T,   U,   V,   W,   X,   Y,   Z*/
   0x38,0x15,0x54,0x3F,0x73,0x67,0x50,0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};

void display_char(char key){
  if((key > 0x40) && (key < 0x5B)){
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = digits[3];
    digits[3] = alphaU[key-0x41];
  }
  else if((key > 0x60) && (key < 0x7B)){
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = digits[3];
    digits[3] = alphaU[key-0x61];
  }
  else if(key == 0x20){/*space*/
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = digits[3];
    digits[3] = 0x00;
  }
  else if(key == 0x2E){/* full stop . */
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = digits[3];
    digits[3] = 0x80;
  }
}

void display_number(int key){
  if((key >= 0) && (key < 10)){
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = digits[3];
    digits[3] = numtab[key];
  }
}

void display(void){
  int i;
  for(i=0;i<4;i++)
    {
      portAstatus = (BYTE) (01 << i);
      writeFAB(DIOC, 0);     // LEDS off	
      
      portCstatus = digits[i];
      writeFAB(DIOA, portAstatus);    // next col/digit sel
      writeFAB(DIOC, portCstatus);    // next LEDs pattern on
      // usleep(500); 
      //portBstatus = 0x0F & ~readFAB(DIOB);
    }
}

void display_string(char *in){
  int i,j;

  while(*in!='\0'){
    for(j=0;j<BIGLOOP;j++){
      display();
    }
    display_char(*in);
    in++;
    /*
    if(in == '\0')
    break;*/
  }
}

#define DELAY 8
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
  char *welcome="Hello World.\0";
  char* ptr;

  ptr = welcome;
  l = 0;
  inithw();
  /*
 for(i=0;i<4;i++){
   digits[i] = menu[i]-0x61;
   }*/

 /*
 digits[0]=alphaU[l++];
 digits[1]=alphaU[l++];
 digits[2]=alphaU[l++];
 digits[3]=alphaU[l++];
 */

 display_string(welcome);
 for(i=0;i<DELAY;i++)display();

 display_string(track);
 for(i=0;i<DELAY;i++)display();
 display_char('.');

 display_string(menu);
 for(i=0;i<DELAY;i++)display();
 display_char('.');

 display_string(info);
 for(i=0;i<DELAY;i++)display();
 display_char('.');

 display_string(time);
 for(i=0;i<DELAY;i++)display();
 display_char('.');

  while(1) {
    for(i=0;i<BIGLOOP;i++)display();

    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = digits[3];
    digits[3] = alphaU[l++];
      
    if (l == 26){
      display_char(' ');
      for(i=0;i<DELAY;i++)display();
      l=0;
      display_string(welcome);
      for(i=0;i<DELAY;i++)display();
    }
  }
  ad_close(pio);
  return 0;
}

