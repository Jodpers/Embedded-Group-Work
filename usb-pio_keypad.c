/* Keypad (16 switches) scanner and 7-seg digit LED refresh tasks
 *   needs to be run at least every 20ms (50Hz minimum)
 *   Linux version, 28-2-04
 * Converted to use the BMCM USB-PIO interface  1-10-11
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libad.h>
#include <sys/termios.h>
#include <ctype.h>      // gcc keypad.c -lad4

#define DIOA (AD_CHA_TYPE_DIGITAL_IO|1)
#define DIOB (AD_CHA_TYPE_DIGITAL_IO|2)
#define DIOC (AD_CHA_TYPE_DIGITAL_IO|3)

#define COLSX 4
#define ROWSX 4

typedef unsigned char BYTE;
typedef unsigned int DWORD;

DWORD keyflags;
BYTE colsel=0;
BYTE kpd0[COLSX];
BYTE kpd1[COLSX];
BYTE kpd2[COLSX];
BYTE ktrue[COLSX];      //current true keypad status
BYTE kedge[COLSX];
BYTE kchange[COLSX];    //1 - key status changed
BYTE digits[COLSX] ={3,2,1,0};
const BYTE segtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,
	       0x07,0x7F,0x6F,0x77,0x7C,0x58,0x5E,0x79,0x71,0x40,0xD3};
const BYTE keytab[] = {1,4,7,10, 2,5,8,0, 3,6,9,11, 15,14,13,12};

BYTE portAstatus = 0;
BYTE portBstatus = 0;
BYTE portCstatus = 0;
int pio;

int inithw(void) {
//  int x;
  //  setterm();
  if((pio = ad_open("usb-pio"))==-1){

       printf("failed to open USB driver\n");
       exit(1); 
  };
  ad_set_line_direction(pio, 1, 0x00);  // A 4 col sel output
  ad_set_line_direction(pio, 2, 0xFF);  // B 4 row sense input
  ad_set_line_direction(pio, 3, 0x00);  // C 8 7-nseg output


  return 0;
}

void writeFAB (int np, BYTE wd) {
 
  ad_digital_out(pio, np, wd);
}

BYTE readFAB (int np) {
  int x;
  ad_digital_in(pio, np, &x);
  return (BYTE) x;
}


BYTE colscan(void) {
  int i;
  BYTE know;
  colsel = (++colsel) % COLSX;
  portAstatus = (BYTE) (01 << colsel);
  portCstatus = segtab[digits[colsel]];
  writeFAB(DIOC, 0);              // LEDS off
  writeFAB(DIOA, portAstatus);    // next col/digit sel
  //     usleep(500);
  writeFAB(DIOC, portCstatus);    // next LEDs pattern on

  portBstatus = 0x0F & ~readFAB(DIOB);

  kpd2[colsel] = kpd1[colsel];
  kpd1[colsel] = kpd0[colsel];
  kpd0[colsel] = (BYTE)portBstatus;
  
  kchange[colsel] = (kpd0[colsel]^kpd1[colsel]) | (kpd1[colsel]^kpd2[colsel]); 
  know = (kpd2[colsel] & ~kchange[colsel]) | (ktrue[colsel] & kchange[colsel]);
  //kedge[colsel] = (know^ktrue[colsel]) & know; 
  ktrue[colsel] = know;
  keyflags = 0;
  keyflags |=  ktrue[0] & 0x0F;
  keyflags |= (ktrue[1] & 0x0F) << 4;
  keyflags |= (ktrue[2] & 0x0F) << 8;
  keyflags |= (ktrue[3] & 0x0F) << 12;
  keyflags &= 0x0000ffff;
  for(i=0; keyflags&1; keyflags>>=1) i++;
   return i;
}
int main () {
  struct tms *tim;
  BYTE key;
  DWORD then;

  inithw();
  while (1) {
	key = colscan();
	if (key <16) {
		digits[0]= keytab[key];
		digits[1]= 16;
		digits[2]= key / 10;
		digits[3]= key % 10;
	} else {
		digits[0]= 17;
		digits[1]= 16;
		digits[2]= 0;
		digits[3]= 0;
	}
//	printf("  key = %x \n",key );
	then = (DWORD)times(&tim)+1000000;
	 while (then > (DWORD)times(&tim)) { };
  }
  ad_close(pio);
    return 0;
}

