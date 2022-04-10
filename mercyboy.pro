TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


DEFINES += DEBUG

DEFINES_SDL2 = VIDEO_SDL2 AUDIO_SDL2 INPUT_SDL2
DEFINES_WIN32 = VIDEO_GDI AUDIO_WAVEOUT INPUT_WIN32
LIBS_SDL2 = -lSDL2main -lSDL2
LIBS_WIN32 = -lwinmm -lgdi32


win32-g++ {
    LIBS += -lmingw32
    LIBS += $${LIBS_WIN32}
    DEFINES += $${DEFINES_WIN32}
}

linux-g++ {
    DEFINES += $${DEFINES_SDL2}
}

SOURCES += \
    audio.c \
    backends.c \
    cpu.c \
    cpu_ops.c \
    main.c \
    main_imp.c \
    mem.c \
    ringbuf.c \
    sys.c \
    trace.c \
    video.c \


SOURCES += \
    video/v_gdi.c \
    video/v_sdl2.c \
    audio/a_dummy.c \
    audio/a_sdl2.c \
    audio/a_wvout.c \
    input/i_win32.c \
    input/i_sdl2.c \

INCLUDEPATH = include/

HEADERS += \
    include/backends.h \
    include/bendlist.h \
    include/compat.h \
    include/cpu.h \
    include/cpu_alu.h \
    include/cpu_help.h \
    include/mem.h \
    include/ringbuf.h \
    include/sys.h \
    include/video.h \
    include/audio.h \
    include/trace.h

DISTFILES += \
    LICENSE \
    README
