/*
 * @file wifi_scan.c
 *
 *  Created on 6 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

#include "top.h"
#include "debug.h"
#define COMMAND_LEN 150
#define DATA_SIZE 512

#define STRING_LENGTH  18

#define TIMEOUT 10

extern int alive;

char closest_mac[STRING_LENGTH] = {'\0'};

/**
 *  @brief Wifi Scan Routine.
 *
 *    This function is called every 10 seconds.
 *    It checks uses 'iwlist wlan0 scan' and updates a global
 *    variable which holds the closest wifi beacon's MAC address.
 *
 *  @param Void.
 *  @return Void.
 */
//void * main(void)
void * wifi_scan(void)
{

/* Wifi Signals */
  extern pthread_mutex_t wifi_Mutex;
  extern pthread_cond_t wifi_Signal;

  struct timespec timeToWait;
  struct timeval now;
  int err;

  FILE *pipein_fp;

  int i;

  char command[COMMAND_LEN];
  char data[DATA_SIZE];

  int highest_quality = 0;
  int new_quality = 0;
  char new_mac[STRING_LENGTH];


  gettimeofday(&now,NULL);
  
  while(alive)
  {
    highest_quality = 0;
    new_quality = 0;
    bzero(new_mac, STRING_LENGTH);
    
    /* Execute a process listing */
//    sprintf(command, "sudo iwlist wlan0 scan | awk -W interactive ' $4 ~ /Address/ { print $5} $1 ~ /Quality/ { print $1 } ' | cut -d= -f2 | cut -d/ -f1");
    sprintf(command, "awk -W interactive ' $4 ~ /Address/ { print $5} $1 ~ /Quality/ { print $1 } ' scan.txt | cut -d= -f2 | cut -d/ -f1");
  
    /* Setup our pipe for reading and execute our command. */
    if((pipein_fp = popen(command,"r")) == NULL){
        fprintf(stderr, "Could not open pipe for output.\n");
        perror("popen");
        exit(1);
    }
  
    /* Processing loop */
    while(fgets(data, DATA_SIZE, pipein_fp))
    {
      strncpy(new_mac,data,17);
      fgets(data, DATA_SIZE, pipein_fp);
      new_quality = atoi(data);
      printd("%s %d\n",new_mac,new_quality);
      if (new_quality > highest_quality)
      {
        highest_quality = new_quality;
        strcpy(closest_mac,new_mac);
      }
    }
    
    /* Close iwlist pipe, checking for errors */
    if (pclose (pipein_fp) != 0)
    {
      fprintf (stderr, "Could not close 'iwlist', or other error.\n");
    }
    if (highest_quality != 0)
    {
      printf("closest mac:\t\t %s\n",closest_mac);
    }
    //sleep(10);
    
    gettimeofday(&now,NULL);
    timeToWait.tv_sec = now.tv_sec + TIMEOUT;
    timeToWait.tv_nsec = 0;
    pthread_mutex_lock(&wifi_Mutex);
    err = pthread_cond_timedwait(&wifi_Signal, &wifi_Mutex, &timeToWait);
    if (err == ETIMEDOUT) {
      printf("timed out\n");
    }
    else
    {
      printf("called\n");
    }
    pthread_mutex_unlock(&wifi_Mutex);
    
  }
 
  return;
}


