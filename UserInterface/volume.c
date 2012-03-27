/*
 * @file volume.c
 *
 *  Created on 16 Feb 2012
 *     @author Pete Hemery
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //threads
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#include "top.h"
#include "threads.h"
#include "menu.h"
#include "states.h"
#include "display.h"
#include "debug.h"

/**
 *  @brief Set the Volume for the master channel.
 *
 *    This function uses the Alsa API to set the volume
 *    for the master channel. In the case of the IGEP this is
 *    'DAC2 Digital Course', since it is configured for car audio.
 *
 *  @param [in] volume long value, between 0 and 99.
 *  @return Void.
 */
void SetAlsaVolume(long volume)
{
  long min, max;
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;
  const char *card = "default";
  //const char *selem_name = "Master";
  const char *selem_name = "DAC2 Digital Course";

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, selem_name);
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

  snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
  snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

  snd_mixer_close(handle);
}

/**
 *  @brief Get the Volume for the master channel.
 *
 *    This function uses the Alsa API to get the volume
 *    for the master channel.
 *
 *  @param [out] ptr pointer to long, output will be between 0 and 99.
 *  @return Void.
 */
void get_volume(long *ptr){
  long min, max;
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;
  const char *card = "default";
  //const char *selem_name = "Master";
  const char *selem_name = "DAC2 Digital Course";

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, selem_name);
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

  snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
  printd("Volume range <%lu,%lu>\n", min, max);
  snd_mixer_selem_get_playback_volume(elem,0,ptr);
  printd("volume val = %lu\n",*ptr);
  *ptr /= (max / 100);
  snd_mixer_close(handle);
}

/**
 *  @brief Volume control from the Menu.
 *
 *    This function uses the Alsa API to get and set
 *    the volume for the master channel.
 *    The use inputs a two digit number to set the volume,
 *    or presses the 'F' and 'B' buttons to adjust the
 *    value incrementally.
 *
 *    The volume control behaves differently from the menu,
 *    so menu flag is set to FALSE.
 *
 *  @param Void.
 *  @return Void.
 */
void volume(void){
  static long output = 50;
  int count = 0;

  char button_read = FALSE;  // Local snapshot of Global 'Button'
  int state_read = SUBMENU_SELECT;

  get_volume(&output);
  display_volume(output);
  set_menu(FALSE);

  while(alive && state_read == SUBMENU_SELECT){

    if(count == 0){
      display_volume(output);
    }

    pthread_mutex_lock(&button_Mutex);
    pthread_cond_wait(&button_Signal, &button_Mutex); // Wait for press
    button_read = button;               // Read the button pressed
    pthread_mutex_unlock(&button_Mutex);

    get_volume(&output);
    if(count == 0){
      display_volume(output);
    }

    pthread_mutex_lock(&state_Mutex);
    state_read = state;
    pthread_mutex_unlock(&state_Mutex);
    if(state_read == EMERGENCY || alive == FALSE){
      set_menu(FALSE); // in display.c
      break; // Get out if there's an emergency
    }

/* Button has been pressed. Now what? */
    switch(button_read){
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if(count == 0){
          reset_buffers();
        }
        if(++count <= 2){
          insert_char(button_read);
        }
        if(count == 2){
          output = (long)atoi(input_buffer);
          SetAlsaVolume(output);
          reset_buffers();
          count = 0;
        }
        break;

      case 'D':
        if(count){
          count--;
          delete_char();
        }
        break;

      case 'B': // Down
        if(--output >= 0){
          SetAlsaVolume(output);
          printd("output: %lu\n",output);
        }
        else{
          output = 0;
          printd("output: MIN\n");
        }
        break;
      case 'F': // Up
        if(++output < 100){
          SetAlsaVolume(output);
          printd("output: %lu\n",output);
        }
        else{
          output = 99;
          printd("output: MAX\n");
        }
        break;

     /* Accept, Cancel or Enter
      * The ACE case ;)
      */
      case 'A':
      case 'C':
      case 'E':
        pthread_mutex_lock(&state_Mutex);
        reset_buffers();
        state = MENU_SELECT; // Go back to menu
        state_read = state;
        pthread_mutex_unlock(&state_Mutex);
        set_menu(TRUE);
        break;
      default:
        break;
    }
  }
  return;
}
