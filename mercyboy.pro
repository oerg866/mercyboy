TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += "D:\coding\SDL2\include"

LIBS += -L"D:\coding\SDL2\lib"
LIBS += -L"D:\coding\SDL2\bin"

win32-gcc {
LIBS += -lmingw32
}

LIBS += -lSDL2main
LIBS += -lSDL2

SOURCES += \
        main.c \
    cpu.c \
    cpu_alu.c \
    cpu_opcodes.c \
    cpu_mem.c \
    mem.c \
    sys.c \
    video.c

HEADERS += \
    cpu.h \
    mem.h \
    sys.h \
    video.h

DISTFILES += \
    LICENSE \
    README
