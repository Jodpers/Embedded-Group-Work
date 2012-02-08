/*
 * top.h
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#ifndef TOP_H_
#define TOP_H_

#define DEBUG	1

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;


#define COLSX 	4
#define ROWSX 	4

#define TRUE	1
#define FALSE 	0
#define ERROR 	-1

#define A 0
#define B 1
#define C 2

#define SLEEP 1400		/* Lowest value needed between write and read */

#define PIN_LEN	4

extern BYTE alive;  	/* Exit condition of while loops */

#endif /* TOP_H_ */
