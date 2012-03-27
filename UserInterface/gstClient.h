/******************************************************************************************
 * gstClient.h                                                                            *
 * Author: James Sleeman                                                                  *
 *****************************************************************************************/

#ifndef GSTCLIENT_H_
define GSTCLIENT_H_

//#define STANDALONE 

void killGst();
void playGst();
void pauseGst();
char * getTimeGst();
void seekGst()

#ifndef STANDALONE
int gst(int port, char ip[])
#endif

#define TRUE 1
#define FALSE 0
