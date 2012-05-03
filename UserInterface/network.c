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
#include "gstClient.h"

extern char closest_mac[MAC_LEN];

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
int follower = -1;
char playCode = '0';
char reqCode = '0';
//char * msg;
char clMac[MAC_LEN] = {'\0'};

extern int logged_in; // (states.c)
extern void set_emergency(int state);

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

  extern int mac_changed;

  while (alive)
    {      
      printf("netFSM state %d\n",state);
      switch (state)
	{
	  //This thread will sleep after entering this state
	case WAITING:
	  printf("going to sleep\n");	  
	  pthread_mutex_lock(&network_Mutex);
	  pthread_cond_wait(&network_Signal, &network_Mutex);
	  opcode = task;
	  if (mac_changed == 1 && logged_in)
    {
      mac_changed = 0;
      
      reqCode = CLOSEST_MAC_ADDRESS;
      state = CREATEHEADERS;
      opcode = PLAY;
	    strncpy(clMac, closest_mac, MAC_LEN);
      pthread_mutex_unlock(&network_Mutex);
	    break;
    }
	  else
    {  
  	  bzero(localRecPacket,MAXDATASIZE);
      strncpy(localRecPacket, receivedPacket, MAC_LEN);
      reqCode = playCode;
	  }
	  pthread_mutex_unlock(&network_Mutex);
	  
	  if (alive == FALSE)
	    break;              /* If the signal to die comes, then actually die! */
	  
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
	      
	      break;
	    }
	  pthread_mutex_lock(&request_Mutex);
	  strncpy(localData, data, PACKETLEN);
	  opcode = task;
	  pthread_mutex_unlock(&request_Mutex);
	  
	case CREATEHEADERS:
	  state = createPacket(localData);
	  printd("Packet to send:%s", packet);
	  
	  break;
	case SEND:
	  
	  printd("packet at sending time:%s\n", packet);
	  len = strlen(packet);
	  printd("len: %d\npacket: %s\n", len, packet);
	  if (len > 0)
	    {
	      send(sockfd, packet, len, 0);
	      printd("sent\n");
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
  pthread_exit(0);
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
      bzero(buffer,MAXDATASIZE);
      if ((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) // Blocking if statement
	    {
	      perror("recv");
	      alive = FALSE;
	      break;
	    }

      printd("Packet received: \"%s\"\n", buffer);
      printd("numbytes %d\n",numbytes);
      if (strlen(buffer) != 0)      //encase we get a null packet
      {
        buffer[numbytes] = '\0';
        bzero(receivedPacket, MAXDATASIZE);
        
        pthread_mutex_lock(&network_Mutex);
        strncpy(receivedPacket, buffer, numbytes);
        task = RECEIVE;
        pthread_cond_signal(&network_Signal);
        pthread_mutex_unlock(&network_Mutex);
      }
    }
  pthread_exit(0);
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

  char * tmp;
  int portGst = 0;
  char * ipGst;
  int i,len;
  int j = 0;

  printd("buffer:%s\n",buffer);

  /*Checks that buffer isn't empty*/
  if(strlen(buffer))
    {
    switch (buffer[0])
      {
      case EMERGENCY:
        printd("Emergency, stop, leave the building! \n\n");
	  /*
	      len = strlen(buffer);
	      tmp = (char*) malloc (len-1);
	      
	      for(i = 0; i < len-1; i++)
	        {
	          tmp[i] = buffer[i+1]; 
	        }
*/
        
	      data[0] = 1;
	      set_emergency(TRUE);  //states.c
	      
	      state = CREATEHEADERS;
	      opcode = ACK;
	      //free(tmp);
        break;

  //10  unauthenticated
  //110 indiv
  //111 leader
  //112 follower

	    case PIN: //format: 111,port.ip
	      printd("buffer[1]:%c\n",buffer[1]);

        if (buffer[1] == PASS)
          {
          loggedIn = PASS;      
          printd("PIN Authenticated\n");
    
          tmp = (char*) malloc (16);
          bzero(tmp,16);
	        tmp[0] = buffer[2];

	        follower = atoi(tmp); //Follower: 0 = Indiv, 1= Follower ,2=Leader
	        
	        /*Copy out Port*/
	        j = 0;
	        while(buffer[j+3] != ',')
		      {
		        tmp[j] = buffer[j+3];
		        j++;
		      }
		      j++;

	        portGst = atoi(tmp);
	        free(tmp);
	        ipGst = (char*) malloc(IPLEN * sizeof (char));
	        bzero(ipGst,IPLEN);
	        i=0;
	        while(buffer[j+3] != ',' && buffer[j+3] != '\0')
		      {
		        ipGst[i++] = buffer[j+3]; 
		        j++;
		      }
		      printf("buffer: %s\n",buffer);
		      printf("ipGst: %s\nportGst %d\n",ipGst,portGst);
	        
	        set_ip_and_port(ipGst,portGst);
	        free(ipGst);
	         
          }
          else
          {
            loggedIn = FAIL;
            printd("PIN failed\n");
            follower = -1;
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
	        printf("buffer[1] = %d\n",buffer[1]);
	        if (buffer[1] == FAIL)
          {
            state = CREATEHEADERS; 
            opcode = ACK;
            
            pthread_mutex_lock(&request_Mutex);
            data[0] = FAIL;
            pthread_cond_signal(&request_Signal);
            pthread_mutex_unlock(&request_Mutex);
          }
	        if (buffer[1] == END_OF_PLAYLIST)
          {
            printd("End of playlist reached");
            pthread_mutex_lock(&request_Mutex);
            data[0] = END_OF_PLAYLIST;
            pthread_cond_signal(&request_Signal);
            pthread_mutex_unlock(&request_Mutex);
          }
	        if (buffer[1] == '1')
          {
            printd("Track Found\n");
            pthread_mutex_lock(&request_Mutex);
            data[0] = '1';
            pthread_cond_signal(&request_Signal);
            pthread_mutex_unlock(&request_Mutex);
          }
          
	        break;

        case TRACKINFO:
          printd("trackinfo:%s\n", buffer);	  
      	  state = CREATEHEADERS;
	        opcode = ACK;
          break;

       
         case ACK: /* Do nothing */
          printd("ACK: %s\n", buffer);

	        state = WAITING;
          break;
        
        case NAK: /* Resends last packet */
            printd("NAK: %s\n", buffer);

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

        case MULTICAST: 
          printd("multicast:%s\n", buffer);

          state = CREATEHEADERS;
          opcode = ACK;
          break;

	      default:
          printd("Unknown packet:%s\n", buffer);
          state = CREATEHEADERS;
          opcode = NAK;
          break;

        }      
    }
  printd("Data: %s\n",data);
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
int createPacket(char * localData)
{

  //21 - play track only - client sent
  //22 - play playlist - client sent
  //23 - finished indiv track, will send ack - client sent
  //24 - finished track in playlist - client sent
  //25 - End of playlist - server sent
  //26 - Send new mac address

  bzero(packet, PACKETLEN); // Clears the packet

  sentt= 1; //This is used to stop any packets being sent before a packet has been sent.
  
  printf("opcode = %c\n",opcode);
  switch (opcode)
    {
    case PIN:
      sprintf(packet, "%c%s\n", opcode, localData); // pinpacket
      break;

    case PLAY:
      if (follower == TRUE)
	    {
          printf("follower\n");
	      return  WAITING;
	    }
          printf("in PLAY state\n");
      
	    
      sprintf(packet, "%c%c%s\n", opcode, reqCode, localData); // request packet, 
      
      if (reqCode == CLOSEST_MAC_ADDRESS)
	    {
	      printf("Sending Closest Mac\n");
	      sprintf(packet, "%c%c%s\n", opcode, reqCode, clMac);
	    }
 
      break;

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
  return SEND;
}

int getFollower()
{
  return follower;
}

