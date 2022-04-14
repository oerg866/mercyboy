#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "video.h"
#include "sys.h"

#include "trace.h"

uint8_t     regs8  [0x0C];
uint16_t   *regs16 = (uint16_t*) regs8;

uint8_t     op;

uint8_t     cpu_ie;
uint8_t     cpu_ei_pending;
uint8_t     cpu_di_pending;

uint8_t     cpu_halted;

#include "cpu_alu.h"

void cpu_init() {

    pc = 0x0100; // set pc to 100
    sp = 0xFFFE; // Set stack to fffe

    cpu_ie = 0;
    cpu_ei_pending = 0;
    cpu_di_pending = 0;
    cpu_halted = 0;

}


volatile uint16_t owo = 0;

void cpu_step() {

    op = cpu_read8(pc);

    trace(TRACE_CPU, "tac: %02x if: %02x ime: %02x, ie: %02x, pc: %04x af: %04x bc: %04x de: %04x hl: %04x sp: %04x op: %02x ly: %02x\n",
    ram_io[0x07], SYS_IF, cpu_ie, ram_ie, pc, af, bc, de, hl, sp, op, video_get_line());

    // Jump Table
    if (op == 0xcb) {
        ipc(1);
        ext_opcodes[r8(pc)]();
    } else {
        opcodes[op]();
    }
}
