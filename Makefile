CFLAGS = -Wall -Wextra -Os
LIBS = -lm

CFLAGS_SDL2 = -DVIDEO_SDL2 -DAUDIO_SDL2 -DINPUT_SDL2 -DUSE_AUDIO_TIMING

# Building with an older version of MINGW32
ifeq ($(MAKE),mingw32-make)
	CC = gcc
endif

ifeq ($(OS),Windows_NT)
	EXECUTABLE = mercyboy.exe
	LIBS_SDL2 = -lSDL2main -lSDL2
	LIBS += -lmingw32
else
	EXECUTABLE = mercyboy
	CFLAGS_SDL2 = $(shell sdl2-config --cflags)
	LIBS_SDL2 = $(shell sdl2-config --libs)
endif

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

CFLAGS += $(CFLAGS_SDL2)
LIBS += $(LIBS_SDL2)

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LIBS)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all clean
