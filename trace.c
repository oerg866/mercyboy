#if defined(DEBUG)

#include "trace.h"

#include <stdio.h>
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
    trace_file = file;
    trace_enabled = enabled;
    trace_print = print;
}

void trace_deinit() {
    if (trace_file)
        fclose(trace_file);
}

void trace(uint8_t trace_lvl, char* fmt, ...) {
    char out_string[1024];
    va_list ap;
    uint8_t i;

    if (trace_enabled & trace_lvl) {
        va_start(ap, fmt);

        for (i = 0; i < 8; i++) {
            if (trace_lvl & 1) break;
            trace_lvl = trace_lvl >> 1;
        }
        vsprintf(out_string, fmt, ap);
        if (trace_file)
            fprintf(trace_file, "%s %s", trace_strings[i], out_string);

        if (trace_print)
            print_msg("%s %s", trace_strings[i], out_string);

        va_end(ap);
    }
}

#endif // defined(DEBUG)
