#include "trace.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

uint8_t     trace_enabled   = 0;    // enabled trace levels
uint8_t     trace_print     = 0;    // if traces should be printed to console
FILE       *trace_file      = NULL;

const char *trace_strings[9] = {
    "CPU:   ",
    "SOUND: ",
    "VIDEO: ",
    "SYS:   ",
    "INT:   ",
    "MBC:   ",
    "UNK:   ",
    "UNK:   ",
    "UNK:   "
};

void trace_init(uint8_t enabled, uint8_t print, FILE *file) {
#ifdef DEBUG
    trace_file = file;
    trace_enabled = enabled;
    trace_print = print;
#endif
}

void trace_deinit() {
#ifdef DEBUG
    if (trace_file)
        fclose(trace_file);
#endif
}

void trace(uint8_t trace_lvl, char* fmt, ...) {
#ifdef DEBUG

    if (trace_enabled & trace_lvl) {
        va_list ap;
        va_start(ap, fmt);
        uint8_t i;
        for (i = 0; i < 8; i++) {
            if (trace_lvl & 1) break;
            trace_lvl = trace_lvl >> 1;
        }
        if (trace_file) {
            fprintf(trace_file, trace_strings[i]);
            vfprintf(trace_file, fmt, ap);
        }
        if (trace_print) {
            printf(trace_strings[i]);
            vprintf(fmt, ap);
        }
        va_end(ap);
    }

#endif
}
