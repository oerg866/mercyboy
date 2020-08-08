#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "video.h"
#include "sys.h"
#include "trace.h"

uint8_t     regs8  [0x0C];
uint16_t   *regs16 = (uint16_t*) regs8;

uint16_t   *pc = (uint16_t*) (regs8) + REG_PC;
uint16_t   *hl = (uint16_t*) (regs8) + REG_HL;
uint16_t   *bc = (uint16_t*) (regs8) + REG_BC;
uint16_t   *de = (uint16_t*) (regs8) + REG_DE;
uint16_t   *sp = (uint16_t*) (regs8) + REG_SP;

uint8_t    *flags;

uint8_t     op;

uint8_t     cpu_ie;
uint8_t     cpu_ei_pending;
uint8_t     cpu_di_pending;

uint8_t     cpu_halted;

inline void sr8(uint16_t reg, uint8_t n) {
    // set reg 8 bit
    regs8[reg] = n;
}
inline void sr16(uint16_t reg, uint16_t n) {
    // set reg 16 bit
    regs16[reg] = n;
}


inline void spc(uint16_t n) {
    *pc = n;           // set pc
//    sr16(REG_SP, *pc); // set pc in regs
}

inline void ipc(uint16_t n) {
    *pc += n;
//    sr16(REG_SP, *pc);
}

void cpu_init() {

    flags = &regs8[REG_F];
    spc(0x0100); // set pc to 100
    sr16(REG_SP, bs(0xFFFE)); // Set stack to fffe

    cpu_ie = 0;
    cpu_ei_pending = 0;
    cpu_di_pending = 0;
    cpu_halted = 0;

}

void run() {

    while (sys_running) {

        op = cpu_read8(*pc);

        trace(TRACE_CPU, "tac: %02x if: %02x ime: %02x, ie: %02x, pc: %04x af: %04x bc: %04x de: %04x hl: %04x sp: %04x op: %02x ly: %02x\n",
          ram_io[0x07], SYS_IF, cpu_ie, ram_ie, *pc, bs(regs16[REG_AF]), bs(*bc), bs(*de), bs(*hl), bs(*sp), op, ram_io[0x44]);

        switch(op) {

        case 0x78: op_ld_a_r(); break;          // ld a, r
        case 0x79: op_ld_a_r(); break;
        case 0x7a: op_ld_a_r(); break;
        case 0x7b: op_ld_a_r(); break;
        case 0x7c: op_ld_a_r(); break;
        case 0x7d: op_ld_a_r(); break;
        case 0x7e: op_ld_a_ind_hl(); break;
        case 0x7f: op_ld_a_r(); break;          // ld a, a
        case 0x0a: op_ld_a_ind_bc(); break;     // ld a, (bc)
        case 0x1a: op_ld_a_ind_de(); break;     // ld a, (de)
        case 0xfa: op_ld_a_ind_mem(); break;     // ld a, (nn)


        case 0x06: op_ld_r_imm(); break;        // ld r, #n
        case 0x0e: op_ld_r_imm(); break;        //
        case 0x16: op_ld_r_imm(); break;        //
        case 0x1e: op_ld_r_imm(); break;        //
        case 0x26: op_ld_r_imm(); break;        //
        case 0x2e: op_ld_r_imm(); break;        //
        case 0x3e: op_ld_r_imm(); break;        // ld a, #n


        case 0x40: op_ld_r_r(); break;          // ld b, r
        case 0x41: op_ld_r_r(); break;
        case 0x42: op_ld_r_r(); break;
        case 0x43: op_ld_r_r(); break;
        case 0x44: op_ld_r_r(); break;
        case 0x45: op_ld_r_r(); break;
        case 0x46: op_ld_r_ind_hl(); break;
        case 0x47: op_ld_r_r(); break;          // ld b, a

        case 0x48: op_ld_r_r(); break;          // ld c, r
        case 0x49: op_ld_r_r(); break;
        case 0x4a: op_ld_r_r(); break;
        case 0x4b: op_ld_r_r(); break;
        case 0x4c: op_ld_r_r(); break;
        case 0x4d: op_ld_r_r(); break;
        case 0x4e: op_ld_r_ind_hl(); break;
        case 0x4f: op_ld_r_r(); break;          // ld c, a

        case 0x50: op_ld_r_r(); break;          // ld d, r
        case 0x51: op_ld_r_r(); break;
        case 0x52: op_ld_r_r(); break;
        case 0x53: op_ld_r_r(); break;
        case 0x54: op_ld_r_r(); break;
        case 0x55: op_ld_r_r(); break;
        case 0x56: op_ld_r_ind_hl(); break;
        case 0x57: op_ld_r_r(); break;          // ld d, a

        case 0x58: op_ld_r_r(); break;          // ld e, r
        case 0x59: op_ld_r_r(); break;
        case 0x5a: op_ld_r_r(); break;
        case 0x5b: op_ld_r_r(); break;
        case 0x5c: op_ld_r_r(); break;
        case 0x5d: op_ld_r_r(); break;
        case 0x5e: op_ld_r_ind_hl(); break;
        case 0x5f: op_ld_r_r(); break;          // ld e, a

        case 0x60: op_ld_r_r(); break;          // ld h, r
        case 0x61: op_ld_r_r(); break;
        case 0x62: op_ld_r_r(); break;
        case 0x63: op_ld_r_r(); break;
        case 0x64: op_ld_r_r(); break;
        case 0x65: op_ld_r_r(); break;
        case 0x66: op_ld_r_ind_hl(); break;
        case 0x67: op_ld_r_r(); break;          // ld h, a

        case 0x68: op_ld_r_r(); break;          // ld l, r
        case 0x69: op_ld_r_r(); break;
        case 0x6a: op_ld_r_r(); break;
        case 0x6b: op_ld_r_r(); break;
        case 0x6c: op_ld_r_r(); break;
        case 0x6d: op_ld_r_r(); break;
        case 0x6e: op_ld_r_ind_hl(); break;
        case 0x6f: op_ld_r_r(); break;          // ld l, a

        case 0x70: op_ld_ind_hl_r(); break;     // ld (hl), r
        case 0x71: op_ld_ind_hl_r(); break;
        case 0x72: op_ld_ind_hl_r(); break;
        case 0x73: op_ld_ind_hl_r(); break;
        case 0x74: op_ld_ind_hl_r(); break;
        case 0x75: op_ld_ind_hl_r(); break;
        case 0x36: op_ld_ind_hl_imm(); break;
        case 0x77: op_ld_ind_hl_r(); break;     // ld (hl), a

        case 0x02: op_ld_ind_bc_a(); break;     // ld (bc), a
        case 0x12: op_ld_ind_de_a(); break;     // ld (de), a
        case 0xEA: op_ld_ind_mem_a(); break;    // ld (nn), a

        case 0xF2: op_ld_a_c(); break;          // ld a, (c)
        case 0xE2: op_ld_c_a(); break;          // ld (c), a

        case 0x3A: op_ldd_a_ind_hl(); break;    // ld a, (hl-)
        case 0x32: op_ldd_ind_hl_a(); break;    // ld (hl-), a

        case 0x2A: op_ldi_a_ind_hl(); break;    // ld a, (hl+)
        case 0x22: op_ldi_ind_hl_a(); break;    // ld (hl+), a

        case 0xE0: op_ldh_n_a(); break;         // ldh (ff00+n), a
        case 0xF0: op_ldh_a_n(); break;         // ldh a, (ff00+n)

        case 0x01: op_ld_r_imm16(); break;      // ld rr, #nn
        case 0x11: op_ld_r_imm16(); break;      // ld rr, #nn
        case 0x21: op_ld_r_imm16(); break;      // ld rr, #nn
        case 0x31: op_ld_r_imm16(); break;      // ld rr, #nn

        case 0xF9: op_ld_sp_hl(); break;        // ld sp, hl

        case 0xF8: op_ldhl_sp_imm(); break;     // ldhl sp, n

        case 0x08: op_ld_ind_imm_sp(); break;   // ld (nn), sp

        case 0xF5: op_push16(); break;          // push rr
        case 0xC5: op_push16(); break;          //
        case 0xD5: op_push16(); break;          //
        case 0xE5: op_push16(); break;          //

        case 0xF1: op_pop16(); break;          // pop rr
        case 0xC1: op_pop16(); break;          //
        case 0xD1: op_pop16(); break;          //
        case 0xE1: op_pop16(); break;          //

        case 0x80: op_add8_a_r(); break;        // add a, r
        case 0x81: op_add8_a_r(); break;
        case 0x82: op_add8_a_r(); break;
        case 0x83: op_add8_a_r(); break;
        case 0x84: op_add8_a_r(); break;
        case 0x85: op_add8_a_r(); break;
        case 0x87: op_add8_a_r(); break;
        case 0x86: op_add8_a_ind_hl(); break;   // add a, (hl)
        case 0xC6: op_add8_a_imm(); break;      // add a, #n

        case 0x88: op_adc_a_r(); break;        // adc a, r
        case 0x89: op_adc_a_r(); break;
        case 0x8A: op_adc_a_r(); break;
        case 0x8B: op_adc_a_r(); break;
        case 0x8C: op_adc_a_r(); break;
        case 0x8D: op_adc_a_r(); break;
        case 0x8F: op_adc_a_r(); break;
        case 0x8E: op_adc_a_ind_hl(); break;   // adc a, (hl)
        case 0xCE: op_adc_a_imm(); break;      // adc a, #n

        case 0x90: op_sub_a_r(); break;        // sub a, r
        case 0x91: op_sub_a_r(); break;
        case 0x92: op_sub_a_r(); break;
        case 0x93: op_sub_a_r(); break;
        case 0x94: op_sub_a_r(); break;
        case 0x95: op_sub_a_r(); break;
        case 0x97: op_sub_a_r(); break;
        case 0x96: op_sub_a_ind_hl(); break;   // sub a, (hl)
        case 0xD6: op_sub_a_imm(); break;      // sub a, #n

        case 0x98: op_sbc_a_r(); break;        // sbc a, r
        case 0x99: op_sbc_a_r(); break;
        case 0x9A: op_sbc_a_r(); break;
        case 0x9B: op_sbc_a_r(); break;
        case 0x9C: op_sbc_a_r(); break;
        case 0x9D: op_sbc_a_r(); break;
        case 0x9F: op_sbc_a_r(); break;
        case 0x9E: op_sbc_a_ind_hl(); break;   // sbc a, (hl)
        case 0xDE: op_sbc_a_imm(); break;      // sbc a, #n

        case 0xA0: op_and_a_r(); break;         // and r
        case 0xA1: op_and_a_r(); break;         //
        case 0xA2: op_and_a_r(); break;         //
        case 0xA3: op_and_a_r(); break;         //
        case 0xA4: op_and_a_r(); break;         //
        case 0xA5: op_and_a_r(); break;         //
        case 0xA7: op_and_a_r(); break;         //
        case 0xA6: op_and_a_ind_hl(); break;    // and (hl)
        case 0xE6: op_and_a_imm(); break;       // and #n

        case 0xB0: op_or_a_r(); break;         // or r
        case 0xB1: op_or_a_r(); break;         //
        case 0xB2: op_or_a_r(); break;         //
        case 0xB3: op_or_a_r(); break;         //
        case 0xB4: op_or_a_r(); break;         //
        case 0xB5: op_or_a_r(); break;         //
        case 0xB7: op_or_a_r(); break;         //
        case 0xB6: op_or_a_ind_hl(); break;    // or (hl)
        case 0xF6: op_or_a_imm(); break;       // or #n

        case 0xA8: op_xor_a_r(); break;         // xor r
        case 0xA9: op_xor_a_r(); break;         //
        case 0xAA: op_xor_a_r(); break;         //
        case 0xAB: op_xor_a_r(); break;         //
        case 0xAC: op_xor_a_r(); break;         //
        case 0xAD: op_xor_a_r(); break;         //
        case 0xAF: op_xor_a_r(); break;         //
        case 0xAE: op_xor_a_ind_hl(); break;    // xor (hl)
        case 0xEE: op_xor_a_imm(); break;       // xor #n

        case 0xB8: op_cp_r(); break;            // CP r
        case 0xB9: op_cp_r(); break;            //
        case 0xBA: op_cp_r(); break;            //
        case 0xBB: op_cp_r(); break;            //
        case 0xBC: op_cp_r(); break;            //
        case 0xBD: op_cp_r(); break;            //
        case 0xBF: op_cp_r(); break;            //
        case 0xBE: op_cp_ind_hl(); break;       // CP (hl)
        case 0xFE: op_cp_imm(); break;          // CP #n

        case 0x04: op_inc8_r(); break;          // inc r
        case 0x0C: op_inc8_r(); break;          //
        case 0x14: op_inc8_r(); break;          //
        case 0x1C: op_inc8_r(); break;          //
        case 0x24: op_inc8_r(); break;          //
        case 0x2C: op_inc8_r(); break;          //
        case 0x3C: op_inc8_r(); break;          //
        case 0x34: op_inc8_ind_hl(); break;     // inc (hl)

        case 0x05: op_dec8_r(); break;          // dec r
        case 0x0D: op_dec8_r(); break;          //
        case 0x15: op_dec8_r(); break;          //
        case 0x1D: op_dec8_r(); break;          //
        case 0x25: op_dec8_r(); break;          //
        case 0x2D: op_dec8_r(); break;          //
        case 0x3D: op_dec8_r(); break;          //
        case 0x35: op_dec8_ind_hl(); break;     // dec (hl)

        case 0x09: op_add16_hl_r(); break;      // add hl, rr
        case 0x19: op_add16_hl_r(); break;
        case 0x29: op_add16_hl_r(); break;
        case 0x39: op_add16_hl_r(); break;

        case 0xE8: op_add16_sp_imm(); break;    // add sp, #nn

        case 0x03: op_inc16_r(); break;         // inc rr
        case 0x13: op_inc16_r(); break;
        case 0x23: op_inc16_r(); break;
        case 0x33: op_inc16_r(); break;

        case 0x0B: op_dec16_r(); break;         // dec rr
        case 0x1B: op_dec16_r(); break;
        case 0x2B: op_dec16_r(); break;
        case 0x3B: op_dec16_r(); break;

        case 0x27: op_daa(); break;             // daa
        case 0x2F: op_cpl(); break;             // cpl
        case 0x3F: op_ccf(); break;             // ccf
        case 0x37: op_scf(); break;             // scf

        case 0x00: op_nop(); break;             // nop
        case 0x76: op_halt(); break;            // halt
        case 0x10: op_stop(); break;            // stop

        case 0xF3: op_di(); break;              // di
        case 0xFB: op_ei(); break;              // ei

        case 0x07: op_rlca(); break;            // rlca
        case 0x17: op_rla(); break;             // rla
        case 0x0F: op_rrca(); break;            // rrca
        case 0x1F: op_rra(); break;             // rra

        case 0xC3: op_jp(); break;              // jp nn

        case 0xC2: op_jp_cc(); break;           // jp cc, nn
        case 0xCA: op_jp_cc(); break;           //
        case 0xD2: op_jp_cc(); break;           //
        case 0xDA: op_jp_cc(); break;           //

        case 0xE9: op_jp_ind_hl(); break;       // jp (hl)

        case 0x18: op_jr(); break;              // jr n

        case 0x20: op_jr_cc(); break;           // jr cc, nn
        case 0x28: op_jr_cc(); break;           //
        case 0x30: op_jr_cc(); break;           //
        case 0x38: op_jr_cc(); break;           //

        case 0xCD: op_call(); break;            // call

        case 0xC4: op_call_cc(); break;         // call cc, nn
        case 0xCC: op_call_cc(); break;         //
        case 0xD4: op_call_cc(); break;         //
        case 0xDC: op_call_cc(); break;         //

        case 0xC7: op_rst(); break;             // rst 0x00
        case 0xCF: op_rst(); break;             // rst 0x08
        case 0xD7: op_rst(); break;             // rst 0x10
        case 0xDF: op_rst(); break;             // rst 0x18
        case 0xE7: op_rst(); break;             // rst 0x20
        case 0xEF: op_rst(); break;             // rst 0x28
        case 0xF7: op_rst(); break;             // rst 0x30
        case 0xFF: op_rst(); break;             // rst 0x38

        case 0xC9: op_ret(); break;             // ret

        case 0xC0: op_ret_cc(); break;          // ret cc, nn
        case 0xC8: op_ret_cc(); break;          //
        case 0xD0: op_ret_cc(); break;          //
        case 0xD8: op_ret_cc(); break;          //

        case 0xD9: op_reti(); break;            // reti

        case 0xCB: cpu_ext_op(); break;         // ext op 0xCB

        default:
            printf("!!! ERROR: Unhandled opcode: %02x !!\n", op);
            cycles(4);
        }

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

}

void cpu_ext_op() {
    ipc(1);
    op = cpu_read8(*pc);

    // BIT
    if ((op >= 0x40) && (op <= 0x7F)) {
        if ((op & 0x07) == 0x06) {
            op_bit_ind_hl();
        } else {
            op_bit_r();
        }
        return;
    }

    // RES
    if ((op >= 0x80) && (op <= 0xBF)) {
        if ((op & 0x07) == 0x06) {
            op_res_ind_hl();
        } else {
            op_res_r();
        }
        return;
    }

    // SET
    if (op >= 0xC0) {
        if ((op & 0x07) == 0x06) {
            op_set_ind_hl();
        } else {
            op_set_r();
        }
        return;
    }

    switch (op) {

    case 0x30: op_swap_r(); break;          // swap r
    case 0x31: op_swap_r(); break;
    case 0x32: op_swap_r(); break;
    case 0x33: op_swap_r(); break;
    case 0x34: op_swap_r(); break;
    case 0x35: op_swap_r(); break;
    case 0x37: op_swap_r(); break;
    case 0x36: op_swap_ind_hl(); break;     // swap (hl)

    case 0x00: op_rlc_r(); break;           // rlc r
    case 0x01: op_rlc_r(); break;
    case 0x02: op_rlc_r(); break;
    case 0x03: op_rlc_r(); break;
    case 0x04: op_rlc_r(); break;
    case 0x05: op_rlc_r(); break;
    case 0x07: op_rlc_r(); break;
    case 0x06: op_rlc_ind_hl(); break;      // rlc (hl)

    case 0x10: op_rl_r(); break;            // rl r
    case 0x11: op_rl_r(); break;
    case 0x12: op_rl_r(); break;
    case 0x13: op_rl_r(); break;
    case 0x14: op_rl_r(); break;
    case 0x15: op_rl_r(); break;
    case 0x17: op_rl_r(); break;
    case 0x16: op_rl_ind_hl(); break;       // rl (hl)

    case 0x08: op_rrc_r(); break;           // rrc r
    case 0x09: op_rrc_r(); break;
    case 0x0A: op_rrc_r(); break;
    case 0x0B: op_rrc_r(); break;
    case 0x0C: op_rrc_r(); break;
    case 0x0D: op_rrc_r(); break;
    case 0x0F: op_rrc_r(); break;
    case 0x0E: op_rrc_ind_hl(); break;      // rrc (hl)

    case 0x18: op_rr_r(); break;            // rr r
    case 0x19: op_rr_r(); break;
    case 0x1A: op_rr_r(); break;
    case 0x1B: op_rr_r(); break;
    case 0x1C: op_rr_r(); break;
    case 0x1D: op_rr_r(); break;
    case 0x1F: op_rr_r(); break;
    case 0x1E: op_rr_ind_hl(); break;       // rr (hl)

    case 0x20: op_sla_r(); break;           // sla r
    case 0x21: op_sla_r(); break;
    case 0x22: op_sla_r(); break;
    case 0x23: op_sla_r(); break;
    case 0x24: op_sla_r(); break;
    case 0x25: op_sla_r(); break;
    case 0x27: op_sla_r(); break;
    case 0x26: op_sla_ind_hl(); break;      // sla (hl)

    case 0x28: op_sra_r(); break;           // sra r
    case 0x29: op_sra_r(); break;
    case 0x2A: op_sra_r(); break;
    case 0x2B: op_sra_r(); break;
    case 0x2C: op_sra_r(); break;
    case 0x2D: op_sra_r(); break;
    case 0x2F: op_sra_r(); break;
    case 0x2E: op_sra_ind_hl(); break;      // sra (hl)

    case 0x38: op_srl_r(); break;           // srl r
    case 0x39: op_srl_r(); break;
    case 0x3A: op_srl_r(); break;
    case 0x3B: op_srl_r(); break;
    case 0x3C: op_srl_r(); break;
    case 0x3D: op_srl_r(); break;
    case 0x3F: op_srl_r(); break;
    case 0x3E: op_srl_ind_hl(); break;      // srl (hl)

    }

}

void cycles(uint16_t n) {
    // Nothing here yet. Maybe soon! :)
    sys_dma_cycles(n);
    sys_cycles(n);
    video_cycles(n);
    return;
}

uint16_t process_interrupts() {

    if (cpu_ie || cpu_halted) {
        for (uint16_t i = 0; i < 5; i++) {
            if ((ram_ie & SYS_IF) & (1 << i)) {
                // if an int is enabled and pending, service it

                trace(TRACE_INT, "Servicing interrupt index %i\n", i);

                // step 1: disable interrupt master enable
                // also clear bit in IF register

                cpu_ei_pending = 0;
                cpu_ie = 0;
                sys_interrupt_clear(1 << i);


                // step 2: push pc onto stack

                *sp = bs(bs(*sp) - 2);
                cpu_write16(bs(*sp), *pc);

                // step 3: new PC is 0x40 + int index

                *pc = 0x40 + (i << 3);

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
