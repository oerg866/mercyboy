include platform/common/common.mk

# Defaults

USE_SDL2 := 1

# Targets, little hack to remove unsupported stuff for old OSs or compilers

ifdef TARGET

	ifeq ($(TARGET),WIN9X)
		EXECUTABLE = mercyboy.exe
		USE_SDL2 = 0
		LIBS += $(LIBS_WIN32)
		DEFS += $(DEFS_WIN32)
	endif

	ifeq ($(TARGET),DOS)
		EXECUTABLE = mercyboy.exe
		USE_SDL2 = 0
		DEFS += $(DEFS_DOS)
	endif

else ifeq ($(OS),Windows_NT) # Default if we're on modernish windows
		EXECUTABLE = mercyboy.exe
		LIBS_SDL2 = -lSDL2main -lSDL2
		LIBS += $(LIBS_WIN32)
		DEFS += $(DEFS_WIN32)

else
		EXECUTABLE = mercyboy
		CFLAGS_SDL2 = $(shell sdl2-config --cflags)
		LIBS_SDL2 = $(shell sdl2-config --libs)

endif


ifeq ($(USE_SDL2),1)
	LIBS += $(LIBS_SDL2)
	CFLAGS += $(CFLAGS_SDL2)
	DEFS += -DVIDEO_SDL2 -DAUDIO_SDL2 -DINPUT_SDL2
endif

# Building with an older version of MINGW32
ifeq ($(MAKE),mingw32-make)
	LIBS += -lmingw32
	CC = gcc
endif

# Generic main.c for all compilers except those who refuse to acknowledge argc/argv
MAIN_FILES = platform/_generic.c platform/main_def.c

SOURCES = $(MAIN_FILES) $(wildcard *.c) $(wildcard audio/*.c) $(wildcard input/*.c) $(wildcard video/*.c)
HEADERS = $(wildcard include/*.h) $(wildcard audio/*.h) $(wildcard input/*.h) $(wildcard video/*.h)

INCLUDES = -Iinclude -Ibackends

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS) Makefile
	$(CC) $(CFLAGS) $(DEFS) -o $(EXECUTABLE) $(INCLUDES) $(SOURCES) $(LIBS)

clean:
	$(DELETE) $(EXECUTABLE)

.PHONY: all clean
