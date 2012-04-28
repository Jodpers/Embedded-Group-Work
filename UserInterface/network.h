/*
 * @file network.h
 *
 *  Created on 7 Feb 2012
 *     @author James Sleeman
 */

#ifndef NETWORK_H_
#define NETWORK_H_

/* Lengths of data received from keypad thread */
#define PINLEN 4
#define TRACKLEN 18
#define PACKETLEN 22 /* Op Code = 1, Location = 1, MAC = 18, newline = 1 */

/* for parsing and create headers state machine*/
#define EMERGENCY '4'
#define PIN '1'
#define PLAY '2'
#define TRACKINFO '3'
#define ACK '5'
#define NAK '6'
#define MULTICAST '7'

#define PASS '1'
#define FAIL '0'

extern char task;
extern char data[];
extern char * emergMsg;

extern int networkSetup();
#endif /* NETWORK_H_ */
