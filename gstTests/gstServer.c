/******************************************************************************************
* gstServer.c *
* Description: This file will setup gstreamer on the server side and stream audio to the *
* client.
* External globals: None *
* Author: James Sleeman *
* This is based on the Gstreamer hello world application which cam be found here: *
* http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/chapter-helloworld.html
*****************************************************************************************/

#include <string.h>

#include <gst/gst.h>
#include <glib.h>

#include <linux/limits.h> // not sure which one i need for MAX_PATH
#include <limits.h>

//#define STANDALONE 1

enum {STOPPED, PLAYING, EOS, ERROR};

int port = 4444;
char ip[16] = {"localhost"};

int gst_playing = STOPPED;
GError *error;

GMainLoop *loop;
GstElement *src, *pipeline;
  
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg))
    {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
      
    case GST_MESSAGE_ERROR:
      {
        gchar *debug;
        GError *error;

        gst_message_parse_error (msg, &error, &debug);
        g_free (debug);

        g_printerr ("Error: %s\n", error->message);
        g_error_free (error);

        printf("error code: %d\n",error->code);
        g_main_loop_quit (loop);
        break;
      }
      
    default:
      break;
    }
  
  return TRUE;
}


#ifdef STANDALONE
int main (int argc, char *argv[])
{
#else
 int gstServer(int port_in, char * ip_in, char * path_in)
{

  char path[500];
  port = port_in;
  strcpy(ip,ip_in);
  strcpy(path,path_in);
#endif

  GstElement *sink;
  GstBus *bus;

  /* Initialisation */
  gst_init (NULL, NULL);
  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("server");
  src = gst_element_factory_make ("filesrc", "src");
  sink = gst_element_factory_make ("tcpclientsink", "client");
  
  if (!pipeline || !src || !sink)
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
  g_object_set (G_OBJECT (src), "location", argv[1], NULL);
#else
  g_object_set (G_OBJECT (src), "location", path, NULL);
#endif
  g_object_set (G_OBJECT (sink), "host", ip, NULL);
  g_object_set (G_OBJECT (sink), "port", port, NULL);


  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* source | sink */
  gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);

  /* we link the elements together */
  /* source -> sink */
  gst_element_link_many (src, sink, NULL);
  

  /* Set the pipeline to "playing" state*/
#ifdef STANDALONE
  g_print ("Now playing: %s\n", argv[1]);
#else
  g_print ("Now playing: %s\n", path);
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
  
  gst_playing = PLAYING;

  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);
  
  gst_playing = STOPPED;

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  
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
void stopGst()
{
  gst_element_set_state(pipeline, GST_STATE_NULL);
}
void setPathGst(char * path)
{
 g_object_set (G_OBJECT (src), "location", path, NULL);
}
