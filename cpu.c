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

    opcodes[op]();

    if ((op != 0xF3) && (op != 0xFB)) {
        if (cpu_ei_pending) {
            cpu_ie = 1;
            cpu_ei_pending = 0;
        } else if (cpu_di_pending) {
            cpu_ie = 0;
            cpu_di_pending = 0;
        }
    }

    if (op != 0x76)
        process_interrupts();   // Don't process interrupts again if we've just returned from a HALT
}

uint16_t process_interrupts() {

    uint16_t i;

    if (cpu_ie || cpu_halted) {
        for (i = 0; i < 5; i++) {
            if ((ram_ie & SYS_IF) & (1 << i)) {
                // if an int is enabled and pending, service it

                trace(TRACE_INT, "Servicing interrupt index %i\n", i);

                // step 1: disable interrupt master enable
                // also clear bit in IF register

                cpu_ei_pending = 0;
                cpu_ie = 0;
                sys_interrupt_clear(1 << i);


                // step 2: push pc onto stack

                sp -= 2;
                cpu_write16(sp, pc);

                // step 3: new PC is 0x40 + int index

                pc = 0x40 + (i << 3);

                // 12 cycles i think??
                cycles(12);
                return 1;
            } else if (cpu_halted && (SYS_IF & (1 << i))) {
                // if we're in a HALT, IME is 0 and an interrupt flag gets set
                // just resume execution
                return 1;
            }


        }

    }
    return 0;

}
