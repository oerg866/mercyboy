include platform/common/common.mk

# Defaults

USE_SDL2 := 1

EXECUTABLE = mercyboy.exe

DEFS = -DVIDEO_DOSVGA -DINPUT_DOS32 -DAUDIO_SB16DJ

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
