#ifndef NET_INTERNAL_H_
#define NET_INTERNAL_H_

#define MAXBUF  1024
#define MAXDATASIZE 1024 /* max number of bytes we can get at once */

/* for parsing and create headers state machine*/
#define PIN '1'
#define PLAY '2'
#define TRACKINFO '3'
#define EMERGENCY '4'
#define ACK '5'
#define NAK '6'
#define MULTICAST '7'

/* for toplevel state machine */
#define CREATEHEADERS 1
#define WAITING 2
#define SEND 3
#define RECEIVE 4

/* server info */
#define PORT "4444"
#define IP "192.168.12.2"

void PANIC(char * msg);
int receive();
int setup();
int parsePacket();
void createHeaders();

#endif /* NET_INTERNAL_H_ */
