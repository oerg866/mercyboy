#include "cpu.h"

inline int op_get_operand8_a(uint8_t byte, int bitshift) {
    // gets an operand, a is possible
    int dst = ((byte >> bitshift) & 0x07) + 2;
    if (dst == 0x09) dst = REG_A;
    return dst;
}

inline uint8_t op_get_cc(uint8_t byte) {
    return ((byte >> 3) & 0x03);
}

void op_ld_r_imm() {
    // ld r, #n
    //  load immediate into register
    int dst = op_get_operand8_a(op, 3);
    regs8[dst] = cpu_read8(*pc+1);
    ipc(2); cycles(8);
}

void op_ld_ind_hl_imm() {
    // ld (hl), #n
    cpu_write8(bs(*hl), cpu_read8(*pc+1));
    ipc(2); cycles(12);
}


void op_ld_r_r() {
    // get source reg

    int src = op_get_operand8_a(op, 0);
    int dst = op_get_operand8_a(op, 3);

    regs8[dst] = regs8[src];
    ipc(1); cycles(4);
}


void op_ld_a_r() {
    int src = (op & 0x07) + 2;
    regs8[REG_A] = regs8[src];
    ipc(1); cycles(4);
}
void op_ld_same() {
    // ld r1, r1 ... effectively a nop
    ipc(1); cycles(4);
}

void op_ld_r_ind_hl() {
    // ld n, (hl)
    int dst = ((op >> 3) & 0x07) + 2;
    regs8[dst] = cpu_read8(bs(*hl));
    ipc(1); cycles(8);
}


void op_ld_ind_bc_a() {
    // ld (bc), a
    cpu_write8(bs(*bc), regs8[REG_A]);
    ipc(1); cycles(8);
}

void op_ld_ind_de_a() {
    // ld (de), a
    cpu_write8(bs(*de), regs8[REG_A]);
    ipc(1); cycles(8);
}


void op_ld_ind_hl_a() {
    // ld (hl), a
    cpu_write8(bs(*hl), regs8[REG_A]);
    ipc(1); cycles(8);
}


void op_ld_ind_mem_a() {
    // ld (nn), a
    cpu_write8(cpu_read16(*pc+1), regs8[REG_A]);
    ipc(3); cycles(16);
}

void op_ld_a_ind_bc() {
    // ld a, (bc)
    regs8[REG_A] = cpu_read8(bs(*bc));
    ipc(1); cycles(8);
}

void op_ld_a_ind_de() {
    // ld a, (de)
    regs8[REG_A] = cpu_read8(bs(*de));
    ipc(1); cycles(8);
}

void op_ld_a_ind_hl() {
    // ld a, (hl)
    regs8[REG_A] = cpu_read8(bs(*hl));
    ipc(1); cycles(8);
}

void op_ld_a_ind_mem() {
    // ld a, (nn)
    regs8[REG_A] = cpu_read8(cpu_read16(*pc+1));
    ipc(3); cycles(16);
}

void op_ld_ind_hl_r() {
    // ld (hl), r
//    int src = (op & 0x07) + 2;
    int src = op_get_operand8_a(op, 0);
    cpu_write8(bs(*hl), regs8[src]);
    ipc(1); cycles(8);
}

void op_ld_a_c() {
    // ld a, ($FF00+C)
    regs8[REG_A] = cpu_read8(0xFF00+regs8[REG_C]);
    ipc(1); cycles(8);
}

void op_ld_c_a() {
    // ld ($FF00+C), a
    cpu_write8(0xFF00+regs8[REG_C], regs8[REG_A]);
    ipc(1); cycles(8);
}

void op_ldd_a_ind_hl() {
    // ld a, (hl-)
    regs8[REG_A] = cpu_read8(bs(*hl)); *hl = bs(bs(*hl) - 1);
    ipc(1); cycles(8);
}

void op_ldd_ind_hl_a() {
    // ld (hl-), a
    cpu_write8(bs(*hl), regs8[REG_A]); *hl = bs(bs(*hl) - 1);
    ipc(1); cycles(8);
}

void op_ldi_a_ind_hl() {
    // ld a, (hl+)
    regs8[REG_A] = cpu_read8(bs(*hl)); *hl = bs(bs(*hl) + 1);
    ipc(1); cycles(8);
}

void op_ldi_ind_hl_a() {
    // ld a, (hl+)
    cpu_write8(bs(*hl), regs8[REG_A]); *hl = bs(bs(*hl) + 1);
    ipc(1); cycles(8);
}

void op_ldh_n_a() {
    // ld ($FF00+n), a
    cpu_write8(0xFF00+cpu_read8(*pc+1), regs8[REG_A]);
    ipc(2); cycles(12);
}

void op_ldh_a_n() {
    // ld a, ($FF00+n)
    regs8[REG_A] = cpu_read8(0xFF00+cpu_read8(*pc+1));
    ipc(2); cycles(12);
}

void op_ld_r_imm16() {
    // ld r, ##
    int dst = (op >> 4)+1;
    sr16(dst, bs(cpu_read16(*pc+1)));
    ipc(3); cycles(12);
}

void op_ld_sp_hl() {
    // ld sp, hl
    *sp = *hl;
    ipc(1); cycles(8);
}

void op_ldhl_sp_imm() {
    // ld hl, sp+n SIGNED!!
    int newhl;
    newhl = bs(*sp) + (int8_t) (cpu_read8(*pc+1));
    newhl = bs(newhl);
    ipc(2); cycles(12);
}

void op_ld_ind_imm_sp() {
    // ld (nn), sp
    cpu_write16(cpu_read16(*pc+1), bs(*sp));
    ipc(3); cycles(20);
}

void op_push16() {
    // push af, bc, de, hl
    int dst = ((op >> 4) & 0x03) + 1;
    if (dst == 0x04) dst = REG_AF;
    *sp = bs(bs(*sp) - 2);
    cpu_write16(bs(*sp), bs(regs16[dst]));
    ipc(1); cycles(16);
}

void op_pop16() {
    // pop af, bc, de, hl
    int dst = ((op >> 4) & 0x03) + 1;
    if (dst == 0x04) dst = REG_AF;
    regs16[dst] = bs(cpu_read16(bs(*sp)));
    *sp = bs(bs(*sp) + 2);
    ipc(1); cycles(12);
}


/*
 *  GENERAL ADD INSTRUCTIONS
 */

void op_add8_a_r() {
    // add a, r
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_add8(&regs8[REG_A], regs8[src]);
    ipc(1); cycles(4);
}

void op_add8_a_ind_hl() {
    // add a, (hl)
    alu_add8(&regs8[REG_A], cpu_read8(bs(*hl)));
    ipc(1); cycles(8);
}

void op_add8_a_imm() {
    // add a, #n
    alu_add8(&regs8[REG_A], cpu_read8(*pc+1));
    ipc(2); cycles(8);
}

void op_add16_hl_r() {
    // add hl, r
    int dst = ((op >> 4) & 0x03) + 1;
    uint16_t reg = bs(*hl);
    alu_add16(&reg, bs(regs16[dst]));
    *hl = bs(reg);
    ipc(1); cycles(8);
}

void op_add16_sp_imm() {
    // add sp, #n
    uint16_t reg = bs(*sp);
    alu_add16(&reg, bs(cpu_read16(*pc+1)));
    *sp = bs(reg);
    *flags &= (FLAG_H | FLAG_C);
    ipc(3); cycles(16);
}



/*
 *  GENERAL ADC INSTRUCTIONS
 */


void op_adc_a_r() {
    // adc a, r
    uint16_t c = (regs8[REG_F] & FLAG_C) >> 4;
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_add8(&regs8[REG_A], regs8[src] + c);
    ipc(1); cycles(4);
}

void op_adc_a_ind_hl() {
    // adc a, (hl)
    uint16_t c = (regs8[REG_F] & FLAG_C) >> 4;
    alu_add8(&regs8[REG_A], cpu_read8(bs(*hl)) + c);
    ipc(1); cycles(8);
}

void op_adc_a_imm() {
    // adc a, #n
    uint16_t c = (regs8[REG_F] & FLAG_C) >> 4;
    alu_add8(&regs8[REG_A], cpu_read8(*pc+1) + c);
    ipc(2); cycles(8);
}



/*
 *  GENERAL SUB INSTRUCTIONS
 */

void op_sub_a_r() {
    // sub a, r
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_sub8(&regs8[REG_A], regs8[src]);
    ipc(1); cycles(4);
}

void op_sub_a_ind_hl() {
    // sub a, (hl)
    alu_sub8(&regs8[REG_A], cpu_read8(bs(*hl)));
    ipc(1); cycles(8);
}

void op_sub_a_imm() {
    // sub a, #n
    alu_sub8(&regs8[REG_A], cpu_read8(*pc+1));
    ipc(2); cycles(8);
}


/*
 *  GENERAL SBC INSTRUCTIONS
 */


void op_sbc_a_r() {
    // sub a, r
    uint16_t c = (regs8[REG_F] & FLAG_C) >> 4;
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_sub8(&regs8[REG_A], regs8[src] + c);
    ipc(1); cycles(4);
}

void op_sbc_a_ind_hl() {
    // sub a, (hl)
    uint16_t c = (regs8[REG_F] & FLAG_C) >> 4;
    alu_sub8(&regs8[REG_A], cpu_read8(bs(*hl)) + c);
    ipc(1); cycles(8);
}

void op_sbc_a_imm() {
    // sub a, #n
    uint16_t c = (regs8[REG_F] & FLAG_C) >> 4;
    alu_sub8(&regs8[REG_A], cpu_read8(*pc+1) + c);
    ipc(2); cycles(8);
}


/*
 * LOGIC: AND
 */

void op_and_a_r() {
    // and a, r
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_and(&regs8[REG_A], regs8[src]);
    ipc(1); cycles(4);
}

void op_and_a_ind_hl() {
    // and a, (hl)
    alu_and(&regs8[REG_A], cpu_read8(bs(*hl)));
    ipc(1); cycles(8);
}

void op_and_a_imm() {
    // and a, #n
    alu_and(&regs8[REG_A], cpu_read8(*pc+1));
    ipc(2); cycles(8);
}



/*
 * LOGIC: OR
 */


void op_or_a_r() {
    // or a, r
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_or(&regs8[REG_A], regs8[src]);
    ipc(1); cycles(4);
}

void op_or_a_ind_hl() {
    // or a, (hl)
    alu_or(&regs8[REG_A], cpu_read8(bs(*hl)));
    ipc(1); cycles(8);
}

void op_or_a_imm() {
    // or a, #n
    alu_or(&regs8[REG_A], cpu_read8(*pc+1));
    ipc(2); cycles(8);
}


/*
 * LOGIC: XOR
 */

void op_xor_a_r() {
    // xor a, r
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    alu_xor(&regs8[REG_A], regs8[src]);
    ipc(1); cycles(4);
}

void op_xor_a_ind_hl() {
    // xor a, (hl)
    alu_xor(&regs8[REG_A], cpu_read8(bs(*hl)));
    ipc(1); cycles(8);
}

void op_xor_a_imm() {
    // xor a, #n
    alu_xor(&regs8[REG_A], cpu_read8(*pc+1));
    ipc(2); cycles(8);
}

/*
 *  COMPARATOR = sub without result
 */

void op_cp_r() {
    // cp r
    int src = (op & 0x07) + 2;
    if (src == 0x09) src = REG_A;
    uint8_t tmp = regs8[REG_A];
    alu_sub8(&tmp, src);
    ipc(1); cycles(4);
}

void op_cp_ind_hl() {
    // cp (hl)
    uint8_t tmp = regs8[REG_A];
    alu_sub8(&tmp, cpu_read8(bs(*hl)));
    ipc(1); cycles(8);
}

void op_cp_imm() {
    // cp #n
    uint8_t tmp = regs8[REG_A];
    alu_sub8(&tmp, cpu_read8(*pc+1));
    ipc(2); cycles(8);
}


/*
 *  INCrement
 */

void op_inc8_r() {
    // inc r

    int dst = ((op >> 3) & 0x07) + 2;
    if (dst == 0x09) dst = REG_A;
    alu_inc8(&regs8[dst]);
    ipc(1); cycles(4);
}

void op_inc8_ind_hl() {
    // inc(hl)
    alu_inc8(mem_addr(bs(*hl)));
    ipc(1); cycles(12);
}

void op_inc16_r() {
    // inc r
    int dst = ((op >> 4) & 0x03) + 1;
    alu_inc16(&regs16[dst]);
    ipc(1); cycles(8);
}

/*
 *  DECrement
 */

void op_dec8_r() {
    // dec r
    int dst = ((op >> 3) & 0x07) + 2;
    if (dst == 0x09) dst = REG_A;
    alu_dec8(&regs8[dst]);
    ipc(1); cycles(4);
}

void op_dec8_ind_hl() {
    // dec (hl)
    alu_dec8(mem_addr(bs(*hl)));
    ipc(1); cycles(12);
}

void op_dec16_r() {
    // inc r
    int dst = ((op >> 4) & 0x03) + 1;
    alu_dec16(&regs16[dst]);
    ipc(1); cycles(8);
}


/*
 *  SWAP
 */

void op_swap_r() {
    // swap r
    int dst = (op & 0x07) + 2;
    if (dst == 0x09) dst = REG_A;
    alu_swap(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_swap_ind_hl(){
    // swap (hl)
    alu_swap(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}


/*
 *  DAA
 */

void op_daa() {
    // daa
    uint8_t n, c, h, z;
    n = *flags & FLAG_N;
    c = *flags & FLAG_C;
    h = *flags & FLAG_H;
    z = *flags & FLAG_Z;

    if (!n) {
        // after addition, adjust if half carry or carry occured
        // or if result is out of bounds
        if (c || (regs8[REG_A] > 0x99)) {
            regs8[REG_A] += 0x60; c = FLAG_C;
        }
        if (h || ((regs8[REG_A] & 0x0F) > 0x09)) {
            regs8[REG_A] += 0x06;
        }
    } else {
        // after subtraction, only adjust if half carry or carry occured
        if (c) {
            regs8[REG_A] -= 0x60;
        }
        if (h) {
            regs8[REG_A] -= 0x06;
        }
    }

    z = 0;
    if (regs8[REG_A] == 0) {
        z = FLAG_Z;
    }

    h = 0;

    alu_flags(z, n, h, c);

    ipc(1); cycles(4);
}

/*
 *  Miscellaneous flag ops
 */

void op_cpl() {
    // CPL complement a
    regs8[REG_A] = ~regs8[REG_A];
    *flags |= FLAG_N;
    *flags |= FLAG_H;
    ipc(1); cycles(4);
}

void op_ccf() {
    // ccf complement carry
    uint8_t c = *flags & FLAG_C;
    *flags &= FLAG_Z; // reset everything but z and c
    c = ~c; c &= FLAG_C;    // invert c
    *flags |= c;
    ipc(1); cycles(4);
}

void op_scf() {
    // scf set carry
    *flags = (*flags & FLAG_Z) | FLAG_C;
    ipc(1); cycles(4);
}


void op_nop() {
    // nop
    ipc(1); cycles(4);
}

void op_halt() {
    // halt until interrupt
    // TODO: Emulate bugs!!!
//    printf("halting...\n");
    int ints = 0;
    ipc(1);
    cycles(4);
    while (ints == 0) {
        ints = process_interrupts();
        cycles(4);
    }
}

void op_stop() {
    // halt until button pressed
//    printf("stopping...\n");
    uint8_t buttons_old = sys_buttons_all;
    while (sys_buttons_all == buttons_old) {
        sys_handle_joypads();
        cycles(4);
    }
    ipc(2); cycles(4);
}

/*
 *  ROTATES
 */

void op_rla() {
    // rla
    alu_rl(&regs8[REG_A]);
    ipc(1); cycles(4);
}

void op_rl_r() {
    // rl r
    // read destination reg
    int dst = op_get_operand8_a(op, 0);
    alu_rl(&regs8[dst]);
    ipc(1); cycles(16);
}

void op_rl_ind_hl() {
    // rl (hl)
    alu_rl(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}

void op_rlca() {
    // rlca
    alu_rlc(&regs8[REG_A]);
    ipc(1); cycles(4);
}

void op_rlc_r() {
    // rlc r
    // read destination reg
    int dst = op_get_operand8_a(op, 0);
    alu_rlc(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_rlc_ind_hl() {
    // rlc (hl)
    alu_rlc(mem_addr(bs(*hl)));
    ipc(1); cycles(16);

}

void op_rra() {
    // rra
    alu_rr(&regs8[REG_A]);
    ipc(1); cycles(4);
}

void op_rr_r() {
    // rr r
    // read destination reg
    int dst = op_get_operand8_a(op, 0);
    alu_rr(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_rr_ind_hl() {
    // rr (hl)
    alu_rr(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}

void op_rrca() {
    // rrca
    alu_rrc(&regs8[REG_A]);
    ipc(1); cycles(4);
}

void op_rrc_r() {
    // rrc r
    int dst = op_get_operand8_a(op, 0);
    alu_rrc(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_rrc_ind_hl() {
    // rrc (hl)
    alu_rrc(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}

/*
 * SHIFTS
 */

void op_sla_r() {
    // sla r
    int dst = op_get_operand8_a(op, 0);
    alu_sl(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_sla_ind_hl() {
    // sla (hl)
    alu_sl(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}

void op_sra_r() {
    // sra r
    int dst = op_get_operand8_a(op, 0);
    alu_sra(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_sra_ind_hl() {
    // sra (hl)
    alu_sra(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}

void op_srl_r() {
    // srl r
    int dst = op_get_operand8_a(op, 0);
    alu_srl(&regs8[dst]);
    ipc(1); cycles(8);
}

void op_srl_ind_hl() {
    // srl (hl)
    alu_srl(mem_addr(bs(*hl)));
    ipc(1); cycles(16);
}

/*
 * BIT OPS
 */

void op_bit_r() {
    // bit b, r
    int dst = op_get_operand8_a(op, 0);
    int bit = (op >> 3) & 0x07;
    alu_bit(&regs8[dst], bit);
    ipc(1); cycles(8);
}

void op_bit_ind_hl() {
    // bit b, (hl)
    int bit = (op >> 3) & 0x07;
    alu_bit(mem_addr(bs(*hl)), bit);
    ipc(1); cycles(16);
}

void op_set_r() {
    // set b, r
    int dst = op_get_operand8_a(op, 0);
    int bit = (op >> 3) & 0x07;
    alu_set(&regs8[dst], bit);
    ipc(1); cycles(8);
}

void op_set_ind_hl() {
    // set b, (hl)

    int bit = (op >> 3) & 0x07;
    alu_set(mem_addr(bs(*hl)), bit);
    ipc(1); cycles(16);
}

void op_res_r() {
    // res b, r
    int dst = op_get_operand8_a(op, 0);
    int bit = (op >> 3) & 0x07;
    alu_res(&regs8[dst], bit);
    ipc(1); cycles(8);
}

void op_res_ind_hl() {
    // res b, (hl)
    int bit = (op >> 3) & 0x07;
    alu_res(mem_addr(bs(*hl)), bit);
    ipc(1); cycles(16);
}

/*
 * JUMPS
 */

void op_jp() {
    // jp nn
    *pc = cpu_read16(*pc+1);
    cycles(12);
}

void op_jp_cc(){
    // jp cc, nn
    uint16_t old_pc = *pc;
    switch (op_get_cc(op)) {
    case COND_NZ:
        if (!(*flags & FLAG_Z)) *pc = cpu_read16(*pc+1); break;
    case COND_Z:
        if (*flags & FLAG_Z) *pc = cpu_read16(*pc+1); break;
    case COND_NC:
        if (!(*flags & FLAG_C)) *pc = cpu_read16(*pc+1); break;
    case COND_C:
        if (*flags & FLAG_C) *pc = cpu_read16(*pc+1); break;
    }

    if (old_pc == *pc) ipc(3);
    cycles(12);
}

void op_jp_ind_hl() {
    // jp (hl)
    *pc = bs(*hl);
    cycles(4);
}

void op_jr() {
    // jr n
    *pc = *pc + (int8_t) cpu_read8(*pc+1) + 2;
    cycles(8);
}

void op_jr_cc(){
    // jr cc, nn
    uint16_t old_pc = *pc;
    int8_t offset = (int8_t) cpu_read8(*pc+1)+2;
    switch (op_get_cc(op)) {
    case COND_NZ:
        if (!(*flags & FLAG_Z)) *pc += offset; break;
    case COND_Z:
        if (*flags & FLAG_Z) *pc += offset; break;
    case COND_NC:
        if (!(*flags & FLAG_C)) *pc += offset; break;
    case COND_C:
        if (*flags & FLAG_C) *pc += offset; break;
    }

    if (old_pc == *pc) ipc(2);
    cycles(8);
}

void op_call() {
    // call nn
    *sp = bs(bs(*sp) - 2);
    cpu_write16(bs(*sp), *pc+3); // save address to stack
    *pc = cpu_read16(*pc+1);
    cycles(12);
}


void op_call_cc(){
    // call cc, nn
    uint16_t old_pc = *pc;
    switch (op_get_cc(op)) {
    case COND_NZ:
        if (!(*flags & FLAG_Z)) *pc = cpu_read16(*pc+1); break;
    case COND_Z:
        if (*flags & FLAG_Z) *pc = cpu_read16(*pc+1); break;
    case COND_NC:
        if (!(*flags & FLAG_C)) *pc = cpu_read16(*pc+1); break;
    case COND_C:
        if (*flags & FLAG_C) *pc = cpu_read16(*pc+1); break;
    }

    if (old_pc == *pc) {
        ipc(3);
    } else {
        *sp = bs(bs(*sp) - 2);
        cpu_write16(bs(*sp), old_pc+3);
    }
    cycles(12);
}

void op_rst() {
    // rst n
    *sp = bs(bs(*sp) - 2);
    cpu_write16(bs(*sp), *pc+1);  // write current pc onto stack
    *pc = (op & 0x38); // read jump vector
    cycles(32);
}

void op_ret() {
    // ret
    *pc = cpu_read16(bs(*sp));
    *sp = bs(bs(*sp) + 2);
    cycles(8);
}

void op_ret_cc() {
    // ret cc
    uint16_t old_pc = *pc;
    switch (op_get_cc(op)) {
    case COND_NZ:
        if (!(*flags & FLAG_Z)) *pc = cpu_read16(bs(*sp)); break;
    case COND_Z:
        if (*flags & FLAG_Z) *pc = cpu_read16(bs(*sp)); break;
    case COND_NC:
        if (!(*flags & FLAG_C)) *pc = cpu_read16(bs(*sp)); break;
    case COND_C:
        if (*flags & FLAG_C) *pc = cpu_read16(bs(*sp)); break;
    }

    if (old_pc == *pc) {
        ipc(1);
    } else {
        *sp = bs(bs(*sp) + 2);
    }
    cycles(8);
}

void op_reti() {
    // reti return and enable interupt
    *pc = cpu_read16(bs(*sp));
    *sp = bs(bs(*sp) + 2);
    // schedule enabling of interrupts
    cpu_ie = 1;

    cycles(8);
}

void op_di() {
    // DI
    cpu_di_pending = 1;
    ipc(1); cycles(4);
}

void op_ei() {
    // EI
    cpu_ei_pending = 1;
    ipc(1); cycles(4);
}


