/*****
 Terminal and PIO specific defines and prototypes
 01/02/2012 - Pete Hemery
 
 terminal code used to send ASCII to USB PIO cable. From:
 http://www.st.ewi.tudelft.nl/~gemund/Courses/In4073/Resources/myterm.c 
*****/

#ifndef PIO_TERM_H_
#define PIO_TERM_H_

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

#include "top.h"

#define SERIAL_DEVICE 	"/dev/ttyACM0"

extern int fd_RS232;   /* Terminal File descriptor */

void setup_term();
void close_term();

void term_initio();
void term_exitio();
void rs232_open(void);
void rs232_close(void);

void setup_ports();
void write_to_port(int, BYTE);

void closing_time(void);

#endif /* PIO_TERM_H_ */

