#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>


void SetAlsaMasterVolume(long volume)
{
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

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

void main(void){
  char input[10] = {0};
  char in = 0;
  int count = 0;
  long output = 50;
  
  while(1){
    in = getchar();
    if(in == 'u'){
      if(++output < 102){
        SetAlsaMasterVolume(output);
        printf("output: %d\n",output);
      }
      else{
        output = 102;
        printf("output: MAX\n",output);
      }
    }
    else if(in == 'd'){
      if(--output >= 0){
        SetAlsaMasterVolume(output);
        printf("output: %d\n",output);
      }
      else{
        output = 0;
        printf("output: %d\n",output);
      }
    }
  }
  return;
}
