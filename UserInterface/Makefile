# Author: James Sleeman

OBJS = display.o external.o gst.o input.o keypad.o menu.o network.o pio_term.o volume.o states.o threads.o wifi_scan.o
MOBJS = main.o
CC = gcc
LFLAGS = -Wall -g -pthread -lasound # lasound is required for alsa libs used in volume control
EXE = client

# libtool --mode=link gcc `pkg-config --cflags --libs gstreamer-0.10` -o gstClient gstClient.c
# libtool --mode=link gcc `pkg-config --cflags --libs gstreamer-0.10` -o gstServer gstServer.c


all : $(OBJS) $(MOBJS)	
	$(CC) -o $(EXE) $(MOBJS) $(OBJS) $(LFLAGS)

clean:
	rm *.o

.c.o:
	$(CC) $(LFLAGS) -c $<
