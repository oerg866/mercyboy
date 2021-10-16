CFLAGS = -Wall -Wextra -Os
LIBS = -lm

CFLAGS += -DVIDEO_SDL2 -DAUDIO_SDL2 -DINPUT_SDL2 $(shell sdl2-config --cflags)
LIBS += $(shell sdl2-config --libs)

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

all: mercyboy

mercyboy: $(SOURCES) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o mercyboy $(SOURCES) $(LIBS)

clean:
	rm -f mercyboy

.PHONY: all clean
