CC = /usr/bin/g++

#OPENGL LIBS FOR LINUX/CSIF
GLLIB :=  -lGL -lGLEW -lGLU -lglut
#OPENGL LIBS FOR MAC
#GLLIB := -framework OpenGL -framework GLUT

#COMPILER FLAGS
CCFLAGS := -std=c++17 -Wno-deprecated
ifneq ($(D),1)
CCFLAGS += -O2
else
CCFLAGS += -O0
CCFLAGS += -g
endif

#include directories
#should include gl.h glut.h etc...
LDLIBS := $(GLLIB) 
INCDIR = -Iimgui-1.74 -Iimgui-1.74/examples/

TARGET = main.x
SOURCES = main.cpp
SOURCES += imgui-1.74/examples/imgui_impl_glut.cpp imgui-1.74/examples/imgui_impl_opengl2.cpp
SOURCES += imgui-1.74/imgui.cpp imgui-1.74/imgui_demo.cpp imgui-1.74/imgui_draw.cpp imgui-1.74/imgui_widgets.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC)  $^ $(CCFLAGS) $(LDLIBS)  -o $@

%.o : %.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<

%.o : imgui-1.74/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<	

%.o : imgui-1.74/examples/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<

imgui-1.74/examples/%.o: %.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<

clean:
	rm -rf $(OBJS) $(TARGET)
