# Configuration
# Select backends by setting the variables BACKEND_VIDEO, BACKEND_AUDIO and BACKEND_INPUT.
# Select timing options by setting the variable TIMING.
# Possible values:
# BACKEND_VIDEO: sdl (default), gdi
# BACKEND_AUDIO: sdl (default)
# BACKEND_INPUT: sdl (default), win32
# TIMING: audio (default), delay

# Handle default values
ifeq ($(BACKEND_VIDEO),)
	BACKEND_VIDEO = sdl
endif
ifeq ($(BACKEND_AUDIO),)
	BACKEND_AUDIO = sdl
endif
ifeq ($(BACKEND_INPUT),)
	BACKEND_INPUT = sdl
endif
ifeq ($(TIMING),)
	TIMING = audio
endif

# Remember which libraries need to be linked.
NEED_SDL = no
NEED_GDI = no
NEED_WIN32 = no

CFLAGS = -Wall -Wextra -Os
LIBS = -lm

# Video backend
ifeq ($(BACKEND_VIDEO),sdl)
	CFLAGS += -DVIDEO_SDL2
	NEED_SDL = yes
endif
ifeq ($(BACKEND_VIDEO),gdi)
	CFLAGS += -DVIDEO_GDI
	NEED_GDI = yes
endif

# Audio backend
ifeq ($(BACKEND_AUDIO),sdl)
	CFLAGS += -DAUDIO_SDL2
	NEED_SDL = yes
endif

# Input backend
ifeq ($(BACKEND_INPUT),sdl)
	CFLAGS += -DINPUT_SDL2
	NEED_SDL = yes
endif
ifeq ($(BACKEND_INPUT),win32)
	CFLAGS += -DINPUT_WIN32
	NEED_WIN32 = yes
endif

# Timing
ifeq ($(TIMING),audio)
	CFLAGS += -DUSE_AUDIO_TIMING
endif

# Building with an older version of MINGW32
ifeq ($(MAKE),mingw32-make)
	CC = gcc
endif

ifeq ($(NEED_SDL),yes)
	ifeq ($(OS),Windows_NT)
		LIBS += -lmingw32 -lSDL2main -lSDL2
	else
		CFLAGS += $(shell sdl2-config --cflags)
		LIBS += $(shell sdl2-config --libs)
	endif
endif
ifeq ($(NEED_GDI),yes)
	# TODO
endif
ifeq ($(NEED_WIN32),yes)
	# TODO
endif

# Set executable name
ifeq ($(OS),Windows_NT)
	EXECUTABLE = mercyboy.exe
else
	EXECUTABLE = mercyboy
endif

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LIBS)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all clean
