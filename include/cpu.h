#pragma once

#ifndef CPU_H
#define CPU_H

#include "compat.h"

#define REG_AF  0x00
#define REG_BC  0x01
#define REG_DE  0x02
#define REG_HL  0x03
#define REG_SP  0x04
#define REG_PC  0x05

#define REG_A   0x01
#define REG_F   0x00
#define REG_B   0x03
#define REG_C   0x02
#define REG_D   0x05
#define REG_E   0x04
#define REG_H   0x07
#define REG_L   0x06

#define COND_NZ 0x00
#define COND_Z  0x01
#define COND_NC 0x02
#define COND_C  0x03

#define FLAG_Z  (1<<7)
#define FLAG_N  (1<<6)
#define FLAG_H  (1<<5)
#define FLAG_C  (1<<4)

#define INT_NONE        0
#define INT_PENDING     1
#define INT_SERVICED    2

extern uint8_t      regs8[];
extern uint16_t    *regs16;

extern uint8_t      op;

extern uint8_t      cpu_ie;
extern uint8_t      cpu_ei_pending;
extern uint8_t      cpu_di_pending;

extern uint8_t      cpu_halted;

// CPU.C

void cpu_init();
void cpu_step();

// MEM.C

uint8_t cpu_read8(uint16_t addr);
int8_t cpu_read8_signed(uint16_t addr);
uint16_t cpu_read16(uint16_t addr);

void cpu_write8(uint16_t addr, uint8_t n);
void cpu_write16(uint16_t addr, uint16_t n);

typedef void (*op_func) (void);

extern const op_func opcodes[256];
extern const op_func ext_opcodes[256];

// Set 8 bit register reg to n
#define sr8(reg, n) regs8[reg] = n
// Set 16 bit register reg to n
#define sr16(reg, n) regs16[reg] = n

// Set PC to n
#define spc(n) pc = n
// Increment PC by n
#define ipc(n) pc += n

#define alu_flags(z, n, h, c) *flags = z|n|h|c

#define _cpu_flags = (reg8[REG_F])
#define _cpu_sp = (reg16[REG_SP])
#define _cpu_pc = (regs16[REG_PC])

#endif // CPU_H
