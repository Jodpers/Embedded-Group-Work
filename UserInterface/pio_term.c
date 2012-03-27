/*
 * @file pio_term.c
 *
 *  Created on 1 Feb 2012
 *     @author Pete Hemery
 *
 *  Terminal and PIO specific defines and prototypes.
 *
 *  terminal code used to send ASCII to USB PIO cable. From:
 *  http://www.st.ewi.tudelft.nl/~gemund/Courses/In4073/Resources/myterm.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>   /* file descriptors */
#include <ctype.h>
#include <termios.h>  /* terminal */
#include <assert.h>
#include <sys/types.h>  //threads
#include <pthread.h>

#include "pio_term.h"

int fd_RS232;   /* Terminal File descriptor */

void setup_term(){
  term_initio();
  rs232_open();
  setup_ports();
}

void close_term(){
  term_exitio();
  rs232_close();
}

/*------------------------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------------------------
 */
struct termios  savetty;

void term_initio(void){
  struct termios tty;

  tcgetattr(0, &savetty);
  tcgetattr(0, &tty);
  tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;
  tcsetattr(0, TCSADRAIN, &tty);
}

void term_exitio(void){
  tcsetattr(0, TCSADRAIN, &savetty);
}
/*------------------------------------------------------------------------------
 * serial I/O (8 bits, 1 stopbit, no parity, 38,400 baud)
 *------------------------------------------------------------------------------
 */
void rs232_open(void){
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

void rs232_close(void){
  int  result;
  result = close(fd_RS232);
  assert (result==0);
}

/**
 *  @brief Setup USB PIO Ports.
 *
 *    This function is USB-PIO specific.
 *    It sets up the 3 ports of the USB-PIO cable
 *    so that ports A and C are input and port B is output.
 *
 *  @param Void.
 *  @return Void.
 */
void setup_ports(void){
  char str[4];
  write(fd_RS232,"@00D000\r",8); /* Port A input */
  usleep(SLEEP);                 /* Needs time for reply */
  usleep(SLEEP);
  read(fd_RS232,str,4);

  write(fd_RS232,"@00D1FF\r",8); /* Port B output */
  usleep(SLEEP);
  read(fd_RS232,str,4);

  write(fd_RS232,"@00D200\r",8); /* Port C input */
  usleep(SLEEP);
  read(fd_RS232,str,4);
}

/**
 *  @brief Write to USB PIO Port.
 *
 *    This function is USB-PIO specific.
 *    It sends the unsigned char value of bits to the port
 *    specified in the int port.
 *
 *  @param [in] port The port number to send 'bits' to.
 *  @param [in] bits Unsigned char representing a
 *              value to display on the 7 Segment LEDs.
 *  @return Void.
 */
void write_to_port(int port, unsigned char bits){
  char str[10];

  snprintf(str,10,"@00P%d%02x\r",port,bits);
  write(fd_RS232,str,8);
  usleep(SLEEP);
  read(fd_RS232,str,4);
}
