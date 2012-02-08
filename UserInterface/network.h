#ifndef NETWORK_H_
#define NETWORK_H_

/* Lengths of data received from keypad thread */
#define PINLEN 4
#define TRACKLEN 18
#define PACKETLEN	20 /* Op Code = 1, Location = 1, MAC = 18 */

extern char task;
extern char data[];

#endif /* NETWORK_H_ */
