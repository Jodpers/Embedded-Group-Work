/******************************************************************************************
 * networking.c                                                                           *
 * Description:                                                                           *
 * Globals: Inputs:                                                                       *
 *         Globals:                                                                       *
 * Author: James Sleeman                                                                  *
 * I have used code from http-client.c                                                    *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in                *
 * whole or in part in accordance to the General Public License (GPL).                    *
 *****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "client_v2.h"

char job;

struct sockaddr_in dest;  
struct addrinfo hints, *p, *servinfo;

int sockfd, numbytes;
int rv;
char s[INET6_ADDRSTRLEN];

char * buffer;
char * packet;

/*****************************************************************************************
 * Name:                                                                                 *
 * Description:                                                                          *
 * Inputs: Parameters:                                                                   *
 *            Globals:                                                                   *
 * Outputs: Globals:                                                                     *
 *           Return:                                                                     *
 ****************************************************************************************/
void netowrkingFSM()
{
  char c;
      setup();
      state = CREATEHEADERS;
    
      while (1)
	{      
	  switch (state)
	    {
	    case WAITING:
	      
	      break;
	    case CREATEHEADERS:
	      createHeaders();
	      printf("Packet to send:%s", packet); 
	      state = SEND;
	      break;
	    case SEND:
	      send(sockfd, packet, (MAXDATASIZE-1), 0);
	      state = RECEIVE;
	      break;
	    case RECEIVE:
	      receive();
	      -state = parsePacket();
	      break;
	    default:
	      printf("Unknown state\n");
	      state = WAITING;
	      break;
	    }
	}
         
      close(sockfd);
      
}
/*****************************************************************************************
 * Name:                                                                                 *
 * Description:                                                                          *
 * Inputs: Parameters:                                                                   *
 *            Globals:                                                                   *
 * Outputs: Globals:                                                                     *
 *           Return:                                                                     *
 ****************************************************************************************/
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/*****************************************************************************************
 * Name:                                                                                 *
 * Description:                                                                          *
 * Inputs: Parameters:                                                                   *
 *            Globals:                                                                   *
 * Outputs: Globals:                                                                     *
 *           Return:                                                                     *
 ****************************************************************************************/
int setup()
{

  memset(&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;
 hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0) 
    {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
    }
 
  /* loop through all the results and connect to the first we can */
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }
  if (p == NULL) 
    {
      fprintf(stderr, "client: failed to connect\n");
      return 2;
    }
}

/*****************************************************************************************
 * Name:                                                                                 *
 * Description:                                                                          *
 * Inputs: Parameters:                                                                   *
 *            Globals:                                                                   *
 * Outputs: Globals:                                                                     *
 *           Return:                                                                     *
 ****************************************************************************************/
int receive()
{
  buffer = malloc(1000);
  if ((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) 
    {
      perror("recv");
      exit(1);
    }
  printf("Packet received: %s\n", buffer);
  buffer[numbytes] = '\0';

}



 /****************************************************************************************
 * Name:                                                                                 *
 * Description: Determines wheather your request was successful e.g. Track request or    *
 *              logging in authentication.                                               *
 * Inputs: Parameters:                                                                   *
 *            Globals:                                                                   *
 * Outputs: Globals:                                                                     *
 *           Return:                                                                     *
 ****************************************************************************************/
int parsePacket()
{
  static int timeout = 3; 
  int state = 1;
  /*Checks that buffer isnt empty*/
  if(strlen(buffer))
    {
      switch (buffer[0])
        {
        case PIN:
          if (buffer[1] == 1)
            {
              loggedIn = 1;
              printf("PIN Authenticated");
            }
          else
            {
              loggedIn = 0;
              printf("PIN Authenticated failed");
            }	  
	  state = CREATEHEADERS;
	  job = ACK;
	  break;
        case TRACKINFO:
          printf("%s", buffer);
	  state = CREATEHEADERS;
	  job = ACK;
          break;
        case EMERGENCY:
          printf("Emergency, stop, leave the building!\n\n");
	  emergency = 1;
	  state = CREATEHEADERS;
	  job = ACK;
          break;
        case ACK: /* Do nothing */
          printf("%s", buffer);
          break;
        case NAK: /* Resends last packet */
          printf("%s", buffer);
	  if (timeout-- > 0)
	    {
	      state = SEND;
	    }
	  else
	    {
	      state = WAITING;
	    }
	  break;
        case MULTICAST: /* Passes port and IP to gstreamer */
          printf("%s", buffer);
	  state = CREATEHEADERS;
	  job = ACK;
          break;
	default:
          printf("Unknown packet:%s", buffer);
	  state = CREATEHEADERS;
	  job = NAK;
          break;
        }
      
    }
  return state;
}

/*****************************************************************************************
 * Name: void createHeaders()                                                            *
 * Description: Creates the headers to be sent to the server                             *
 * Inputs: Parameters:                                                                   *
 *            Globals:                                                                   *
 * Outputs: Globals:                                                                     *
 *           Return: None                                                                *
 ****************************************************************************************/
void createHeaders()
{
 
  char * pin;
  char * track;
  char * port;
  char * ip;
  

  pin = malloc(5);
  packet = malloc(6);
  memset (packet,' ',6);
 

  job = '1';

   switch (job)
        {
        case PIN:
           pin = "1245";
          sprintf(packet, "%c%s/0", job, pin); // pinpacket
          break;
        case PLAY:
        case TRACKINFO:
         sprintf(packet, "%c%s", job, track); // request packet, used for play and track info
          break;
        case ACK:
        case NAK:
          sprintf(packet, "%c", job);
          break;
        default:
	  printf("tried to create unknown packet\n");
          break;
        }
}
