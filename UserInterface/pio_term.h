/*
 * @file pio_term.h
 *
 *  Created on 1 Feb 2012
 *     @author Pete Hemery
 *
 *  Terminal and PIO specific defines and prototypes.
 *
 *  terminal code used to send ASCII to USB PIO cable. From:
 *  http://www.st.ewi.tudelft.nl/~gemund/Courses/In4073/Resources/myterm.c
 */

#ifndef PIO_TERM_H_
#define PIO_TERM_H_

#define SERIAL_DEVICE 	"/dev/ttyACM0"
#define SLEEP 1400      /* Lowest value needed between write and read */

extern int fd_RS232;   /* Terminal File descriptor */

void setup_term();
void close_term();

void term_initio();
void term_exitio();
void rs232_open(void);
void rs232_close(void);

void setup_ports();
void write_to_port(int, unsigned char);

#endif /* PIO_TERM_H_ */

