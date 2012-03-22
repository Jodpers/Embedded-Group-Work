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

#include "gstClient.g"

char ip[15];
int port;
GstElement * pipeline; // Moved here to allow other gst functions to use the variable.

int playing = FALSE;

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

  playing = TRUE;
}


#ifdef STANDALONE
int main (int argc, char *argv[])
#else
int gst(int port, char ip[])
#endif
  
{
  GMainLoop *loop;
  
  GstElement *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;
  
  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Check input arguments, used before intergration for testing */
#ifdef STANDALONE
  if (argc != 2) {
    g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    return -1;
  }
#endif

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("server");  /*Changed from "audio-player"*/
  
  /* Changed from "filesrc","file-source"*/
  source   = gst_element_factory_make ("tcpserversrc",  "file-source"); 
  
  demuxer  = gst_element_factory_make ("oggdemux",      "ogg-demuxer");
  decoder  = gst_element_factory_make ("vorbisdec",     "vorbis-decoder");
  conv     = gst_element_factory_make ("audioconvert",  "converter");
  
  /*Changed from "autoaudiosink", "audio-output"*/
  sink     = gst_element_factory_make ("autoaudiosink", "audio-output"); 

  if (!pipeline || !source || !demuxer || !decoder || !conv || !sink) 
    {
      g_printerr ("One element could not be created. Exiting.\n");
      return -1;
    }
  
  /* Set up the pipeline */

  /* we set the input filename to the source element */
#ifdef STANDALONE
  g_object_set (G_OBJECT (source), "host", argv[1], NULL); /*Changed from location*/
  g_object_set (G_OBJECT (source), "port", argv[2], NULL);
#else
  g_object_set (G_OBJECT (source), "host", ip, NULL); 
  g_object_set (G_OBJECT (source), "port", port, NULL);
#endif

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
  gst_bin_add_many (GST_BIN (pipeline), source, demuxer, decoder, conv, sink, NULL);
  
  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  gst_element_link (source, demuxer);
  gst_element_link_many (decoder, conv, sink, NULL);
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);
  
  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/


  /* Set the pipeline to "playing" state*/
#ifdef STANDALONE
  g_print ("Now playing: %s\n", argv[1]);
#endif
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state(pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref(GST_OBJECT (pipeline));

  return 0;
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

char * getTimeGst()
{

  char time[7] = {0};
  GstFormat format = GST_FORMAT_TIME; //Time in nanoseconds 
  gint64 curPos; //Stores the current position
  if(playing)
    {
      if(gst_element_query_position(pipeline, &format, &curPos))
	{
	  /* The maximum time supported is by this print statement is 9 hours 59 minutes 
	     and 59 seconds */
	  snprintf(msg, 7, "%u:%02u:%.2u\n", GST_TIME_ARGS (curPos)); 
	}
    }
                      
  return time;
}

  /* http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstElement.html#gst-element-seek-simple */
void seekGst()
{

}
