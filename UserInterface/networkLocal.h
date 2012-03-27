/*
 * @file networkLocal.h
 *
 *  Created on 7 Feb 2012
 *     @author James Sleeman
 */

#ifndef NETWORKLOCAL_H_
#define NETWORKLOCAL_H_

#define MAXBUF  1024
#define MAXDATASIZE 1024 /* max number of bytes we can get at once */


/* for toplevel state machine */
#define CREATEHEADERS 1
#define WAITING 2
#define SEND 3
#define RECEIVE 4
#define PARSEPACKET 5

/* server info */
#define PORT "4444"
//#define IP "164.11.222.103"
//#define IP "164.11.222.88"

#define IP "localhost"

<<<<<<< HEAD
#define IPLEN 14

//#define IP "localhost"

#define TIMEOUTVALUE 3

void PANIC(char * msg);
void * receive(void);
int parsePacket(char * buffer);
void createPacket(char * localData);

#endif /* NETWORKLOCAL_H_ */   
