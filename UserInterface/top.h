/*
 * top.h
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#ifndef TOP_H_
#define TOP_H_

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;


#define COLS 	4
#define ROWS 	4

#define TRUE	1
#define FALSE 	0
#define ERROR 	-1

#define A 0
#define B 1
#define C 2

#define SLEEP 1400		/* Lowest value needed between write and read */

#define PIN_LEN	4
#define BUFFER_SIZE     50
#define BUFFER_MAX      BUF_SIZE - 1

extern BYTE alive;  	/* Exit condition of while loops */

extern char display_buffer[BUFFER_SIZE];
#endif /* TOP_H_ */
