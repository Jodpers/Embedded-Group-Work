# Author: James Sleeman

OBJS = display.o external.o gst.o input.o keypad.o menu.o network.o pio_term.o volume.o states.o threads.o timer.o wifi_scan.o gstClient.o 
MOBJS = main.o
#CC = gcc
ARCH=arm
CC=/home/student/netlabhome/year3/igep/buildroot-2011.08/output/host/usr/bin/arm-unknown-linux-uclibcgnueabi-gcc

LFLAGS = -Wall -g -pthread -lasound `pkg-config --cflags --libs gstreamer-0.10` # lasound is required for alsa libs used in volume control
EXE = client

# libtool --mode=link gcc 
# libtool --mode=link gcc `pkg-config --cflags --libs gstreamer-0.10` -o gstServer 


all : $(OBJS) $(MOBJS)	
	$(CC) -o $(EXE) $(MOBJS) $(OBJS) $(LFLAGS)

clean:
	rm *.o

.c.o:
	$(CC) $(LFLAGS) -c $<
#gcc -fPIC -g -c `pkg-config --cflags --libs gstreamer-0.10`
