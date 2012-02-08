#ifndef NETWORKING_H_
#define NETWORKING_H_

/* Lengths of data received from keypad thread */
#define PINLEN 4
#define TRACKLEN 18

int sendRequest(char requestType, char * data);

#endif /* NETWORKING_H_ */
