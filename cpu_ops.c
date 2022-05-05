#include "cpu.h"
#include "cpu_help.h"
#include "cpu_alu.h"

#include "sys.h"
#include "video.h"
#include "console.h"
#include "trace.h"
#include "mem.h"

#include "compat.h"



static inline void cpu_reset_idle_cycles_max()
/* Re-calculates the amount of time the CPU can run before having to process system / video cycles. */
{
    uint32_t video_idle_cycles = video_get_idle_cycle_count();
    uint32_t sys_idle_cycles = sys_get_idle_cycle_count();
    cpu_idle_cycles_max = MIN(sys_idle_cycles, video_idle_cycles);
}

static inline void cycles(uint32_t n)
/* Increments "idle" cycle counter by n amount, and if we're done idling we
   take a leap and process them all at once. */
{
    cpu_idle_cycles += n;
    if (cpu_idle_cycles >= cpu_idle_cycles_max) {
        sys_cycles_idle(cpu_idle_cycles);
        video_cycles(cpu_idle_cycles);
        cpu_idle_cycles -= cpu_idle_cycles_max;
        cpu_reset_idle_cycles_max();
    }
}

// Helper macros to check pending interrupt enable / disable and processing actions.
// Put into macros because there are 3 instructions where this doesn't occur
// and we don't want to waste cycles on every instruction to check if this is
// one of those where it *doesn't* happen

// check for pending EI/DI, happens for everything EXCEPT 0xf3 (di) and 0xfb (ei)
#define ei_di() if (cpu_ei_pending) { cpu_ie = 1; cpu_ei_pending = 0; } \
           else if (cpu_di_pending) { cpu_ie = 0; cpu_di_pending = 0; }

static inline void service_ints() {
    // check for interrupts to be serviced and then... service them
    uint8_t ie_mask = (ram_ie & SYS_IF);

/**************************************************************/
// This macro is to unroll the loop that checks whether or not a particular interrupt should be serviced
#define check_and_service_ints(id) \
    if (ie_mask & (1 << id)) { \
        cpu_ei_pending = 0; \
        cpu_ie = 0; \
        sys_interrupt_clear(1 << id); \
        sp -= 2; \
        w16(sp, pc); \
        pc = 0x40 + (id << 3); \
        cycles(12); \
        return; \
    }


/**************************************************************/
    if (!ie_mask) return;

    check_and_service_ints(0);
    check_and_service_ints(1);
    check_and_service_ints(2);
    check_and_service_ints(3);
    check_and_service_ints(4);
}

#define ints() { if (cpu_ie) service_ints(); }

void op_nop() { ipc(1); cycles(4); ei_di(); ints(); }

#define op_ld(reg1, reg2) reg1 = reg2; ipc(1); cycles(4); ei_di(); ints()
void op_ld_a_b() { op_ld(a, b); }
void op_ld_a_c() { op_ld(a, c); }
void op_ld_a_d() { op_ld(a, d); }
void op_ld_a_e() { op_ld(a, e); }
void op_ld_a_h() { op_ld(a, h); }
void op_ld_a_l() { op_ld(a, l); }

void op_ld_b_a() { op_ld(b, a); }
void op_ld_b_c() { op_ld(b, c); }
void op_ld_b_d() { op_ld(b, d); }
void op_ld_b_e() { op_ld(b, e); }
void op_ld_b_h() { op_ld(b, h); }
void op_ld_b_l() { op_ld(b, l); }

void op_ld_c_a() { op_ld(c, a); }
void op_ld_c_b() { op_ld(c, b); }
void op_ld_c_d() { op_ld(c, d); }
void op_ld_c_e() { op_ld(c, e); }
void op_ld_c_h() { op_ld(c, h); }
void op_ld_c_l() { op_ld(c, l); }

void op_ld_d_a() { op_ld(d, a); }
void op_ld_d_b() { op_ld(d, b); }
void op_ld_d_c() { op_ld(d, c); }
void op_ld_d_e() { op_ld(d, e); }
void op_ld_d_h() { op_ld(d, h); }
void op_ld_d_l() { op_ld(d, l); }

void op_ld_e_a() { op_ld(e, a); }
void op_ld_e_b() { op_ld(e, b); }
void op_ld_e_c() { op_ld(e, c); }
void op_ld_e_d() { op_ld(e, d); }
void op_ld_e_h() { op_ld(e, h); }
void op_ld_e_l() { op_ld(e, l); }

void op_ld_h_a() { op_ld(h, a); }
void op_ld_h_b() { op_ld(h, b); }
void op_ld_h_c() { op_ld(h, c); }
void op_ld_h_d() { op_ld(h, d); }
void op_ld_h_e() { op_ld(h, e); }
void op_ld_h_l() { op_ld(h, l); }

void op_ld_l_a() { op_ld(l, a); }
void op_ld_l_b() { op_ld(l, b); }
void op_ld_l_c() { op_ld(l, c); }
void op_ld_l_d() { op_ld(l, d); }
void op_ld_l_e() { op_ld(l, e); }
void op_ld_l_h() { op_ld(l, h); }

#define op_ld_r_imm8(reg) reg = r8(pc+1); ipc(2); cycles(8); ei_di(); ints()
void op_ld_a_n() { op_ld_r_imm8(a); }   // ld r, #n
void op_ld_b_n() { op_ld_r_imm8(b); }
void op_ld_c_n() { op_ld_r_imm8(c); }
void op_ld_d_n() { op_ld_r_imm8(d); }
void op_ld_e_n() { op_ld_r_imm8(e); }
void op_ld_h_n() { op_ld_r_imm8(h); }
void op_ld_l_n() { op_ld_r_imm8(l); }

void op_ld_ind_hl_n() { w8(hl, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); } // ld (hl), #n

#define op_ld_ind_hl_r(reg) w8(hl, reg); ipc(1); cycles(8); ei_di(); ints()
void op_ld_ind_hl_a() { op_ld_ind_hl_r(a); } // ld (hl), r
void op_ld_ind_hl_b() { op_ld_ind_hl_r(b); }
void op_ld_ind_hl_c() { op_ld_ind_hl_r(c); }
void op_ld_ind_hl_d() { op_ld_ind_hl_r(d); }
void op_ld_ind_hl_e() { op_ld_ind_hl_r(e); }
void op_ld_ind_hl_h() { op_ld_ind_hl_r(h); }
void op_ld_ind_hl_l() { op_ld_ind_hl_r(l); }

#define op_ld_r_ind_hl(reg) reg = r8(hl); ipc(1); cycles(8); ei_di(); ints()
void op_ld_a_ind_hl() { op_ld_r_ind_hl(a); } // ld r, (hl)
void op_ld_b_ind_hl() { op_ld_r_ind_hl(b); }
void op_ld_c_ind_hl() { op_ld_r_ind_hl(c); }
void op_ld_d_ind_hl() { op_ld_r_ind_hl(d); }
void op_ld_e_ind_hl() { op_ld_r_ind_hl(e); }
void op_ld_h_ind_hl() { op_ld_r_ind_hl(h); }
void op_ld_l_ind_hl() { op_ld_r_ind_hl(l); }

void op_ld_ind_bc_a() { w8(bc, a); ipc(1); cycles(8); ei_di(); ints(); } // ld (rr), a
void op_ld_ind_de_a() { w8(de, a); ipc(1); cycles(8); }
// void op_ld_ind_hl_a() { w8(hl, a); ipc(1); cycles(8); ei_di(); ints(); }

void op_ld_a_ind_bc() { a = r8(bc); ipc(1); cycles(8); ei_di(); ints(); } // ld a, (rr)
void op_ld_a_ind_de() { a = r8(de); ipc(1); cycles(8); ei_di(); ints(); }
// void op_ld_a_ind_hl() { a = r8(bc); ipc(1); cycles(8); ei_di(); ints(); }

void op_ld_ind_mem_a() { w8(r16(pc+1), a); ipc(3); cycles(16); ei_di(); ints(); } // ld (nn), a
void op_ld_a_ind_mem() { a = r8(r16(pc+1)); ipc(3); cycles(16); ei_di(); ints(); } // ld a, (nn)

void op_ld_a_ind_ff00_c() { a = r8(0xff00+c); ipc(1); cycles(8); ei_di(); ints(); } // ld a, ($ff00+c)
void op_ld_ind_ff00_c_a() { w8(0xff00+c, a); ipc(1); cycles(8); ei_di(); ints(); } // ld ($ff00+c), a

void op_ldh_n_a() { w8(0xff00+r8(pc+1), a); ipc(2); cycles(12); ei_di(); ints(); } // ld ($ff00+n), a
void op_ldh_a_n() { a = r8(0xff00+r8(pc+1)); ipc(2); cycles(12); ei_di(); ints(); } // ld a, ($ff00+n)

void op_ldd_a_ind_hl() { a = r8(hl); hl -= 1; ipc(1); cycles(8); ei_di(); ints(); } // ld a, (hl-)
void op_ldd_ind_hl_a() { w8(hl, a); hl -= 1; ipc(1); cycles(8); ei_di(); ints(); } // ld (hl-), a
void op_ldi_a_ind_hl() { a = r8(hl); hl += 1; ipc(1); cycles(8); ei_di(); ints(); } // ld a, (hl+)
void op_ldi_ind_hl_a() { w8(hl, a); hl += 1; ipc(1); cycles(8); ei_di(); ints(); } // ld (hl+), a

#define op_ld_r16_imm16(reg) reg = r16(pc+1); ipc(3); cycles(12); ei_di(); ints()
void op_ld_bc_nn() { op_ld_r16_imm16(bc); } // ld (rr), #nn
void op_ld_de_nn() { op_ld_r16_imm16(de); }
void op_ld_hl_nn() { op_ld_r16_imm16(hl); }
void op_ld_sp_nn() { op_ld_r16_imm16(sp); }

void op_ld_sp_hl() { sp = hl; ipc(1); cycles(8); ei_di(); ints(); }
void op_ld_hl_sp_n() { // ld hl, sp+(signed)n
    uint16_t n = r8(pc+1);
    hl = sp;
    if (n & 0x80) n |= 0xff00; // Fix sign extension
    alu_add16_imm8(&hl, n);
    ipc(2); cycles(12); ei_di(); ints();
}

void op_ld_ind_nn_sp() { w16(r16(pc+1), sp); ipc(3); cycles(20); ei_di(); ints(); } // ld (nn), sp

#define op_push(reg) sp -= 2; w16(sp, reg); ipc(1); cycles(16); ei_di(); ints()
void op_push_af() { op_push(af); }
void op_push_bc() { op_push(bc); }
void op_push_de() { op_push(de); }
void op_push_hl() { op_push(hl); }

#define op_pop(reg) reg = r16(sp); sp += 2; ipc(1); cycles(12); ei_di(); ints()
void op_pop_af() { af = r16(sp) & 0xFFF0; sp += 2; ipc(1); cycles(12); ei_di(); ints(); } // Only upper bits of F respected
void op_pop_bc() { op_pop(bc); }
void op_pop_de() { op_pop(de); }
void op_pop_hl() { op_pop(hl); }


#define op_add8_a_r(reg) alu_add8(&a, reg); ipc(1); cycles(4); ei_di(); ints()
void op_add8_a_a() { op_add8_a_r(a); }
void op_add8_a_b() { op_add8_a_r(b); }
void op_add8_a_c() { op_add8_a_r(c); }
void op_add8_a_d() { op_add8_a_r(d); }
void op_add8_a_e() { op_add8_a_r(e); }
void op_add8_a_h() { op_add8_a_r(h); }
void op_add8_a_l() { op_add8_a_r(l); }

void op_add8_a_ind_hl() { alu_add8(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_add8_a_imm() { alu_add8(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_add16_hl_r(reg) alu_add16(&hl, reg); ipc(1); cycles(8); ei_di(); ints()
void op_add16_hl_bc() { op_add16_hl_r(bc); }
void op_add16_hl_de() { op_add16_hl_r(de); }
void op_add16_hl_hl() { op_add16_hl_r(hl); }
void op_add16_hl_sp() { op_add16_hl_r(sp); }

void op_add16_sp_imm() {    // add sp, #(signed)n
    uint16_t n = r8(pc+1);
    if (n & 0x80) n |= 0xff00; // Fix sign extension
    alu_add16_imm8(&sp, n);
    ipc(2); cycles(16); ei_di(); ints();
}

#define op_adc8_a_r(reg) alu_adc8(&a, (uint16_t) reg); ipc(1); cycles(4); ei_di(); ints()
void op_adc8_a_a() { op_adc8_a_r(a); }
void op_adc8_a_b() { op_adc8_a_r(b); }
void op_adc8_a_c() { op_adc8_a_r(c); }
void op_adc8_a_d() { op_adc8_a_r(d); }
void op_adc8_a_e() { op_adc8_a_r(e); }
void op_adc8_a_h() { op_adc8_a_r(h); }
void op_adc8_a_l() { op_adc8_a_r(l); }

void op_adc8_a_ind_hl() { alu_adc8(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_adc8_a_imm() { alu_adc8(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_sub8_a_r(reg) alu_sub8(&a, reg); ipc(1); cycles(4); ei_di(); ints()
void op_sub8_a_a() { op_sub8_a_r(a); }
void op_sub8_a_b() { op_sub8_a_r(b); }
void op_sub8_a_c() { op_sub8_a_r(c); }
void op_sub8_a_d() { op_sub8_a_r(d); }
void op_sub8_a_e() { op_sub8_a_r(e); }
void op_sub8_a_h() { op_sub8_a_r(h); }
void op_sub8_a_l() { op_sub8_a_r(l); }

void op_sub8_a_ind_hl() { alu_sub8(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_sub8_a_imm() { alu_sub8(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_sbc8_a_r(reg) alu_sbc8(&a, (uint16_t) reg); ipc(1); cycles(4); ei_di(); ints()
void op_sbc8_a_a() { op_sbc8_a_r(a); }
void op_sbc8_a_b() { op_sbc8_a_r(b); }
void op_sbc8_a_c() { op_sbc8_a_r(c); }
void op_sbc8_a_d() { op_sbc8_a_r(d); }
void op_sbc8_a_e() { op_sbc8_a_r(e); }
void op_sbc8_a_h() { op_sbc8_a_r(h); }
void op_sbc8_a_l() { op_sbc8_a_r(l); }

void op_sbc8_a_ind_hl() { alu_sbc8(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_sbc8_a_imm() { alu_sbc8(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_and_a_r(reg) alu_and(&a, reg); ipc(1); cycles(4); ei_di(); ints()
void op_and_a_a() { op_and_a_r(a); }
void op_and_a_b() { op_and_a_r(b); }
void op_and_a_c() { op_and_a_r(c); }
void op_and_a_d() { op_and_a_r(d); }
void op_and_a_e() { op_and_a_r(e); }
void op_and_a_h() { op_and_a_r(h); }
void op_and_a_l() { op_and_a_r(l); }

void op_and_a_ind_hl() { alu_and(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_and_a_imm() { alu_and(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_or_a_r(reg) alu_or(&a, reg); ipc(1); cycles(4); ei_di(); ints()
void op_or_a_a() { op_or_a_r(a); }
void op_or_a_b() { op_or_a_r(b); }
void op_or_a_c() { op_or_a_r(c); }
void op_or_a_d() { op_or_a_r(d); }
void op_or_a_e() { op_or_a_r(e); }
void op_or_a_h() { op_or_a_r(h); }
void op_or_a_l() { op_or_a_r(l); }

void op_or_a_ind_hl() { alu_or(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_or_a_imm() { alu_or(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_xor_a_r(reg) alu_xor(&a, reg); ipc(1); cycles(4); ei_di(); ints()
void op_xor_a_a() { op_xor_a_r(a); }
void op_xor_a_b() { op_xor_a_r(b); }
void op_xor_a_c() { op_xor_a_r(c); }
void op_xor_a_d() { op_xor_a_r(d); }
void op_xor_a_e() { op_xor_a_r(e); }
void op_xor_a_h() { op_xor_a_r(h); }
void op_xor_a_l() { op_xor_a_r(l); }

void op_xor_a_ind_hl() { alu_xor(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_xor_a_imm() { alu_xor(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_cp_a_r(reg) alu_cp(&a, reg); ipc(1); cycles(4); ei_di(); ints()
void op_cp_a_a() { op_cp_a_r(a); }
void op_cp_a_b() { op_cp_a_r(b); }
void op_cp_a_c() { op_cp_a_r(c); }
void op_cp_a_d() { op_cp_a_r(d); }
void op_cp_a_e() { op_cp_a_r(e); }
void op_cp_a_h() { op_cp_a_r(h); }
void op_cp_a_l() { op_cp_a_r(l); }

void op_cp_a_ind_hl() { alu_cp(&a, r8(hl)); ipc(1); cycles(8); ei_di(); ints(); }
void op_cp_a_imm() { alu_cp(&a, r8(pc+1)); ipc(2); cycles(8); ei_di(); ints(); }

#define op_inc8_r(reg) alu_inc8(&reg); ipc(1); cycles(4); ei_di(); ints()
void op_inc8_a() { op_inc8_r(a); }
void op_inc8_b() { op_inc8_r(b); }
void op_inc8_c() { op_inc8_r(c); }
void op_inc8_d() { op_inc8_r(d); }
void op_inc8_e() { op_inc8_r(e); }
void op_inc8_h() { op_inc8_r(h); }
void op_inc8_l() { op_inc8_r(l); }

void op_inc8_ind_hl() { alu_inc8(mem_addr(hl)); ipc(1); cycles(12); ei_di(); ints(); }

#define op_inc16_r(reg) alu_inc16(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_inc16_bc() { op_inc16_r(bc); }
void op_inc16_de() { op_inc16_r(de); }
void op_inc16_hl() { op_inc16_r(hl); }
void op_inc16_sp() { op_inc16_r(sp); }

#define op_dec8_r(reg) alu_dec8(&reg); ipc(1); cycles(4); ei_di(); ints()
void op_dec8_a() { op_dec8_r(a); }
void op_dec8_b() { op_dec8_r(b); }
void op_dec8_c() { op_dec8_r(c); }
void op_dec8_d() { op_dec8_r(d); }
void op_dec8_e() { op_dec8_r(e); }
void op_dec8_h() { op_dec8_r(h); }
void op_dec8_l() { op_dec8_r(l); }

void op_dec8_ind_hl() { alu_dec8(mem_addr(hl)); ipc(1); cycles(12); ei_di(); ints(); }

#define op_dec16_r(reg) alu_dec16(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_dec16_bc() { op_dec16_r(bc); }
void op_dec16_de() { op_dec16_r(de); }
void op_dec16_hl() { op_dec16_r(hl); }
void op_dec16_sp() { op_dec16_r(sp); }

#define op_swap_r(reg) alu_swap(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_swap_a() { op_swap_r(a); }
void op_swap_b() { op_swap_r(b); }
void op_swap_c() { op_swap_r(c); }
void op_swap_d() { op_swap_r(d); }
void op_swap_e() { op_swap_r(e); }
void op_swap_h() { op_swap_r(h); }
void op_swap_l() { op_swap_r(l); }

void op_swap_ind_hl() { alu_swap(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

/* Rotate instructions */

#define op_rl_r(reg) alu_rl(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_rl_a() { op_rl_r(a); }
void op_rl_b() { op_rl_r(b); }
void op_rl_c() { op_rl_r(c); }
void op_rl_d() { op_rl_r(d); }
void op_rl_e() { op_rl_r(e); }
void op_rl_h() { op_rl_r(h); }
void op_rl_l() { op_rl_r(l); }

void op_rla() { alu_rl(&a); f &= ~FLAG_Z; ipc(1); cycles(4); ei_di(); ints(); } // short ver. 4 cycles & resets z flag!!
void op_rl_ind_hl() { alu_rl(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

#define op_rlc_r(reg) alu_rlc(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_rlc_a() { op_rlc_r(a); }
void op_rlc_b() { op_rlc_r(b); }
void op_rlc_c() { op_rlc_r(c); }
void op_rlc_d() { op_rlc_r(d); }
void op_rlc_e() { op_rlc_r(e); }
void op_rlc_h() { op_rlc_r(h); }
void op_rlc_l() { op_rlc_r(l); }

void op_rlca() { alu_rlc(&a); f &= ~FLAG_Z; ipc(1); cycles(4); ei_di(); ints(); } // short ver. 4 cycles & resets z flag!!
void op_rlc_ind_hl() { alu_rlc(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

#define op_rr_r(reg) alu_rr(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_rr_a() { op_rr_r(a); }
void op_rr_b() { op_rr_r(b); }
void op_rr_c() { op_rr_r(c); }
void op_rr_d() { op_rr_r(d); }
void op_rr_e() { op_rr_r(e); }
void op_rr_h() { op_rr_r(h); }
void op_rr_l() { op_rr_r(l); }

void op_rra() { alu_rr(&a); f &= ~FLAG_Z; ipc(1); cycles(4); ei_di(); ints(); } // short ver. 4 cycles & resets z flag!!
void op_rr_ind_hl() { alu_rr(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

#define op_rrc_r(reg) alu_rrc(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_rrc_a() { op_rrc_r(a); }
void op_rrc_b() { op_rrc_r(b); }
void op_rrc_c() { op_rrc_r(c); }
void op_rrc_d() { op_rrc_r(d); }
void op_rrc_e() { op_rrc_r(e); }
void op_rrc_h() { op_rrc_r(h); }
void op_rrc_l() { op_rrc_r(l); }

void op_rrca() { alu_rrc(&a); f &= ~FLAG_Z; ipc(1); cycles(4); ei_di(); ints(); } // short ver. 4 cycles & resets z flag!!
void op_rrc_ind_hl() { alu_rrc(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

/* Shift instructions */

#define op_sla_r(reg) alu_sl(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_sla_a() { op_sla_r(a); }
void op_sla_b() { op_sla_r(b); }
void op_sla_c() { op_sla_r(c); }
void op_sla_d() { op_sla_r(d); }
void op_sla_e() { op_sla_r(e); }
void op_sla_h() { op_sla_r(h); }
void op_sla_l() { op_sla_r(l); }

void op_sla_ind_hl() { alu_sl(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

#define op_sra_r(reg) alu_sra(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_sra_a() { op_sra_r(a); }
void op_sra_b() { op_sra_r(b); }
void op_sra_c() { op_sra_r(c); }
void op_sra_d() { op_sra_r(d); }
void op_sra_e() { op_sra_r(e); }
void op_sra_h() { op_sra_r(h); }
void op_sra_l() { op_sra_r(l); }

void op_sra_ind_hl() { alu_sra(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

#define op_srl_r(reg) alu_srl(&reg); ipc(1); cycles(8); ei_di(); ints()
void op_srl_a() { op_srl_r(a); }
void op_srl_b() { op_srl_r(b); }
void op_srl_c() { op_srl_r(c); }
void op_srl_d() { op_srl_r(d); }
void op_srl_e() { op_srl_r(e); }
void op_srl_h() { op_srl_r(h); }
void op_srl_l() { op_srl_r(l); }

void op_srl_ind_hl() { alu_srl(mem_addr(hl)); ipc(1); cycles(16); ei_di(); ints(); }

/* Bit Operations */

#define op_bit_r(reg, bit) alu_bit(&reg, bit); ipc(1); cycles(8); ei_di(); ints()
void op_bit_0_a() { op_bit_r(a, 0); }
void op_bit_1_a() { op_bit_r(a, 1); }
void op_bit_2_a() { op_bit_r(a, 2); }
void op_bit_3_a() { op_bit_r(a, 3); }
void op_bit_4_a() { op_bit_r(a, 4); }
void op_bit_5_a() { op_bit_r(a, 5); }
void op_bit_6_a() { op_bit_r(a, 6); }
void op_bit_7_a() { op_bit_r(a, 7); }

void op_bit_0_b() { op_bit_r(b, 0); }
void op_bit_1_b() { op_bit_r(b, 1); }
void op_bit_2_b() { op_bit_r(b, 2); }
void op_bit_3_b() { op_bit_r(b, 3); }
void op_bit_4_b() { op_bit_r(b, 4); }
void op_bit_5_b() { op_bit_r(b, 5); }
void op_bit_6_b() { op_bit_r(b, 6); }
void op_bit_7_b() { op_bit_r(b, 7); }

void op_bit_0_c() { op_bit_r(c, 0); }
void op_bit_1_c() { op_bit_r(c, 1); }
void op_bit_2_c() { op_bit_r(c, 2); }
void op_bit_3_c() { op_bit_r(c, 3); }
void op_bit_4_c() { op_bit_r(c, 4); }
void op_bit_5_c() { op_bit_r(c, 5); }
void op_bit_6_c() { op_bit_r(c, 6); }
void op_bit_7_c() { op_bit_r(c, 7); }

void op_bit_0_d() { op_bit_r(d, 0); }
void op_bit_1_d() { op_bit_r(d, 1); }
void op_bit_2_d() { op_bit_r(d, 2); }
void op_bit_3_d() { op_bit_r(d, 3); }
void op_bit_4_d() { op_bit_r(d, 4); }
void op_bit_5_d() { op_bit_r(d, 5); }
void op_bit_6_d() { op_bit_r(d, 6); }
void op_bit_7_d() { op_bit_r(d, 7); }

void op_bit_0_e() { op_bit_r(e, 0); }
void op_bit_1_e() { op_bit_r(e, 1); }
void op_bit_2_e() { op_bit_r(e, 2); }
void op_bit_3_e() { op_bit_r(e, 3); }
void op_bit_4_e() { op_bit_r(e, 4); }
void op_bit_5_e() { op_bit_r(e, 5); }
void op_bit_6_e() { op_bit_r(e, 6); }
void op_bit_7_e() { op_bit_r(e, 7); }

void op_bit_0_h() { op_bit_r(h, 0); }
void op_bit_1_h() { op_bit_r(h, 1); }
void op_bit_2_h() { op_bit_r(h, 2); }
void op_bit_3_h() { op_bit_r(h, 3); }
void op_bit_4_h() { op_bit_r(h, 4); }
void op_bit_5_h() { op_bit_r(h, 5); }
void op_bit_6_h() { op_bit_r(h, 6); }
void op_bit_7_h() { op_bit_r(h, 7); }

void op_bit_0_l() { op_bit_r(l, 0); }
void op_bit_1_l() { op_bit_r(l, 1); }
void op_bit_2_l() { op_bit_r(l, 2); }
void op_bit_3_l() { op_bit_r(l, 3); }
void op_bit_4_l() { op_bit_r(l, 4); }
void op_bit_5_l() { op_bit_r(l, 5); }
void op_bit_6_l() { op_bit_r(l, 6); }
void op_bit_7_l() { op_bit_r(l, 7); }

#define op_bit_ind_hl(bit) alu_bit(mem_addr(hl), bit); ipc(1); cycles(16); ei_di(); ints()
void op_bit_0_ind_hl() { op_bit_ind_hl(0); }
void op_bit_1_ind_hl() { op_bit_ind_hl(1); }
void op_bit_2_ind_hl() { op_bit_ind_hl(2); }
void op_bit_3_ind_hl() { op_bit_ind_hl(3); }
void op_bit_4_ind_hl() { op_bit_ind_hl(4); }
void op_bit_5_ind_hl() { op_bit_ind_hl(5); }
void op_bit_6_ind_hl() { op_bit_ind_hl(6); }
void op_bit_7_ind_hl() { op_bit_ind_hl(7); }

/* Bit ops: Set ********************************************************************/

#define op_set_r(reg, bit) alu_set(&reg, bit); ipc(1); cycles(8); ei_di(); ints()
void op_set_0_a() { op_set_r(a, 0); }
void op_set_1_a() { op_set_r(a, 1); }
void op_set_2_a() { op_set_r(a, 2); }
void op_set_3_a() { op_set_r(a, 3); }
void op_set_4_a() { op_set_r(a, 4); }
void op_set_5_a() { op_set_r(a, 5); }
void op_set_6_a() { op_set_r(a, 6); }
void op_set_7_a() { op_set_r(a, 7); }

void op_set_0_b() { op_set_r(b, 0); }
void op_set_1_b() { op_set_r(b, 1); }
void op_set_2_b() { op_set_r(b, 2); }
void op_set_3_b() { op_set_r(b, 3); }
void op_set_4_b() { op_set_r(b, 4); }
void op_set_5_b() { op_set_r(b, 5); }
void op_set_6_b() { op_set_r(b, 6); }
void op_set_7_b() { op_set_r(b, 7); }

void op_set_0_c() { op_set_r(c, 0); }
void op_set_1_c() { op_set_r(c, 1); }
void op_set_2_c() { op_set_r(c, 2); }
void op_set_3_c() { op_set_r(c, 3); }
void op_set_4_c() { op_set_r(c, 4); }
void op_set_5_c() { op_set_r(c, 5); }
void op_set_6_c() { op_set_r(c, 6); }
void op_set_7_c() { op_set_r(c, 7); }

void op_set_0_d() { op_set_r(d, 0); }
void op_set_1_d() { op_set_r(d, 1); }
void op_set_2_d() { op_set_r(d, 2); }
void op_set_3_d() { op_set_r(d, 3); }
void op_set_4_d() { op_set_r(d, 4); }
void op_set_5_d() { op_set_r(d, 5); }
void op_set_6_d() { op_set_r(d, 6); }
void op_set_7_d() { op_set_r(d, 7); }

void op_set_0_e() { op_set_r(e, 0); }
void op_set_1_e() { op_set_r(e, 1); }
void op_set_2_e() { op_set_r(e, 2); }
void op_set_3_e() { op_set_r(e, 3); }
void op_set_4_e() { op_set_r(e, 4); }
void op_set_5_e() { op_set_r(e, 5); }
void op_set_6_e() { op_set_r(e, 6); }
void op_set_7_e() { op_set_r(e, 7); }

void op_set_0_h() { op_set_r(h, 0); }
void op_set_1_h() { op_set_r(h, 1); }
void op_set_2_h() { op_set_r(h, 2); }
void op_set_3_h() { op_set_r(h, 3); }
void op_set_4_h() { op_set_r(h, 4); }
void op_set_5_h() { op_set_r(h, 5); }
void op_set_6_h() { op_set_r(h, 6); }
void op_set_7_h() { op_set_r(h, 7); }

void op_set_0_l() { op_set_r(l, 0); }
void op_set_1_l() { op_set_r(l, 1); }
void op_set_2_l() { op_set_r(l, 2); }
void op_set_3_l() { op_set_r(l, 3); }
void op_set_4_l() { op_set_r(l, 4); }
void op_set_5_l() { op_set_r(l, 5); }
void op_set_6_l() { op_set_r(l, 6); }
void op_set_7_l() { op_set_r(l, 7); }

#define op_set_ind_hl(bit) alu_set(mem_addr(hl), bit); ipc(1); cycles(16); ei_di(); ints()
void op_set_0_ind_hl() { op_set_ind_hl(0); }
void op_set_1_ind_hl() { op_set_ind_hl(1); }
void op_set_2_ind_hl() { op_set_ind_hl(2); }
void op_set_3_ind_hl() { op_set_ind_hl(3); }
void op_set_4_ind_hl() { op_set_ind_hl(4); }
void op_set_5_ind_hl() { op_set_ind_hl(5); }
void op_set_6_ind_hl() { op_set_ind_hl(6); }
void op_set_7_ind_hl() { op_set_ind_hl(7); }

/* Bit-Ops: RES **********************************************************************/

#define op_res_r(reg, bit) alu_res(&reg, bit); ipc(1); cycles(8); ei_di(); ints()
void op_res_0_a() { op_res_r(a, 0); }
void op_res_1_a() { op_res_r(a, 1); }
void op_res_2_a() { op_res_r(a, 2); }
void op_res_3_a() { op_res_r(a, 3); }
void op_res_4_a() { op_res_r(a, 4); }
void op_res_5_a() { op_res_r(a, 5); }
void op_res_6_a() { op_res_r(a, 6); }
void op_res_7_a() { op_res_r(a, 7); }

void op_res_0_b() { op_res_r(b, 0); }
void op_res_1_b() { op_res_r(b, 1); }
void op_res_2_b() { op_res_r(b, 2); }
void op_res_3_b() { op_res_r(b, 3); }
void op_res_4_b() { op_res_r(b, 4); }
void op_res_5_b() { op_res_r(b, 5); }
void op_res_6_b() { op_res_r(b, 6); }
void op_res_7_b() { op_res_r(b, 7); }

void op_res_0_c() { op_res_r(c, 0); }
void op_res_1_c() { op_res_r(c, 1); }
void op_res_2_c() { op_res_r(c, 2); }
void op_res_3_c() { op_res_r(c, 3); }
void op_res_4_c() { op_res_r(c, 4); }
void op_res_5_c() { op_res_r(c, 5); }
void op_res_6_c() { op_res_r(c, 6); }
void op_res_7_c() { op_res_r(c, 7); }

void op_res_0_d() { op_res_r(d, 0); }
void op_res_1_d() { op_res_r(d, 1); }
void op_res_2_d() { op_res_r(d, 2); }
void op_res_3_d() { op_res_r(d, 3); }
void op_res_4_d() { op_res_r(d, 4); }
void op_res_5_d() { op_res_r(d, 5); }
void op_res_6_d() { op_res_r(d, 6); }
void op_res_7_d() { op_res_r(d, 7); }

void op_res_0_e() { op_res_r(e, 0); }
void op_res_1_e() { op_res_r(e, 1); }
void op_res_2_e() { op_res_r(e, 2); }
void op_res_3_e() { op_res_r(e, 3); }
void op_res_4_e() { op_res_r(e, 4); }
void op_res_5_e() { op_res_r(e, 5); }
void op_res_6_e() { op_res_r(e, 6); }
void op_res_7_e() { op_res_r(e, 7); }

void op_res_0_h() { op_res_r(h, 0); }
void op_res_1_h() { op_res_r(h, 1); }
void op_res_2_h() { op_res_r(h, 2); }
void op_res_3_h() { op_res_r(h, 3); }
void op_res_4_h() { op_res_r(h, 4); }
void op_res_5_h() { op_res_r(h, 5); }
void op_res_6_h() { op_res_r(h, 6); }
void op_res_7_h() { op_res_r(h, 7); }

void op_res_0_l() { op_res_r(l, 0); }
void op_res_1_l() { op_res_r(l, 1); }
void op_res_2_l() { op_res_r(l, 2); }
void op_res_3_l() { op_res_r(l, 3); }
void op_res_4_l() { op_res_r(l, 4); }
void op_res_5_l() { op_res_r(l, 5); }
void op_res_6_l() { op_res_r(l, 6); }
void op_res_7_l() { op_res_r(l, 7); }

#define op_res_ind_hl(bit) alu_res(mem_addr(hl), bit); ipc(1); cycles(16); ei_di(); ints()
void op_res_0_ind_hl() { op_res_ind_hl(0); }
void op_res_1_ind_hl() { op_res_ind_hl(1); }
void op_res_2_ind_hl() { op_res_ind_hl(2); }
void op_res_3_ind_hl() { op_res_ind_hl(3); }
void op_res_4_ind_hl() { op_res_ind_hl(4); }
void op_res_5_ind_hl() { op_res_ind_hl(5); }
void op_res_6_ind_hl() { op_res_ind_hl(6); }
void op_res_7_ind_hl() { op_res_ind_hl(7); }

/* Jumps */

#define op_jp_cc(condition) if (condition) { pc = r16(pc+1); cycles(16); } else { ipc(3); cycles(12); } ei_di(); ints()
void op_jp_nz() { op_jp_cc(!(f & FLAG_Z)); }
void op_jp_z() { op_jp_cc(f & FLAG_Z); }
void op_jp_nc() { op_jp_cc(!(f & FLAG_C)); }
void op_jp_c() { op_jp_cc(f & FLAG_C); }

void op_jp() { pc = r16(pc+1); cycles(16); }
void op_jp_hl() { pc = hl; cycles(4); }

#define op_jr_cc(condition) if (condition) { pc += (int8_t) r8_signed(pc+1) + 2; cycles(12); } else { ipc(2); cycles(8); } ei_di(); ints()
void op_jr_nz() { op_jr_cc(!(f & FLAG_Z)); }
void op_jr_z() { op_jr_cc(f & FLAG_Z); }
void op_jr_nc() { op_jr_cc(!(f & FLAG_C)); }
void op_jr_c() { op_jr_cc(f & FLAG_C); }

void op_jr() {pc += (int16_t) r8_signed(pc+1) + 2; cycles(8); }

/* Calls */

#define op_call_cc(condition) if (condition) { sp -= 2; w16(sp, pc+3); pc = r16(pc+1); cycles(24); } else { ipc(3); cycles(12); } ei_di(); ints()
void op_call_nz() { op_call_cc(!(f & FLAG_Z)); }
void op_call_z() { op_call_cc(f & FLAG_Z); }
void op_call_nc() { op_call_cc(!(f & FLAG_C)); }
void op_call_c() { op_call_cc(f & FLAG_C); }

void op_call() { sp -=2; w16(sp, pc+3); pc = r16(pc+1); cycles(24); ei_di(); ints(); }

/* Rst */

#define op_rst(loc) sp -= 2; w16(sp, pc+1); pc = loc; cycles(16); ei_di(); ints()
void op_rst_00() { op_rst(0x00); }
void op_rst_08() { op_rst(0x08); }
void op_rst_10() { op_rst(0x10); }
void op_rst_18() { op_rst(0x18); }
void op_rst_20() { op_rst(0x20); }
void op_rst_28() { op_rst(0x28); }
void op_rst_30() { op_rst(0x30); }
void op_rst_38() { op_rst(0x38); }

/* Ret */

void op_ret() { pc = r16(sp); sp += 2; cycles(16); ei_di(); ints(); }

#define op_ret_cc(condition) if (condition) { pc = r16(sp); sp += 2; cycles(20); } else { ipc(1); cycles(8); } ei_di(); ints()
void op_ret_nz() { op_ret_cc(!(f & FLAG_Z)); }
void op_ret_z() { op_ret_cc(f & FLAG_Z); }
void op_ret_nc() { op_ret_cc(!(f & FLAG_C)); }
void op_ret_c() { op_ret_cc(f & FLAG_C); }

/* Interrupt stuff */

void op_reti() { pc = r16(sp); sp += 2; cpu_ie = 1; cycles(8); ei_di(); ints(); } // Return & enable interrupts
void op_di() { cpu_di_pending = 1; ipc(1); cycles(4); ints(); } // Dísable interupts
void op_ei() { cpu_ei_pending = 1; ipc(1); cycles(4); ints(); } // Enable interupts

/* Random operations that have nothing to do with arithmetic or whatever */

void op_daa() { alu_daa(); ipc(1); cycles(4); ei_di(); ints(); } // decimal adjust a, f*** this s***

void op_cpl() { a = ~a; f |= FLAG_N | FLAG_H; ipc(1); cycles(4); ei_di(); ints(); } // cpl complement a
void op_ccf() { f = (f & FLAG_Z) | (~f & FLAG_C); ipc(1); cycles(4); ei_di(); ints(); } // ccf complement carry flag

void op_scf() { f = (f & FLAG_Z) | FLAG_C; ipc(1); cycles(4); ei_di(); ints(); } // set carry flag, discard N and H

void op_halt() {
    int32_t cycles_to_idle;
    int32_t video_idle_cycles;
    int32_t sys_idle_cycles;
    ipc(1);

    do {
        cycles(4);
    } while (!SYS_IF);

    ei_di();
    ints();
}

void op_stop() {
    volatile uint8_t last_buttons = sys_buttons_all;
    while(sys_buttons_all == last_buttons) {
        sys_update_buttons(); cycles(4);
    }
    ipc(2); cycles(4); ei_di(); ints();
}

void op_illegal() { print_msg("Illegal opcode %d\n", op); ipc(1); cycles(4); ei_di(); ints(); }

void op_prefix_cb() { ipc(1); ext_opcodes[r8(pc)](); };

const op_func opcodes[256] = {
//  x0              x1              x2              x3              x4              x5              x6              x7              x8              x9              xA              xB              xC              xD              xE              xF
    op_nop,         op_ld_bc_nn,    op_ld_ind_bc_a, op_inc16_bc,    op_inc8_b,      op_dec8_b,      op_ld_b_n,      op_rlca,        op_ld_ind_nn_sp,op_add16_hl_bc, op_ld_a_ind_bc, op_dec16_bc,    op_inc8_c,      op_dec8_c,      op_ld_c_n,      op_rrca,
    op_stop,        op_ld_de_nn,    op_ld_ind_de_a, op_inc16_de,    op_inc8_d,      op_dec8_d,      op_ld_d_n,      op_rla,         op_jr,          op_add16_hl_de, op_ld_a_ind_de, op_dec16_de,    op_inc8_e,      op_dec8_e,      op_ld_e_n,      op_rra,
    op_jr_nz,       op_ld_hl_nn,    op_ldi_ind_hl_a,op_inc16_hl,    op_inc8_h,      op_dec8_h,      op_ld_h_n,      op_daa,         op_jr_z,        op_add16_hl_hl, op_ldi_a_ind_hl,op_dec16_hl,    op_inc8_l,      op_dec8_l,      op_ld_l_n,      op_cpl,
    op_jr_nc,       op_ld_sp_nn,    op_ldd_ind_hl_a,op_inc16_sp,    op_inc8_ind_hl, op_dec8_ind_hl, op_ld_ind_hl_n, op_scf,         op_jr_c,        op_add16_hl_sp, op_ldd_a_ind_hl,op_dec16_sp,    op_inc8_a,      op_dec8_a,      op_ld_a_n,      op_ccf,

    op_nop,         op_ld_b_c,      op_ld_b_d,      op_ld_b_e,      op_ld_b_h,      op_ld_b_l,      op_ld_b_ind_hl, op_ld_b_a,      op_ld_c_b,      op_nop,         op_ld_c_d,      op_ld_c_e,      op_ld_c_h,      op_ld_c_l,      op_ld_c_ind_hl, op_ld_c_a,
    op_ld_d_b,      op_ld_d_c,      op_nop,         op_ld_d_e,      op_ld_d_h,      op_ld_d_l,      op_ld_d_ind_hl, op_ld_d_a,      op_ld_e_b,      op_ld_e_c,      op_ld_e_d,      op_nop,         op_ld_e_h,      op_ld_e_l,      op_ld_e_ind_hl, op_ld_e_a,
    op_ld_h_b,      op_ld_h_c,      op_ld_h_d,      op_ld_h_e,      op_nop,         op_ld_h_l,      op_ld_h_ind_hl, op_ld_h_a,      op_ld_l_b,      op_ld_l_c,      op_ld_l_d,      op_ld_l_e,      op_ld_l_h,      op_nop,         op_ld_l_ind_hl, op_ld_l_a,
    op_ld_ind_hl_b, op_ld_ind_hl_c, op_ld_ind_hl_d, op_ld_ind_hl_e, op_ld_ind_hl_h, op_ld_ind_hl_l, op_halt,        op_ld_ind_hl_a, op_ld_a_b,      op_ld_a_c,      op_ld_a_d,      op_ld_a_e,      op_ld_a_h,      op_ld_a_l,      op_ld_a_ind_hl, op_nop,

    op_add8_a_b,    op_add8_a_c,    op_add8_a_d,    op_add8_a_e,    op_add8_a_h,    op_add8_a_l,    op_add8_a_ind_hl,op_add8_a_a,   op_adc8_a_b,    op_adc8_a_c,    op_adc8_a_d,    op_adc8_a_e,    op_adc8_a_h,    op_adc8_a_l,    op_adc8_a_ind_hl,op_adc8_a_a,
    op_sub8_a_b,    op_sub8_a_c,    op_sub8_a_d,    op_sub8_a_e,    op_sub8_a_h,    op_sub8_a_l,    op_sub8_a_ind_hl,op_sub8_a_a,   op_sbc8_a_b,    op_sbc8_a_c,    op_sbc8_a_d,    op_sbc8_a_e,    op_sbc8_a_h,    op_sbc8_a_l,    op_sbc8_a_ind_hl,op_sbc8_a_a,
    op_and_a_b,     op_and_a_c,     op_and_a_d,     op_and_a_e,     op_and_a_h,     op_and_a_l,     op_and_a_ind_hl,op_and_a_a,     op_xor_a_b,     op_xor_a_c,     op_xor_a_d,     op_xor_a_e,     op_xor_a_h,     op_xor_a_l,     op_xor_a_ind_hl,op_xor_a_a,
    op_or_a_b,      op_or_a_c,      op_or_a_d,      op_or_a_e,      op_or_a_h,      op_or_a_l,      op_or_a_ind_hl, op_or_a_a,      op_cp_a_b,      op_cp_a_c,      op_cp_a_d,      op_cp_a_e,      op_cp_a_h,      op_cp_a_l,      op_cp_a_ind_hl, op_cp_a_a,

    op_ret_nz,      op_pop_bc,      op_jp_nz,       op_jp,          op_call_nz,     op_push_bc,     op_add8_a_imm,  op_rst_00,      op_ret_z,       op_ret,         op_jp_z,        op_prefix_cb,   op_call_z,      op_call,        op_adc8_a_imm,  op_rst_08,
    op_ret_nc,      op_pop_de,      op_jp_nc,       op_illegal,     op_call_nc,     op_push_de,     op_sub8_a_imm,  op_rst_10,      op_ret_c,       op_reti,        op_jp_c,        op_illegal,     op_call_c,      op_illegal,     op_sbc8_a_imm,  op_rst_18,
    op_ldh_n_a,     op_pop_hl,      op_ld_ind_ff00_c_a,op_illegal,  op_illegal,     op_push_hl,     op_and_a_imm,   op_rst_20,      op_add16_sp_imm,op_jp_hl,       op_ld_ind_mem_a,op_illegal,     op_illegal,     op_illegal,     op_xor_a_imm,   op_rst_28,
    op_ldh_a_n,     op_pop_af,      op_ld_a_ind_ff00_c,op_di,       op_illegal,     op_push_af,     op_or_a_imm,    op_rst_30,      op_ld_hl_sp_n,  op_ld_sp_hl,    op_ld_a_ind_mem,op_ei,          op_illegal,     op_illegal,     op_cp_a_imm,    op_rst_38
};

const op_func ext_opcodes[256] = {
//  x0              x1              x2              x3              x4              x5              x6              x7              x8              x9              xA              xB              xC              xD              xE              xF
    op_rlc_b,       op_rlc_c,       op_rlc_d,       op_rlc_e,       op_rlc_h,       op_rlc_l,       op_rlc_ind_hl,  op_rlc_a,       op_rrc_b,       op_rrc_c,       op_rrc_d,       op_rrc_e,       op_rrc_h,       op_rrc_l,       op_rrc_ind_hl,  op_rrc_a,
    op_rl_b,        op_rl_c,        op_rl_d,        op_rl_e,        op_rl_h,        op_rl_l,        op_rl_ind_hl,   op_rl_a,        op_rr_b,        op_rr_c,        op_rr_d,        op_rr_e,        op_rr_h,        op_rr_l,        op_rr_ind_hl,   op_rr_a,
    op_sla_b,       op_sla_c,       op_sla_d,       op_sla_e,       op_sla_h,       op_sla_l,       op_sla_ind_hl,  op_sla_a,       op_sra_b,       op_sra_c,       op_sra_d,       op_sra_e,       op_sra_h,       op_sra_l,       op_sra_ind_hl,  op_sra_a,
    op_swap_b,      op_swap_c,      op_swap_d,      op_swap_e,      op_swap_h,      op_swap_l,      op_swap_ind_hl, op_swap_a,      op_srl_b,       op_srl_c,       op_srl_d,       op_srl_e,       op_srl_h,       op_srl_l,       op_srl_ind_hl,  op_srl_a,

    op_bit_0_b,     op_bit_0_c,     op_bit_0_d,     op_bit_0_e,     op_bit_0_h,     op_bit_0_l,     op_bit_0_ind_hl,op_bit_0_a,     op_bit_1_b,     op_bit_1_c,     op_bit_1_d,     op_bit_1_e,     op_bit_1_h,     op_bit_1_l,     op_bit_1_ind_hl,op_bit_1_a,
    op_bit_2_b,     op_bit_2_c,     op_bit_2_d,     op_bit_2_e,     op_bit_2_h,     op_bit_2_l,     op_bit_2_ind_hl,op_bit_2_a,     op_bit_3_b,     op_bit_3_c,     op_bit_3_d,     op_bit_3_e,     op_bit_3_h,     op_bit_3_l,     op_bit_3_ind_hl,op_bit_3_a,
    op_bit_4_b,     op_bit_4_c,     op_bit_4_d,     op_bit_4_e,     op_bit_4_h,     op_bit_4_l,     op_bit_4_ind_hl,op_bit_4_a,     op_bit_5_b,     op_bit_5_c,     op_bit_5_d,     op_bit_5_e,     op_bit_5_h,     op_bit_5_l,     op_bit_5_ind_hl,op_bit_5_a,
    op_bit_6_b,     op_bit_6_c,     op_bit_6_d,     op_bit_6_e,     op_bit_6_h,     op_bit_6_l,     op_bit_6_ind_hl,op_bit_6_a,     op_bit_7_b,     op_bit_7_c,     op_bit_7_d,     op_bit_7_e,     op_bit_7_h,     op_bit_7_l,     op_bit_7_ind_hl,op_bit_7_a,

    op_res_0_b,     op_res_0_c,     op_res_0_d,     op_res_0_e,     op_res_0_h,     op_res_0_l,     op_res_0_ind_hl,op_res_0_a,     op_res_1_b,     op_res_1_c,     op_res_1_d,     op_res_1_e,     op_res_1_h,     op_res_1_l,     op_res_1_ind_hl,op_res_1_a,
    op_res_2_b,     op_res_2_c,     op_res_2_d,     op_res_2_e,     op_res_2_h,     op_res_2_l,     op_res_2_ind_hl,op_res_2_a,     op_res_3_b,     op_res_3_c,     op_res_3_d,     op_res_3_e,     op_res_3_h,     op_res_3_l,     op_res_3_ind_hl,op_res_3_a,
    op_res_4_b,     op_res_4_c,     op_res_4_d,     op_res_4_e,     op_res_4_h,     op_res_4_l,     op_res_4_ind_hl,op_res_4_a,     op_res_5_b,     op_res_5_c,     op_res_5_d,     op_res_5_e,     op_res_5_h,     op_res_5_l,     op_res_5_ind_hl,op_res_5_a,
    op_res_6_b,     op_res_6_c,     op_res_6_d,     op_res_6_e,     op_res_6_h,     op_res_6_l,     op_res_6_ind_hl,op_res_6_a,     op_res_7_b,     op_res_7_c,     op_res_7_d,     op_res_7_e,     op_res_7_h,     op_res_7_l,     op_res_7_ind_hl,op_res_7_a,

    op_set_0_b,     op_set_0_c,     op_set_0_d,     op_set_0_e,     op_set_0_h,     op_set_0_l,     op_set_0_ind_hl,op_set_0_a,     op_set_1_b,     op_set_1_c,     op_set_1_d,     op_set_1_e,     op_set_1_h,     op_set_1_l,     op_set_1_ind_hl,op_set_1_a,
    op_set_2_b,     op_set_2_c,     op_set_2_d,     op_set_2_e,     op_set_2_h,     op_set_2_l,     op_set_2_ind_hl,op_set_2_a,     op_set_3_b,     op_set_3_c,     op_set_3_d,     op_set_3_e,     op_set_3_h,     op_set_3_l,     op_set_3_ind_hl,op_set_3_a,
    op_set_4_b,     op_set_4_c,     op_set_4_d,     op_set_4_e,     op_set_4_h,     op_set_4_l,     op_set_4_ind_hl,op_set_4_a,     op_set_5_b,     op_set_5_c,     op_set_5_d,     op_set_5_e,     op_set_5_h,     op_set_5_l,     op_set_5_ind_hl,op_set_5_a,
    op_set_6_b,     op_set_6_c,     op_set_6_d,     op_set_6_e,     op_set_6_h,     op_set_6_l,     op_set_6_ind_hl,op_set_6_a,     op_set_7_b,     op_set_7_c,     op_set_7_d,     op_set_7_e,     op_set_7_h,     op_set_7_l,     op_set_7_ind_hl,op_set_7_a
};
