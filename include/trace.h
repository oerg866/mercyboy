
#ifndef TRACE_H
#define TRACE_H

// Definitions for tracing levels

#define TRACE_CPU   (1<<0)
#define TRACE_AUDIO (1<<1)
#define TRACE_VIDEO (1<<2)
#define TRACE_SYS   (1<<3)
#define TRACE_INT   (1<<4)
#define TRACE_MBC   (1<<5)
#define TRACE_ALL   0xff

#include <stdio.h>

#include "compat.h"

#if defined(__WIN16__) || (defined(_MSVC_VER) && _MSVC_VER <= 1000)
static inline void print_msg(char* fmt, ...) {
    char out_string[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(out_string, sizeof(out_string), fmt, ap);
    OutputDebugStringA(out_string);
    va_end(ap);
}
#else
#define print_msg printf
#endif

#if defined(DEBUG)
void trace_init(uint8_t enabled, uint8_t print, FILE *file);
void trace(uint8_t trace_lvl, char* fmt, ...);
#else 
inline void trace_init(uint8_t enabled, uint8_t print, FILE *file) {}
inline void trace(uint8_t trace_lvl, char* fmt, ...) {}
#endif

#endif // TRACE_H
