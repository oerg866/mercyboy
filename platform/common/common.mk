# MercyBoy makefile common variables

CFLAGS = -Wall -Wextra -Wfatal-errors -Wno-unused-parameter -Wno-missing-field-initializers -Os
LIBS = -lm

ifeq ($(DEBUG),1)
	CFLAGS += -g -DDEBUG
endif

ComSpec ?= $(COMSPEC)

ifeq ($(ComSpec),)
	DELETE = rm -f
else
	DELETE = $(ComSpec) \/C del
endif

# CC may not be defined when using mingw32-make, so we assume gcc(djgpp)
ifeq ($(MAKE),mingw32-make)
	CC = gcc
endif

LIBS_WIN32 = -lgdi32 -lwinmm
DEFS_WIN32 = -DVIDEO_GDI -DAUDIO_WAVEOUT -DINPUT_WIN32
