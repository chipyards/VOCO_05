# directories
GTKBASE= F:/Appli/msys64/mingw32

# listes
SOURCESC=
SOURCESCPP= jluplot.cpp gluplot.cpp layers.cpp jdsp.cpp appli.cpp
HEADERS= glostru.h jdsp.h jluplot.h gluplot.h layers.h
EXE= jluplot.exe

OBJS= $(SOURCESC:.c=.o) $(SOURCESCPP:.cpp=.o)

# maintenir les libs et includes dans l'ordre alphabetique SVP

# LIBS= `pkg-config --libs gtk+-2.0`
LIBS= -L$(GTKBASE)/lib \
-latk-1.0 \
-lcairo \
-lgdk-win32-2.0 \
-lgdk_pixbuf-2.0 \
-lglib-2.0 \
-lgmodule-2.0 \
-lgobject-2.0 \
-lgtk-win32-2.0 \
-lpango-1.0 \
-lpangocairo-1.0 \
-lpangowin32-1.0 \
# -mwindows
# enlever -mwindows pour avoir la console stdout

# INCS= `pkg-config --cflags gtk+-2.0` -mms-bitfields
INCS= -mms-bitfields \
-I$(GTKBASE)/include/atk-1.0 \
-I$(GTKBASE)/include/cairo \
-I$(GTKBASE)/include/gdk-pixbuf-2.0 \
-I$(GTKBASE)/include/glib-2.0 \
-I$(GTKBASE)/include/gtk-2.0 \
-I$(GTKBASE)/include/pango-1.0 \
-I$(GTKBASE)/lib/glib-2.0/include \
-I$(GTKBASE)/lib/gtk-2.0/include \


# cibles

ALL : $(OBJS) 
	g++ -o $(EXE) $(OBJS) $(LIBS)

clean : 
	del *.o

.cpp.o: 
	g++ $(INCS) -c $<

.c.o: 
	gcc $(INCS) -c $<

# dependances

jluplot.o : ${HEADERS}
gluplot.o : ${HEADERS}
layers.o : ${HEADERS}
jdsp.o : ${HEADERS}
appli.o : ${HEADERS}
