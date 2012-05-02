#include <stdio.h>
#include <pthread.h>

#define FALSE 0
#define TRUE 1

//#define MULTI

enum {STOPPED, PLAYING, EOS, ERROR};


#ifndef MULTI
extern int gstServer(int port, char * ip, char * path);
#else
extern int gstMultiCastServer(int port, char * ip, char * path);
#endif

extern int gst_playing;

void main(void)
{
  int ret = 0;
  int count = 1000;
  
#ifndef MULTI
  while(--count)
  {
    ret = gstServer(4444, "127.0.0.1", "/media/Data/Ab/My Music/FlashGot/Kid Koala - Third World Lover.mp3");
    printf("gstServer returned: %d\n",ret);
/*
    if (gst_playing == STOPPED)
    {
      ret = gstServer(4444, "127.0.0.1", "/media/Data/Ab/Work/ESD/example.ogg");
      printf("gstServer returned: %d\n",ret);
    }
    */
  }
#else
  while(1)
  {
    ret = gstMultiCastServer(4444, "224.0.0.2", "/media/Data/Ab/Work/ESD/bingxiang.ogg");
    printf("gstServer returned: %d\n",ret);
  }
#endif  
  
  return ;
}
