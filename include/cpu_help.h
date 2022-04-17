#ifndef CPU_HELP_H
#define CPU_HELP_H

/*  This file contains macros for use within the cpu_*.c source files. Their ambiguous and specific nature
    imply that including those in cpu.h or other source files would lead to MASSIVE CARNAGE. */

// These are for little endian only!
// Support for big endian platforms will come much (much (much)) later!!

#define a (regs8[1])
#define f (regs8[0])
#define b (regs8[3])
#define c (regs8[2])
#define d (regs8[5])
#define e (regs8[4])
#define h (regs8[7])
#define l (regs8[6])

#define af (regs16[0])
#define bc (regs16[1])
#define de (regs16[2])
#define hl (regs16[3])
#define sp (regs16[4])
#define pc (regs16[5])

#define r8(x) cpu_read8(x)
#define r8_signed(x) cpu_read8_signed(x)
#define w8(x, value) cpu_write8(x, value)
#define r16(x) cpu_read16(x)
#define w16(x, value) cpu_write16(x, value)

#define nextbyte r8(pc+1)

#define cycles(n) \
    sys_dma_cycles(n); \
    sys_cycles(n); \
    video_cycles(n);

#endif
