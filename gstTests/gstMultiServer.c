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

int port = 12000;
char ip[16] = {"224.0.0.2"};

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

        g_main_loop_quit (loop);
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

/* http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/chapter-dataaccess.html */
static gboolean
cb_have_data (GstPad    *pad,
	      GstBuffer *buffer,
	      gpointer   u_data)
{
  gint x, y;
  guint16 *data = (guint16 *) GST_BUFFER_DATA (buffer), t;

  /* invert data */
/*
  for (y = 0; y < 288; y++) {
    for (x = 0; x < 384 / 2; x++) {
      t = data[384 - 1 - x];
      data[384 - 1 - x] = data[x];
      data[x] = t;
    }
    data += 384;
  }
  */
  printf("callback!\n");

  return TRUE;
}

#ifdef STANDALONE
int main (int argc, char *argv[])
{
#else
 int gstMultiCastServer(int port_in, char * ip_in, char * path_in)
{

  char path[100];
  port = port_in;
  strcpy(ip,ip_in);
  strcpy(path,path_in);
#endif
  GstElement *demuxer, *decoder, *conv, *filter, *rtp, *sink;
  GstCaps *filtercaps;
  GstPad *pad;
  GstBus *bus;

  /* Initialisation */
  gst_init (NULL, NULL);
  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("multicast-server");
  src      = gst_element_factory_make ("filesrc", "src");
  demuxer  = gst_element_factory_make ("oggdemux", "demuxer");
  decoder  = gst_element_factory_make ("vorbisdec", "decoder");
  conv     = gst_element_factory_make ("audioconvert", "converter");
  filter   = gst_element_factory_make ("capsfilter", "filter");
  rtp      = gst_element_factory_make ("rtpL16pay", "rtp");
  sink     = gst_element_factory_make ("udpsink", "sink");

  if (!pipeline || !src || !demuxer || !decoder || !conv || !filter || !rtp || !sink)
  {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /*
  gst-launch filesrc location=/media/Data/Ab/Work/ESD/example.ogg ! oggdemux ! vorbisdec ! audioconvert ! audio/x-raw-int,channels=1,depth=16,width=16,rate=44100 ! rtpL16pay ! udpsink host=224.0.0.2 port=12000
  */
  
  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), src, demuxer, decoder, conv, filter, rtp, sink, NULL);

  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  gst_element_link (src, demuxer);
  gst_element_link_many (decoder, conv, filter, rtp, sink, NULL);
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);

  /* we add capabilities to the pipeline */
  filtercaps = gst_caps_new_simple ("audio/x-raw-int",
     "channels", G_TYPE_INT, 1,
     "width", G_TYPE_INT, 16,
     "depth", G_TYPE_INT, 16,
     "rate", G_TYPE_INT, 44100,
     NULL);  
  
  /* Set up the pipeline */
  
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);
    
  /* we set the input filename to the source element */
#ifdef STANDALONE
  g_object_set (G_OBJECT (src), "location", argv[1], NULL);
#else
  g_object_set (G_OBJECT (src), "location", path, NULL);
#endif
  
  g_object_set (G_OBJECT (sink), "host", ip, NULL);
  g_object_set (G_OBJECT (sink), "port", port, NULL);


/*
  pad = gst_element_get_pad (src, "src");
  gst_pad_add_buffer_probe (pad, G_CALLBACK (cb_have_data), NULL);
  gst_object_unref (pad);
*/
  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


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
  }

  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipline\n");
  gst_object_unref (GST_OBJECT (pipeline));

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
/*void stopGst()
{
  gst_element_set_state(pipeline, GST_STATE_STOPPED);
}*/
void setPathGst(char * path)
{
 g_object_set (G_OBJECT (src), "location", path, NULL);
}
