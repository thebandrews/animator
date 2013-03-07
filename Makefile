#  WARNING: As of Fall 2010, this makefile doesn't work...
#  Makefile for fltk applications
#

LOCAL = /usr
INCLUDE = -I$(LOCAL)/include -I/usr/X11R6/include
LIBDIR = -L$(LOCAL)/lib -L/usr/X11R6/lib

LIBS = -lfltk -lfltk_gl -lXext -lX11 -lm -lGL -lGLU -lfltk_images

CFLAGS = -g -FLTK1

CC = g++

.SUFFIXES: .o .cpp .cxx

.cpp.o: 
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $*.o $<

.cxx.o: 
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $*.o $<

ALL.O = imageio.o Fl_Native_File_Chooser.o camera.o modelerapp.o modelerdraw.o modelerui.o modelerview.o\
	sample.o

modeler: $(ALL.O)
	$(CC) $(CFLAGS) -o $@ $(ALL.O) $(INCLUDE) $(LIBDIR) $(LIBS)

clean:
	rm -f $(ALL.O) modeler
