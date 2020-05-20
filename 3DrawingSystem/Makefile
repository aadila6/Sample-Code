CC = /usr/bin/g++

#OPENGL LIBS FOR LINUX/CSIF
GLLIB :=  -lGL -lGLEW -lGLU -lglut
#OPENGL LIBS FOR MAC
#GLLIB := -framework OpenGL -framework GLUT

#COMPILER FLAGS
CCFLAGS := -std=c++11 -Wno-deprecated
ifneq ($(D),1)
CCFLAGS += -O2
else
CCFLAGS += -O0
CCFLAGS += -g
endif

#include directories
#should include gl.h glut.h etc...
LDLIBS := $(GLLIB)

TARGET = main
OBJS = main.o


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC)  $^ $(CCFLAGS) $(LDLIBS)  -o $@

%.o : %.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<

clean:
	rm -rf $(OBJS) $(TARGET)

