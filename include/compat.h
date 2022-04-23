#pragma once

/*
 *  COMPATIBILITY LAYER TO ACHIEVE PORTABILITY
 *
 *  Supported platforms:
 *
 *      GCC (32/64-Bit)
 *      Microsoft Visual C++ 1.0 (32-Bit)
 *      Microsoft Visual C++ 4.2 (32-Bit)
 *
 *  Planned:
 *      OpenWatcom / DJGPP (DOS32)
 *      Microsoft Visual C++ 1.52 (16-Bit)
 */

#ifndef COMPAT_H
#define COMPAT_H

#define VERSION_STRING "0.x"

// Figure out platform variables for Win16 and really old MSC compilers

#if defined(_MSC_VER) && !defined(_WIN32)
#define __WIN16__
#endif

#if defined(_MSC_VER) && MSC_VER <= 800
#define __OLDMSC
#endif

// Main function :-)
extern int main_default(int argc, char *argv[]);

// MSVC windows.h include

#if defined (__WIN32__) || (defined (_WIN32) && defined(_MSC_VER))
#include <windows.h>
#endif

/*
 *  Windows 16-Bit & early 32-Bit Integer types
 */

#if defined (__WIN16__) || (defined (_MSC_VER) && _MSC_VER < 1300)

#include <limits.h>

#define int8_t      signed char
#define int16_t     signed short
#define int32_t     signed long

#define uint8_t     unsigned char
#define uint16_t    unsigned short
#define uint32_t    unsigned long


#define INT8_MIN    CHAR_MIN
#define INT16_MIN   SHRT_MIN
#define INT32_MIN   LONG_MIN

#define INT8_MAX    CHAR_MAX
#define INT16_MAX   SHRT_MAX
#define INT32_MAX   LONG_MAX

#define UINT8_MIN   UCHAR_MIN
#define UINT16_MIN  USHRT_MIN
#define UINT32_MIN  ULONG_MIN

#define UINT8_MAX   UCHAR_MAX
#define UINT16_MAX  USHRT_MAX
#define UINT32_MAX  ULONG_MAX

#elif defined(__GNUC__)

#include <stdint.h>

#endif

/*
 *  Debug Mode for Win16
 */

#if defined (__WIN16__) && defined (_DEBUG)
#define DEBUG
#endif

/*
 *  Memory allocation... I don't know how to solve this elegantly
 */

#if defined (__WIN16__)

#include <malloc.h>
#define alloc_mem(x) _halloc(x, 1)

#elif defined(_MSC_VER)

#include <malloc.h>
#define alloc_mem malloc

#else

#define alloc_mem malloc

#endif

/*
 *  Microsoft Visual C++ old version inline keyword
 */

#if defined (_MSC_VER) || defined(__WIN16__)

#define inline _inline

#endif

// Sleep & Yield function

#if defined (__WIN16__) || defined(__WIN32__) || defined(_WIN32)
#define sleep_ms(x) Sleep(x)
#define yield() Sleep(0)
#else

inline void sleep_ms(int ms) {

}

#include <sched.h>
#define yield sched_yield

#endif

/*
 *  TEXT macro for Windows 16-Bit
 */

#if defined (__WIN16__)

#define TEXT(y) y

#endif

/*
 *  Byteswapping
 */

#if defined(__WIN32__) || (defined(_MSC_VER) && _MSC_VER >= 1300)

/* generic modern Win32 implementation */

#include <stdlib.h>
#define bs _byteswap_ushort

#elif defined(__linux__)

/* Linux implementation */

#include <byteswap.h>
#define bs bswap_16

#else

/* Machine independent implementation (e.g. win16 doesn't seem to have anything for this) */

inline uint16_t bs (uint16_t val) {
    return (val << 8) | (val >> 8);
}

#endif

/* MIN / MAX */

#ifndef MAX
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef bool
#define bool uint8_t
#endif

#ifndef true
#define true 1
#define false 0
#endif

/* Primitive semaphore using a single boolean */
#define _lock volatile uint8_t

#endif

/* DWORD_PTR is iffy because it is 8 bytes on 64-bit, older MSVCs don't have this type actually */

#if (defined(_MSC_VER) && (!defined(DWORD_PTR)))
#define DWORD_PTR DWORD
#endif
