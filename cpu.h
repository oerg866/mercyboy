#pragma once

#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define REG_AF  0x00
#define REG_BC  0x01
#define REG_DE  0x02
#define REG_HL  0x03
#define REG_SP  0x04
#define REG_PC  0x05

#define REG_A   0x00
#define REG_F   0x01
#define REG_B   0x02
#define REG_C   0x03
#define REG_D   0x04
#define REG_E   0x05
#define REG_H   0x06
#define REG_L   0x07

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

#if defined(__WIN32__)

#include <stdlib.h>
#define bs _byteswap_ushort

#elif defined(__linux__)

#define bs bswap_16
#include <byteswap.h>

#elif defined(__APPLE__)

#include <arpa/inet.h>
#define bs ntohs

#endif

extern uint8_t      regs8[];
extern uint16_t    *regs16;
extern uint8_t     *flags;

extern uint16_t    *pc;
extern uint16_t    *hl;
extern uint16_t    *bc;
extern uint16_t    *de;
extern uint16_t    *sp;

extern uint8_t      op;

extern uint8_t      cpu_ie;
extern uint8_t      cpu_ei_pending;
extern uint8_t      cpu_di_pending;

extern uint8_t      cpu_halted;

// CPU.C

void sr8(uint16_t reg, uint8_t n);
void sr16(uint16_t reg, uint16_t n);
void spc(uint16_t n);
void ipc(uint16_t n);
void cpu_init();
void run();
void cpu_ext_op();
void cycles(uint16_t n);
uint16_t process_interrupts();

// CPU_MEM.C

uint8_t cpu_read8(uint16_t addr);
int8_t cpu_read8_signed(uint16_t addr);
uint16_t cpu_read16(uint16_t addr);

void cpu_write8(uint16_t addr, uint8_t n);
void cpu_write16(uint16_t addr, uint16_t n);


// CPU_ALU.C

void alu_flags(uint8_t z, uint8_t n, uint8_t h, uint8_t c);
void alu_add8(uint8_t *a, uint8_t b);
void alu_adc8(uint8_t *a, uint16_t b);
void alu_add16(uint16_t *a, uint16_t b);
void alu_add16_imm8 (uint16_t *a, uint16_t b);
void alu_sub8(uint8_t *a, uint8_t b);
void alu_sbc8(uint8_t *a, uint16_t b);
void alu_sub16(uint16_t *a, uint16_t b);
void alu_and(uint8_t *a, uint8_t b);
void alu_or(uint8_t *a, uint8_t b);
void alu_xor(uint8_t *a, uint8_t b);
void alu_inc8(uint8_t *a);
void alu_dec8(uint8_t *a);
void alu_inc16(uint16_t *a);
void alu_dec16(uint16_t *a);
void alu_swap(uint8_t *a);
void alu_rl(uint8_t *a);
void alu_rlc(uint8_t *a);
void alu_rr(uint8_t *a);
void alu_rrc(uint8_t *a);

void alu_sl(uint8_t *a);
void alu_sra(uint8_t *a);
void alu_srl(uint8_t *a);

void alu_bit(uint8_t *a, uint8_t b);
void alu_set(uint8_t *a, uint8_t b);
void alu_res(uint8_t *a, uint8_t b);

// CPU_OPCODES.C

int op_get_operand8_a(uint8_t byte, int bitshift);
uint8_t op_get_cc(uint8_t byte);

void op_ld_r_imm();
void op_ld_a_imm();
void op_ld_r_r();
void op_ld_a_r();
void op_ld_same();
void op_ld_r_ind_hl();
void op_ld_ind_hl_imm();
void op_ld_ind_bc_a();
void op_ld_ind_de_a();
void op_ld_ind_hl_a();
void op_ld_ind_mem_a();
void op_ld_a_ind_bc();
void op_ld_a_ind_de();
void op_ld_a_ind_hl();
void op_ld_a_ind_mem();
void op_ld_ind_hl_r();
void op_ld_a_c();
void op_ld_c_a();
void op_ldd_a_ind_hl();
void op_ldd_ind_hl_a();
void op_ldi_a_ind_hl();
void op_ldi_ind_hl_a();
void op_ldh_n_a();
void op_ldh_a_n();
void op_ld_r_imm16();
void op_ld_sp_hl();
void op_ldhl_sp_imm();
void op_ld_ind_imm_sp();

void op_push16();
void op_pop16();

void op_add8_a_r();
void op_add8_a_ind_hl();
void op_add8_a_imm();

void op_add16_hl_r();
void op_add16_sp_imm();

void op_adc_a_r();
void op_adc_a_ind_hl();
void op_adc_a_imm();

void op_sub_a_r();
void op_sub_a_ind_hl();
void op_sub_a_imm();

void op_sbc_a_r();
void op_sbc_a_ind_hl();
void op_sbc_a_imm();

void op_and_a_r();
void op_and_a_ind_hl();
void op_and_a_imm();

void op_or_a_r();
void op_or_a_ind_hl();
void op_or_a_imm();

void op_xor_a_r();
void op_xor_a_ind_hl();
void op_xor_a_imm();

void op_cp_r();
void op_cp_ind_hl();
void op_cp_imm();

void op_inc8_r();
void op_inc8_ind_hl();
void op_inc16_r();

void op_dec8_r();
void op_dec8_ind_hl();
void op_dec16_r();

void op_swap_r();
void op_swap_ind_hl();


void op_daa();
void op_cpl();
void op_ccf();
void op_scf();


void op_nop();
void op_halt();
void op_stop();

void op_rla();

void op_rl_r();
void op_rl_ind_hl();
void op_rlca();
void op_rlc_r();

void op_rlc_ind_hl();

void op_rra();

void op_rr_r();
void op_rr_ind_hl();


void op_rrca();
void op_rrc_r();
void op_rrc_ind_hl();
void op_sla_r();

void op_sla_ind_hl();
void op_sra_r();
void op_sra_ind_hl();
void op_srl_r();
void op_srl_ind_hl();
void op_bit_r();
void op_bit_ind_hl();

void op_set_r();

void op_set_ind_hl();
void op_res_r();
void op_res_ind_hl();
void op_jp();
void op_jp_cc();
void op_jp_ind_hl();
void op_jr();
void op_jr_cc();

void op_call();

void op_call_cc();

void op_rst();

void op_ret();

void op_ret_cc();
void op_reti();

void op_di();
void op_ei();

#endif // CPU_H
