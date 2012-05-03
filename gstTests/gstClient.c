/******************************************************************************************
 * gstClient.c                                                                            *
 * Description: This is file is used to setup gstreamer to receive an audio stream and    *
 *              playback the stream and offer the controls to change the state of the     *
 *              pipleine used to stream the audio.                                        *
 * External globals: None                                                                 *
 * Author: James Sleeman                                                                  *
 * This is based on the Gstreamer hello world application which cam be found here:        *
 * http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/chapter-helloworld.html
 *****************************************************************************************/

#include <string.h>
#include <gst/gst.h>
#include <glib.h>

#define STANDALONE 1

//#define OGG

#include "gstClient.h"

enum {STOPPED, PLAYING, EOS, ERROR};

char ip[16] = {'\0'};
int port = 4444;

GstElement * pipeline; // Moved here to allow other gst functions to use the variable.
GMainLoop *loop;

int gst_playing = FALSE;

char trackTime[12] = {0};

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) 
    {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit(loop);
      break;
      
    case GST_MESSAGE_ERROR: 
      {
	      gchar  *debug;
	      GError *error;

	      gst_message_parse_error(msg, &error, &debug);
	      g_free(debug);

	      g_printerr("Error: %s\n", error->message);
	      g_error_free(error);

	      g_main_loop_quit(loop);
	      break;
      }

    default:
      break;
    }
  
  return TRUE;
}

static void on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;
  
  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");
  
  sinkpad = gst_element_get_static_pad (decoder, "sink");
  
  gst_pad_link (pad, sinkpad);  
  gst_object_unref (sinkpad);
}

#ifdef STANDALONE
int main (int argc, char *argv[])
#else


void set_ip_and_port(char *ip_in, int port_in)
{
  strcpy(ip,ip_in);
  port = port_in;
}

void * gst(void)
#endif
{

  GstElement *src, *sink;
#ifdef OGG
  GstElement *demuxer, *decoder, *conv;
#else
  GstElement *rtpmpadepay, *filter, *mad;
#endif
  GstBus *bus;
  GstCaps *filtercaps;
  
  printf("gst thread alive!\n");
  
  /* Initialisation */ 
  gst_init (NULL,NULL);

  loop = g_main_loop_new (NULL, FALSE);

  /* Check input arguments, used before intergration for testing */
#ifdef STANDALONE
  if (argc != 2) {
    g_printerr ("Usage: %s <IP> <PORT>\n", argv[0]);
    return -1;
  }
#endif


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("client");

#ifdef OGG
  src      = gst_element_factory_make ("tcpserversrc",  "src"); 
  demuxer  = gst_element_factory_make ("oggdemux",      "ogg-demuxer");
  decoder  = gst_element_factory_make ("vorbisdec",     "vorbis-decoder");
  conv     = gst_element_factory_make ("audioconvert",  "converter");
    
#else
/*  MP3 */
  src         = gst_element_factory_make ("udpsrc",  "src");
  filter      = gst_element_factory_make ("capsfilter", "filter");
  rtpmpadepay = gst_element_factory_make ("rtpmpadepay", "rtpmpadepay");
  mad         = gst_element_factory_make ("mad", "mad");
#endif

  sink        = gst_element_factory_make ("autoaudiosink", "sink");
  
#ifdef OGG
  if (!pipeline || !src || !demuxer || !decoder || !conv || !sink)
#else
  if (!pipeline || !src || !filter || !rtpmpadepay || !mad || !sink)
#endif
    {
      g_printerr ("One element could not be created. Exiting.\n");
#ifdef STANDALONE
      return -1;
#else
      pthread_exit(-1);
#endif
    }
    
  /* Set up the pipeline */

  /* we set the input filename to the source element */
#ifdef STANDALONE
//  g_object_set (G_OBJECT (src), "host", argv[1], NULL); /*Changed from location*/
  g_object_set (G_OBJECT (src), "port", atoi(argv[1]), NULL);
#else
  g_object_set (G_OBJECT (src), "host", ip, NULL); 
  g_object_set (G_OBJECT (src), "port", port, NULL);
#endif

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

#ifdef OGG
  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), src, demuxer, decoder, conv, sink, NULL);
  
  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  gst_element_link (src, demuxer);
  gst_element_link_many (decoder, conv, sink, NULL);
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);
  
  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/
#else
  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), src, filter, rtpmpadepay, mad, sink, NULL);

  /* we link the elements together */
  gst_element_link_many (src, filter, rtpmpadepay, mad, sink, NULL);
  /*
  gst_element_link_many (src, filter, rtpmpadepay, NULL);
  gst_element_link_many (mad, sink, NULL);
  g_signal_connect (rtpmpadepay, "pad-added", G_CALLBACK (on_pad_added), mad);
*/

    /* we add capabilities to the pipeline */
  filtercaps = gst_caps_new_simple ("application/x-rtp",
     "media", G_TYPE_STRING, ("audio"),
     "clock-rate", G_TYPE_INT, 90000,
     "encoding-name", G_TYPE_STRING, "MPA",
     "payload", G_TYPE_INT, 96,
     NULL);
     
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);
       
#endif

  /* Set the pipeline to "playing" state*/
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  /* wait until it's up and running or failed */
  if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) 
  {
    g_error ("Failed to go into PLAYING state");
#ifdef STANDALONE
    return -1;
#else
    pthread_exit(-1);
#endif
  }
  
  gst_playing = PLAYING;

  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);
  
  gst_playing = STOPPED;
  
  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state(pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref(GST_OBJECT (pipeline));
  
#ifdef STANDALONE
  return 0;
#else
  pthread_exit(0);
#endif
}

void killGst()
{
  g_main_loop_quit(loop);
}

void playGst()
{
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void pauseGst()
{
  gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

int getTimeGst(char * trackTime)
{

  GstFormat format = GST_FORMAT_TIME; //Time in nanoseconds 
  gint64 curPos; //Stores the current position
  if(gst_playing)
    {
      if(gst_element_query_position(pipeline, &format, &curPos))
      {
      
        /* The maximum time supported is by this print statement is 9 hours 59 minutes
           and 59 seconds */
        snprintf(trackTime, 9, "%u:%02u:%.2u\n", GST_TIME_ARGS (curPos));
        printf("trackTime %s\n",trackTime);
      }
      return curPos/(1000UL*1000UL*1000UL);
    }
                      
  return 0;
}

  /* http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstElement.html#gst-element-seek-simple */
void seekGst()
{

}
