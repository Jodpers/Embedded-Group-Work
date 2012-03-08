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
#define IP "192.168.1.76"

#define TIMEOUTVALUE 3

void PANIC(char * msg);
void * receive(void);
int parsePacket(char * buffer);
void createHeaders(char opcode, char * localData);

#endif /* NETWORKLOCAL_H_ */