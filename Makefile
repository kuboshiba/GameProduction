CC = gcc

CFLAGS = -O2 -Wall $(COPTIONS)

LDLIBS = -no-pie -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_gfx -lcwiimote

TARGET = main
OBJS = $(TARGET).o input.o system.o define.o server.o client.o
HEADS = header/define.h

$(TARGET): $(OBJS)
$(OBJS): $(HEADS)

clean:
	@rm -f $(TARGET) $(OBJS) core *~