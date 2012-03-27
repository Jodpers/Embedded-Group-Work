/******************************************************************************************
 * networking.c                                                                           *
 * Description:                                                                           *
 * External globals: int task: This is used to inform a thread what to do with the data   *
 *                             passed to/from a thread.                                   *
 *                   char data[]: This is used to transfer data between threads           *
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

#include <sys/types.h>  //threads
#include <pthread.h>
#include <netinet/tcp.h>

#include "top.h"
#include "network.h"
#include "networkLocal.h"
#include "threads.h"
#include "debug.h"

char task;
char data[PACKETLEN] = {0}; /* Setting to NULL */

struct sockaddr_in dest;  
struct addrinfo hints, *p, *servinfo;

int sockfd, numbytes;
int rv;
char s[INET6_ADDRSTRLEN];

char receivedPacket[MAXDATASIZE] = {0};
char packet[PACKETLEN] = {0};

char opcode;
int sentt = 0;

/*****************************************************************************************
 * Name: networkingFSM                                                                   *
 * Description: This state machine will control the networking section of the client     *
 * Inputs: Parameters: None                                                              *
 *            Globals: alive: This is an exit condition for while loops                  *
                       task: This is used to inform a thread what to do with the data    *
 *                           passed to/from a thread.                                    *
 *                     data[]: This is used to transfer data between threads             *
 * Outputs: Globals: task and data above are also outputs                                *
 *           Return: None                                                                *
 ****************************************************************************************/
void * networkingFSM(void)
{
  char state = WAITING;
  char localData[PACKETLEN] = {0};
  char localRecPacket[MAXDATASIZE]  = {0};
  int len = 0;

  while (alive)
    {      
      
      switch (state)
	{
	  //This thread will sleep after entering this state
	case WAITING:	  
	  pthread_mutex_lock(&network_Mutex);
	  pthread_cond_wait(&network_Signal, &network_Mutex);
	  opcode = task;
	  strncpy(localRecPacket, receivedPacket, PACKETLEN);
	  pthread_mutex_unlock(&network_Mutex);
	  
	  if (opcode == RECEIVE)
	    {
	      printd("sentt:%d\n", sentt);
	      if (sentt == 0) /* This was added, as some times an empty packet was being 
                                 sent after the socket was init'd */ 
		{
		  /* Do nothing receiving packet with out sending*/
		}
	      else
		{
		  state = PARSEPACKET;
		}

	    case WAITING:

	    	pthread_mutex_lock(&network_Mutex);
	    	pthread_cond_wait(&network_Signal, &network_Mutex);
	    	opcode = task;
	    	strncpy(localRecPacket, receivedPacket, PACKETLEN);
	    	pthread_mutex_unlock(&network_Mutex);
	    	if (opcode == RECEIVE)
			{
			  printd("sent:%d\n", sentt);
			  if (sentt == 0)
			    {
			      /* Do nothing receiving packet with out sending*/
			    }
			  else
			    {
		      state = PARSEPACKET;
			    }
	    	  break;
			}
			pthread_mutex_lock(&request_Mutex);
			strncpy(localData, data, PACKETLEN);
			opcode = task;
			pthread_mutex_unlock(&request_Mutex);

	    case CREATEHEADERS:
	      createHeaders(opcode,localData);

	      printd("Packet to send:%s", packet);

	      state = SEND;
	      break;
	    case SEND:

	      printd("packet at sending time:%s\n", packet);
	      len = strlen(packet);
	      printd("len: %d\n", len);
		if (len > 0)
		  {
		    send(sockfd, packet, len+1, 0);
		    printd("sent\n");
		  }
	      state = WAITING;
>>>>>>> 71278dec2815463d54bc26ea9c785eb306d1feea
	      break;
	    }            //Will drop through if UI has woke the thread up
	  
	  pthread_mutex_lock(&request_Mutex);
	  strncpy(localData, data, PACKETLEN);
	  opcode = task;
	  pthread_mutex_unlock(&request_Mutex);
	  
	case CREATEHEADERS:
	  createPacket(localData);
	  printd("Packet to send:%s", packet);
	  state = SEND;
	  break;

	case SEND:
	  printd("packet at sending time:%s\n", packet);
	  len = strlen(packet);
	  printd("len: %d\n", len);
	  
	  if (len > 0)
	    {
	      send(sockfd, packet, len+1, 0);
	      printf("sent\n");
	    }
	  state = WAITING;
	  break;

	case PARSEPACKET:
	  state = parsePacket(localRecPacket);
	  break;

	default:         /* Should never happen... */
	  printd("Unknown state\n");
	  state = WAITING;
	  break;
	}
    }
         
  close(sockfd);
  return 0;
}
/*****************************************************************************************
 * Name: get_in_addr                                                                     *
 * Description: This function will get the address for IPv4 or IPv6                      *
 * Inputs: Parameters: sockaddr: this will contain the address                           *
 *            Globals: None                                                              *
 * Outputs: Globals: None                                                                *
 *           Return: Returns a pointer to the address                                    *
 ****************************************************************************************/
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*****************************************************************************************
 * Name: networkSetup                                                                    *
 * Description: This function will initialise the socket, bind and connect.              *
 * Inputs: Parameters: None                                                              *
 *            Globals: None                                                              *
 * Outputs: Globals: None                                                                *
 *           Return: 0 - Passed, 1 - Failed to get infomation about the address,         *
 *                   2 - Failed to connect                                               *
 ****************************************************************************************/
int networkSetup()
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
  for(p = servinfo; p != NULL; p = p->ai_next) 
    {

      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
	{
	  perror("client: socket");
	  continue;
	}
      
      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
	{
	  close(sockfd);
	  perror("client: connect");
	  continue;
	}
      break;
    }
  if (p == NULL) 
    {
      //  fprintf(stderr, "client: failed to connect\n");
      return 2;
    }
  
  return 0;
}

/*****************************************************************************************
 * Name: receive                                                                         *
 * Description: This function will wait to receive a packet, once the packet has been    *
 *              received the packet will be saved, and the function will go back to      *
 *              waiting for the next packet to be received                               *
 * Inputs: Parameters: None                                                              *
 *            Globals: alive: This is an exit condition for while loops                  *
                       task: This is used to inform a thread what to do with the data    *
 *                           passed to/from a thread.                                    *
 *                     data[]: This is used to transfer data between threads             *
 * Outputs: Globals: task and data above are also outputs                                *
 *           Return: None                                                                *
 ****************************************************************************************/
void * receive(void)
{
  while(alive)
    {
      char buffer[MAXDATASIZE];
      if ((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) // Blocking if statement
	{
	  perror("recv");
	  alive = FALSE;
	  break;
	}

      printd("Packet received: %s\n", buffer);      
      buffer[numbytes] = '\0';
      
      pthread_mutex_lock(&network_Mutex);
      strncpy(receivedPacket, buffer, numbytes);
      task = RECEIVE;
      pthread_cond_signal(&network_Signal);
      pthread_mutex_unlock(&network_Mutex);
    }
}

/****************************************************************************************
 * Name: parsePacket                                                                    *
 * Description: Determines whether your request was successful e.g. Track request or    *
 *              logging in authentication.                                              *
 * Inputs: Parameters: buffer: This will contain the packet to parse                    *
 *            Globals: None                                                             *
 * Outputs: Globals: data[]: This is used to transfer data between threads              *
 *                   opcode: This will set the opcode for packet creation               *
 *           Return: state: This will change the state of the Networking FSM            *
 ****************************************************************************************/
int parsePacket(char * buffer)
{
  
  static int timeout = TIMEOUTVALUE;
  int state = 1;
  char loggedIn;
  char emergency = '0';

  char tmp[5];
  int portGst;
  char ipGst[16];

  printd("buffer:%s\n",buffer);

  /*Checks that buffer isn't empty*/
  if(strlen(buffer))
    {
      switch (buffer[0])
        {
        
	case PIN:
	  printd("buffer[1]:%c\n",buffer[1]);

          if (buffer[1] == PASS)
            {
              loggedIn = PASS;      
              printd("PIN Authenticated\n");
            }
          else
            {
              loggedIn = FAIL;
              printd("PIN failed\n");

            }
	  
	  //returns the result of the log in attempt to the UI
	  pthread_mutex_lock(&request_Mutex);
	  data[0] = loggedIn;
	  pthread_cond_signal(&request_Signal);
	  pthread_mutex_unlock(&request_Mutex);

	  state = CREATEHEADERS;
	  opcode = ACK;
	  break;
	  
	case PLAY:
	  if (buffer[1] == FAIL)
	    {
	      state = CREATEHEADERS; 
	      opcode = ACK;
	      
	      pthread_mutex_lock(&request_Mutex);
	      data[0] = FAIL;
	      pthread_cond_signal(&request_Signal);
	      pthread_mutex_unlock(&request_Mutex);
	    }
	  break;

        case TRACKINFO:
          printd("%s", buffer);	  
	  state = CREATEHEADERS;
	  opcode = ACK;
          break;

        case EMERGENCY:
          printd("Emergency, stop, leave the building!\n\n");
	  emergency = 1;

	  pthread_mutex_lock(&state_Mutex);
	  data[0] = emergency;
	  pthread_cond_signal(&state_Signal);
	  pthread_mutex_unlock(&state_Mutex);
	  state = CREATEHEADERS;
	  opcode = ACK;
          break;

        case ACK: /* Do nothing */
          printd("%s", buffer);
          state = WAITING;
          break;
        
	case NAK: /* Resends last packet */
          printd("%s", buffer);

	  if (timeout-- > 0) // Stops resending after it gets bored.
	    {
	      state = SEND;
	    }
	  else
	    {
	      timeout = TIMEOUTVALUE;
	      state = WAITING;
	    }
	  break;

        case MULTICAST: /* Passes port and IP to gstreamer */
          printd("%s", buffer);

	  portGst = atoi(tmp); 

	  state = CREATEHEADERS;
	  opcode = ACK;
          break;

	default:
          printd("Unknown packet:%s", buffer);
	  state = CREATEHEADERS;
	  opcode = NAK;
          break;

        }      
    }
  return state;
}

/*****************************************************************************************
 * Name: void createPacket                                                               *
 * Description: Creates the packets to be sent to the server                             *
 * Inputs: Parameters: localData: This contains the data to be sent                      *
 *            Globals: opcode this is used to select the type of packet to create        *
 * Outputs: Globals: Packet: this stores information to be sent                          *
 *           Return: None                                                                *
 ****************************************************************************************/
void createPacket(char * localData)
{

<<<<<<< HEAD
  char track[TRACKLEN];
  bzero(packet, PACKETLEN); // Clears the packet

  sentt= 1; //This is used to stop any packets being sent before a packet has been sent.
  switch (opcode)
    {
    case PIN:
      sprintf(packet, "%c%s\n", opcode, localData); // pinpacket
      break;

    case PLAY:
    case TRACKINFO:
      sprintf(packet, "%c%s\n", opcode, localData); // request packet, used for play and track info
      break;

    case ACK:
    case NAK:
      sprintf(packet, "%c\n", opcode);
      break;

    default:
      printd("tried to create unknown packet\n");
      break;
    }
}
          
