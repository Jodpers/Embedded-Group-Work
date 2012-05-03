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
#include <pthread.h>

#include <gst/gst.h>
#include <glib.h>

#include "gstClient.h"
#include "debug.h"


extern int getFollower();


GstElement *pipeline; // Moved here to allow other gst functions to use the variable.
GMainLoop *loop;


char ip[16] = {'\0'};
int port = 0;
int ip_set = FALSE;

int gst_playing = FALSE;

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

void set_ip_and_port(char *ip_in, int port_in)
{
  strcpy(ip,ip_in);
  port = port_in;
  printd("ip: %s port:%d\n",ip,port);
  ip_set = TRUE;
}

void * gst(void)
{
  GstElement *src, *rtpmpadepay, *filter, *mad, *sink;
  GstBus *bus;
  GstCaps *filtercaps;
  
  printf("gst thread alive!\n");
  
  while(ip_set == FALSE); //Wait for ip and port initialisation
  ip_set = FALSE;

  /* Initialisation */ 
  gst_init (NULL,NULL);

  loop = g_main_loop_new (NULL, FALSE);


  /* Create gstreamer elements */
  pipeline    = gst_pipeline_new ("client");
  src         = gst_element_factory_make ("udpsrc",  "src");
  filter      = gst_element_factory_make ("capsfilter", "filter");
  rtpmpadepay = gst_element_factory_make ("rtpmpadepay", "rtpmpadepay");
  mad         = gst_element_factory_make ("mad", "mad");
  sink        = gst_element_factory_make ("alsasink", "sink");

  if (!pipeline || !src || !filter || !rtpmpadepay || !mad || !sink)
  {
    g_printerr ("One element could not be created. Exiting.\n");
    pthread_exit((void *)-1);
  }

  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), src, filter, rtpmpadepay, mad, sink, NULL);

  /* we link the elements together */
  gst_element_link_many (src, filter, rtpmpadepay, mad, sink, NULL);

/*
Unicast
 gst-launch -v udpsrc port=12000 ! "application/x-rtp, media=(string)audio, clock-rate=(int)90000, endcoding-name=(string)MPA, payload=(int)96" ! rtpmpadepay ! mad ! alsasink sync=false */
/*
Multicast
gst-launch -v udpsrc multicast-group=224.0.0.2 port=12000 ! "application/x-rtp, media=(string)audio, clock-rate=(int)90000, endcoding-name=(string)MPA, payload=(int)96" ! rtpmpadepay ! mad ! alsasink sync=false
*/

  /* we add capabilities to the pipeline */
  filtercaps = gst_caps_new_simple ("application/x-rtp",
     "media", G_TYPE_STRING, ("audio"),
     "clock-rate", G_TYPE_INT, 90000,
     "encoding-name", G_TYPE_STRING, ("MPA"),
     "payload", G_TYPE_INT, 96,
     NULL);
  
  /* Set up the pipeline */
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);
  
  /* setup the source element */
  if (getFollower() > 0)  // If the follower number is 1 or 2 then its multicast
  {
    g_object_set (G_OBJECT (src), "multicast-group", ip, NULL);
  }
  g_object_set (G_OBJECT (src), "port", port, NULL);
  g_object_set (G_OBJECT (sink), "sync", FALSE, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


  /* Set the pipeline to "playing" state*/
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  /* wait until it's up and running or failed */
  if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE)
  {
    g_error ("Failed to go into PLAYING state");
    pthread_exit((void *)-1);
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

  pthread_exit((void *)0);
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

long long int getTimeGst()
{
  char trackTime[12] = {0};

  GstFormat format = GST_FORMAT_TIME; //Time in nanoseconds 
  gint64 curPos; //Stores the current position
  if(gst_playing)
    {
      if(gst_element_query_position(pipeline, &format, &curPos))
      {

        /* The maximum time supported is by this print statement is 9 hours 59 minutes
           and 59 seconds */
        snprintf(trackTime, 11, "%u:%02u:%.2u.%2.2u\n", GST_TIME_ARGS (curPos));
        printf("trackTime %s\n",trackTime);
      }
      return curPos;
    }
                      
  return 0;
}

  /* http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstElement.html#gst-element-seek-simple */
void seekGst()
{

}
