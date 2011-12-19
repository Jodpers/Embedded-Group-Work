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
  usleep(100000);
  ad_set_line_direction(pio, 1, 0x00);  // A 4 col sel output
  usleep(100000);
  ad_set_line_direction(pio, 2, 0xFF);  // B 4 row sense input
  usleep(100000);
  ad_set_line_direction(pio, 3, 0x00);  // C 8 7-nseg output
  usleep(100000);

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
const BYTE segtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,
		       0x07,0x7F,0x6F,0x77,0x7C,0x58,0x5E,0x79,0x71,0x40,0xD3};
const BYTE keytab[] = {1,4,7,10, 2,5,8,0, 3,6,9,11, 15,14,13,12};
/*                        A,   B,   C,   D,   E,   F,   G,   H,   I,   J,   K*/
const BYTE alphaU[] = {0x77,0x7C,0x39,0x5E,0x79,0x71,0x6F,0x76,0x30,0x1F,0x76,
/*    L,   M,   N,   O,   P,   Q,   R,   S,   T,   U,   V,   W,   X,   Y,   Z*/
  0x78,0x15,0x54,0x3F,0x73,0x67,0x50,0x6D,0x77,0x3E,0x1C,0x2A,0x76,0x6E,0x5B}
int main () {
  inithw();

  portAstatus = (BYTE) (1);
  usleep(100000);
  writeFAB(DIOA, portAstatus);    // next col/digit sel
  usleep(100000);
  portCstatus = 0x080;
  usleep(100000);
  writeFAB(DIOC, portCstatus);    // next LEDs pattern on
  usleep(100000);
  portBstatus = 0x0F & ~readFAB(DIOB);

  ad_close(pio);
  return 0;
}

