TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


DEFINES += DEBUG


win32-g++ {
DEFINES += INPUT_SDL2
DEFINES += AUDIO_SDL2
DEFINES += VIDEO_SDL2
LIBS += -lmingw32
}

linux-g++ {
DEFINES += INPUT_SDL2
DEFINES += AUDIO_SDL2
DEFINES += VIDEO_SDL2
}

contains(DEFINES, VIDEO_GDI) {
    LIBS += -lgdi32
}

contains(DEFINES, AUDIO_SDL2)|contains(DEFINES, VIDEO_SDL2)|contains(DEFINES, INPUT_SDL2) {
    LIBS += -lSDL2main
    LIBS += -lSDL2
}

contains(DEFINES, AUDIO_SDL2) {
    DEFINES += USE_AUDIO_TIMING
}

SOURCES += \
    main.c \
    cpu.c \
    cpu_alu.c \
    cpu_opcodes.c \
    mem.c \
    sys.c \
    video.c \
    audio.c \
    audio_sdl2.c \
    trace.c \
    video_sdl2.c \
    input_sdl2.c \
    input_win32.c \
    video_gdi.c

HEADERS += \
    cpu.h \
    mem.h \
    sys.h \
    video.h \
    audio.h \
    audio_sdl2.h \
    trace.h \
    input.h

DISTFILES += \
    LICENSE \
    README
