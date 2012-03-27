/*
 * @file wifi_scan.c
 *
 *  Created on 6 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#define COMMAND_LEN 150
#define DATA_SIZE 512

#define MAC_LENGTH  18


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
void wifi_scan(void)
{
  FILE *pipein_fp;

  int i;

  char command[COMMAND_LEN];
  char data[DATA_SIZE];
  char * tokens;

  char info[10][DATA_SIZE];
  int info_cnt = 0;
  int token_cp_flag = 0;
  int signal_token = 0;
  /* TODO
  int highest_signal = 0;
  int highest_index = 0;
  char *highest_address[MAC_LENGTH] = {0};*/

  /* Execute a process listing */
  sprintf(command, "sudo iwlist wlan0 scan | grep -e Address -e Signal -e ESSID");
  
  /* Setup our pipe for reading and execute our command. */
  if((pipein_fp = popen(command,"r")) == NULL){
      fprintf(stderr, "Could not open pipe for output.\n");
      perror("popen");
      exit(1);
  }

  /* Processing loop */
  while(fgets(data, DATA_SIZE, pipein_fp))
  {
    printd("%s",data);
	tokens = strtok (data," ,.=\"\n");
    while (tokens != NULL){
      if (token_cp_flag == 1){
  		  strcpy(info[info_cnt++],tokens);
	      token_cp_flag = 0;
	    }
      if (signal_token == 2){
  		  strcpy(info[info_cnt++],tokens);
	      signal_token = 0;
	    }

      if ((strcmp("Address:",tokens) == 0) ||
          (strcmp("ESSID:",tokens) == 0) ||
          (strcmp("Quality",tokens) == 0)){
	      token_cp_flag = 1;
	    }

      /* Output will be two lines, 'Signal' then 'level'. So skip 'level' */
      if (signal_token){
	      signal_token++;
	    }
      if (strcmp("Signal",tokens) == 0){
	      signal_token++;
	    }

      printd ("%s\n",tokens);
      tokens = strtok (NULL, " ,.=\"\n");
    }
  }
  /* Print the token strings */
  for (i = 0; i < info_cnt; i++)
  {
    printd("%s\n",info[i]);
  }
  
 /* Close iwlist pipe, checking for errors */
  if (pclose (pipein_fp) != 0)
  {
    fprintf (stderr, "Could not close 'iwlist', or other error.\n");
  }
 
  return;
}


