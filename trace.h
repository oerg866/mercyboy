
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
#include <stdint.h>

void trace_init(uint8_t enabled, uint8_t print, FILE *file);
void trace(uint8_t trace_lvl, char* fmt, ...);

#endif // TRACE_H
