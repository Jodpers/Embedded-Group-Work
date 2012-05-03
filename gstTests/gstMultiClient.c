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

#include <gst/gst.h>
#include <glib.h>

#define STANDALONE 1

//#define OGG

#include "gstClient.h"

char ip[16] = "224.0.0.2";
int port = 4444;
GstElement *src, * pipeline; // Moved here to allow other gst functions to use the variable.
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


#ifdef STANDALONE
int main (int argc, char *argv[])
#else

#include <string.h>

void set_multicast_ip_and_port(char *ip_in, int port_in)
{
  strcpy(ip,ip_in);
  port = port_in;
  printf("ip: %s port:%d\n",ip,port);
}

void * gst_multicast(void)
#endif
{
 
#ifdef OGG
  GstElement *gstrtpjb, *rtp, *conv;
#else
  GstElement *rtpmpadepay, *mad;
#endif
  GstElement *filter, *sink;
  GstBus *bus;
  GstCaps *filtercaps;
  
  /* Initialisation */ 
  gst_init (NULL,NULL);

  loop = g_main_loop_new (NULL, FALSE);

  /* Check input arguments, used before intergration for testing */
#ifdef STANDALONE
  if (argc != 3) {
    g_printerr ("Usage: %s <IP> <PORT>\n", argv[0]);
    return -1;
  }
#endif

#ifdef OGG
  /* Create gstreamer elements */
  pipeline  = gst_pipeline_new ("multicast-client");
  src       = gst_element_factory_make ("udpsrc", "src");
  filter    = gst_element_factory_make ("capsfilter", "filter");
  gstrtpjb  = gst_element_factory_make ("gstrtpjitterbuffer", "gstrtpjb");
  rtp       = gst_element_factory_make ("rtpL16depay", "rtp");
  conv     = gst_element_factory_make ("audioconvert", "converter");
  sink     = gst_element_factory_make ("autoaudiosink", "sink");
  
  if (!pipeline || !src || !filter || !gstrtpjb || !rtp || !conv || !sink) 
  {
    g_printerr ("One element could not be created. Exiting.\n");
#ifdef STANDALONE
      return -1;
#else
      pthread_exit(-1);
#endif
  }
  
/*
  OGG

gst-launch udpsrc multicast-group=224.0.0.2 port=12000 ! "application/x-rtp,media=(string)audio, clock-rate=(int)44100, width=16,height=16, encoding-name=(string)L16, encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, payload=(int)96" ! gstrtpjitterbuffer do-lost=true ! rtpL16depay ! audioconvert ! alsasink sync=false
*/
   
  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), src, filter, gstrtpjb, rtp, conv, sink, NULL);

  /* we link the elements together */
  gst_element_link_many (src, filter, gstrtpjb, rtp, conv, sink, NULL);

  /* we add capabilities to the pipeline */
  filtercaps = gst_caps_new_simple ("application/x-rtp",
     "media", G_TYPE_STRING, ("audio"),
     "clock-rate", G_TYPE_INT, 44100,
     "width", G_TYPE_INT, 16,
     "height", G_TYPE_INT, 16,
     "encoding-name", G_TYPE_STRING, ("L16"),
     "encoding-params", G_TYPE_STRING, ("1"),
     "channels", G_TYPE_INT, 1,
     "channel-positions", G_TYPE_INT, 1,
     "payload", G_TYPE_INT, 96,
     NULL);
  
  
  /* Set up the pipeline */
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);
  
  g_object_set (G_OBJECT (gstrtpjb), "do-lost", TRUE, NULL);
#else
  /* Create gstreamer elements */
  pipeline  = gst_pipeline_new ("multicast-client");
  src       = gst_element_factory_make ("udpsrc", "src");
  filter    = gst_element_factory_make ("capsfilter", "filter");
  rtpmpadepay  = gst_element_factory_make ("rtpmpadepay", "rtpmpadepay");
  mad       = gst_element_factory_make ("mad", "mad");
  sink     = gst_element_factory_make ("autoaudiosink", "sink");
  
  if (!pipeline || !src || !filter || !rtpmpadepay || !mad || !sink) 
  {
    g_printerr ("One element could not be created. Exiting.\n");
#ifdef STANDALONE
      return -1;
#else
      pthread_exit(-1);
#endif
  }
  
/*
  MP3
   
gst-launch udpsrc multicast-group=224.0.0.2 port=4444 caps="application/x-rtp, media=(string)audio, clock-rate=(int)90000, encoding-name=(string)MPA, payload=(int)96" ! rtpmpadepay ! mad ! autoaudiosink
*/
   
  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), src, filter, rtpmpadepay, mad, sink, NULL);

  /* we link the elements together */
  gst_element_link_many (src, filter, rtpmpadepay, mad, sink, NULL);

  /* we add capabilities to the pipeline */
  filtercaps = gst_caps_new_simple ("application/x-rtp",
     "media", G_TYPE_STRING, ("audio"),
     "clock-rate", G_TYPE_INT, 90000,
     "encoding-name", G_TYPE_STRING, "MPA",
     "payload", G_TYPE_INT, 96,
     NULL);
  
  
  /* Set up the pipeline */
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);
    
#endif

  /* we set the input filename to the source element */
#ifdef STANDALONE
  g_object_set (G_OBJECT (src), "multicast-group", argv[1], NULL);
  g_object_set (G_OBJECT (src), "port", atoi(argv[2]), NULL);
#else
  g_object_set (G_OBJECT (src), "multicast-group", ip, NULL); 
  g_object_set (G_OBJECT (src), "port", port, NULL);
#endif

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* Set the pipeline to "playing" state*/
#ifdef STANDALONE
  g_print ("Now playing: %s\n", argv[1]);
#endif

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* wait until it's up and running or failed */
  if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) {
    g_error ("Failed to go into PLAYING state");
#ifdef STANDALONE
    return -1;
#else
    pthread_exit(-1);
#endif
  }
  
  gst_playing = TRUE;
  
  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);
  
  gst_playing = FALSE;

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

  /* http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstElement.html#gst-element-seek-simple */
void seekGst()
{

}
