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
#define PACKETLEN 25 /* Op Code = 1, Location = 1, MAC = 18, newline = 1 */

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

#define PLAY_TRACK_ONLY '1'
#define PLAYLIST '2'
#define FIN_INDIV_TRACK '3'
#define FIN_PLAYLIST_TRACK '4'
#define END_OF_PLAYLIST '5'
#define CLOSEST_MAC_ADDRESS '6'

extern char task;
extern char data[];

extern int networkSetup();
#endif /* NETWORK_H_ */
