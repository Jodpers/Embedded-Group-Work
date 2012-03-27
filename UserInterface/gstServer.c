/******************************************************************************************
 * gstServer.c                                                                            *
 * Description: This file will setup gstreamer on the server side and stream audio to the *
 *              client.
 * External globals: None                                                                 *
 * Author: James Sleeman                                                                  *
 * This is based on the Gstreamer hello world application which cam be found here:        *
 * http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/chapter-helloworld.html
 *****************************************************************************************/

#include <gst/gst.h>
#include <glib.h>

#include <linux/limits.h> // not sure which one i need for MAX_PATH
#include <limits.h>

char path[PATH_MAX];
int port;
char ip[15];

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
	gchar  *debug;
	GError *error;
	
	gst_message_parse_error (msg, &error, &debug);
	g_free (debug);
	
	g_printerr ("Error: %s\n", error->message);
	g_error_free (error);
	
	g_main_loop_quit (loop);
	break;
      }
      
    default:
      break;
    }
  
  return TRUE;
}

int main (int argc, char *argv[])
{
  GMainLoop *loop;
  
  GstElement *pipeline, *source, *sink;
  GstBus *bus;

  /* Initialisation */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("client"); 
  source   = gst_element_factory_make ("filesrc",       "file-source");
  sink     = gst_element_factory_make ("autoaudiosink", "client");

  if (!pipeline || !source || !sink) 
    {
      g_printerr ("One element could not be created. Exiting.\n");
      return -1;
    }

  /* Set up the pipeline */
  /* we set the input filename to the source element */
#ifdef STANDALONE
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);
#else
  g_object_set (G_OBJECT (source), "location", path, NULL);
#endif
  g_object_set (G_OBJECT (source), "host", ip, NULL); 
  g_object_set (G_OBJECT (source), "port", port, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* source | sink */
  gst_bin_add_many (GST_BIN (pipeline), source, sink, NULL);


  /* we link the elements together */
  /* source -> sink */
  gst_element_link (source, sink);

  /* Set the pipeline to "playing" state*/
#ifdef STANDALONE
  g_print ("Now playing: %s\n", argv[1]);
#else
  g_print ("Now playing: %s\n", path);
#endif

  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

