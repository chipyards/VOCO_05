# directories
# GTKBASE= F:/Appli/msys64/mingw32
# on ne depend plus d'un F: absolu, mais on doit compiler avec le shell mingw32
GTKBASE= /mingw32

# listes
SOURCESC= modpop3.c
SOURCESCPP= jluplot.cpp gluplot.cpp jdsp.cpp appli.cpp
HEADERS= glostru.h jdsp.h jluplot.h gluplot.h layer_u.h modpop3.h cli_parse.h
EXE= voco.exe

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
INCS= -Wall -Wno-parentheses -Wno-deprecated-declarations -O2 -mms-bitfields \
-I$(GTKBASE)/include/atk-1.0 \
-I$(GTKBASE)/include/cairo \
-I$(GTKBASE)/include/gdk-pixbuf-2.0 \
-I$(GTKBASE)/include/glib-2.0 \
-I$(GTKBASE)/include/gtk-2.0 \
-I$(GTKBASE)/include/harfbuzz \
-I$(GTKBASE)/include/pango-1.0 \
-I$(GTKBASE)/lib/glib-2.0/include \
-I$(GTKBASE)/lib/gtk-2.0/include \


# cibles

ALL : $(OBJS) 
	g++ -o $(EXE) $(OBJS) $(LIBS)

clean : 
	rm *.o

.cpp.o: 
	g++ $(INCS) -c $<

.c.o: 
	gcc $(INCS) -c $<

# dependances

jluplot.o : ${HEADERS}
gluplot.o : ${HEADERS}
jdsp.o : ${HEADERS}
appli.o : ${HEADERS}
