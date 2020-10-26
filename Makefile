CC = gcc

CFLAGS = -Wall $(COPTIONS)

LDLIBS = -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_gfx -lcwiimote -Wl,-rpath,/usr/local/lib

TARGET = main
OBJS = $(TARGET).o system.o input.o define.o
HEADS = header/define.h

$(TARGET): $(OBJS)
$(OBJS): $(HEADS)

clean:
	@rm -f $(TARGET) $(OBJS) core *~