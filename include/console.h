/* Universal console functionality */

#include "compat.h"

#if defined(__WIN16__) || (defined(_MSC_VER) && _MSC_VER <= 1000)
static inline void print_msg(char* fmt, ...) {
#include <stdarg.h>
    char out_string[1024];
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(out_string, sizeof(out_string), fmt, ap);
    OutputDebugStringA(out_string);
    va_end(ap);
}
#elif defined(__DJGPP__)

#include <pc.h>
#include <stdio.h>
#define print_msg(...) { printf(__VA_ARGS__); }

#else

#include <stdio.h>
#define print_msg printf

#endif
