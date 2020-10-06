cc = gcc

CFLAGS = -Wall $(COPTIONS)

LDLIBS = -lm -lSDL2 -lSDL2_image -lSDL2_ttf -Wl,-rpath,/usr/local/lib

TARGET = main
OBJS = $(TARGET).o
HEADS = header/define.h

$(TARGET): $(OBJS)
$(OBJS): $(HEADS)