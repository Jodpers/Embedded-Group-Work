/******************************************************************************************
 * gstClient.h                                                                            *
 * Author: James Sleeman                                                                  *
 *****************************************************************************************/

#ifndef GSTCLIENT_H_
#define GSTCLIENT_H_

//#define STANDALONE 

void killGst();
void playGst();
void pauseGst();
int getTimeGst(char * trackTime);
void seekGst();
void set_ip_and_port(char *ip_in, int port_in);

#ifndef STANDALONE
void * gst(void);
#endif

#endif
