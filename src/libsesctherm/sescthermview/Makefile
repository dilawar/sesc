CC = g++
LIB_DIRS=-L/lib -L/usr/local/lib 
LDLIBS=-lfltk -lfltk_gl -lGL -lGLU -lm -LX11 
#LDLIBS=-lfltk_gl -lfltk -glu32 -lopengl32 -lgdi32 -lwsock32
CFLAGS=-g -I. -I/usr/include 

OBJECTS= SESCthermViewMain.o SESCthermView.o SESCthermViewUI.o

all: SESCthermView

SESCthermView: $(OBJECTS)
	${CC} $(OBJECTS) ${LIB_DIRS} ${LDLIBS} -o SESCthermView

SESCthermViewMain.o: SESCthermViewMain.cxx
	${CC} ${CFLAGS} -c SESCthermViewMain.cxx

SESCthermView.o: SESCthermView.cxx SESCthermView.h
	${CC} ${CFLAGS} -c SESCthermView.cxx

SESCthermViewUI.o: SESCthermView.cxx SESCthermViewUI.h
	${CC} ${CFLAGS} -c SESCthermViewUI.cxx 

clean:
	-rm *.o SESCthermView

