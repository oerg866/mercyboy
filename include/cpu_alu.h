#ifndef CPU_ALU_H
#define CPU_ALU_H

#include "compat.h"
#include "cpu.h"
#include "cpu_help.h"

static inline void alu_add8(uint8_t *op1, uint8_t op2) {
    uint16_t result = *op1 + op2;
    uint8_t halfcarry = (((*op1 & 0x0F) + (op2 & 0x0F)) << 1) & FLAG_H;

    *op1 = (uint8_t) result;
    // Flags: n reset / h, c, z handled
    f = halfcarry;
    if (result & 0xFF00) f |= FLAG_C;
    if (*op1 == 0) f |= FLAG_Z;
}

static inline void alu_adc8(uint8_t *op1, uint16_t op2) {
    uint8_t oldcarry = (f & FLAG_C) >> 4;
    uint16_t result = *op1 + op2 + oldcarry;
    uint8_t halfcarry = (((*op1 & 0x0F) + (op2 & 0x0F) + oldcarry) << 1) & FLAG_H;

    *op1 = (uint8_t) result;
    // Flags: n reset / h, c, z handled
    f = halfcarry;
    if (result & 0xFF00) f |= FLAG_C;
    if (*op1 == 0) f |= FLAG_Z;
}

static inline void alu_add16(uint16_t *op1, uint16_t op2) {
    uint32_t result = *op1 + op2;
    uint16_t halfcarry = (((*op1 & 0x0FFF) + (op2 & 0x0FFF)) >> (8-1)) & FLAG_H;

    *op1 = (uint16_t) result;
    // Flags: z unchanged / n reset / h, c handled
    f &= FLAG_Z;
    f |= halfcarry;
    if (result & 0xFFFF0000) f |= FLAG_C;
}

static inline void alu_add16_imm8 (uint16_t *op1, uint16_t op2) {
    // 2nd parameter is 16 bit for sign extension in negative numbers...
    // Flags: z reset / n reset / h, c handled
    f = (((*op1 & 0x0F) + (op2 & 0x0F)) << 1) & FLAG_H; // Contrary to normal 16 bit arithmetic, bit 3 is taken for half-carry
    if (((*op1 & 0xFF) + (op2 & 0xFF)) & 0xFF00) f |= FLAG_C; // this goes through 8 bit ALU so this is a bit hacky.
    *op1 += op2;
}

static inline void alu_sub8(uint8_t *op1, uint8_t op2) {
    uint16_t result = *op1 - op2;
    uint8_t halfcarry = (((*op1 & 0x0F) - (op2 & 0x0F)) << 1) & FLAG_H;

    *op1 = result;
    // Flags: n set / h, c, z handled
    f = FLAG_N | halfcarry;
    if (*op1 == 0) f |= FLAG_Z;
    if (result & 0xFF00) f |= FLAG_C;
}

static inline void alu_cp(uint8_t *op1, uint8_t op2) {    // like alu_sub8, except it doesn't save the result
    uint16_t result = *op1 - op2;
    uint8_t halfcarry = (((*op1 & 0x0F) - (op2 & 0x0F)) << 1) & FLAG_H;

    // Flags: n set / h, c, z handled
    f = FLAG_N | halfcarry;
    if (result == 0) f |= FLAG_Z;
    if (result & 0xFF00) f |= FLAG_C;
}

static inline void alu_sbc8(uint8_t *op1, uint16_t op2) {
    uint8_t oldcarry = (f & FLAG_C) >> 4;
    uint16_t result = *op1 - op2 - oldcarry;
    uint8_t halfcarry = (((*op1 & 0x0F) - (op2 & 0x0F) - oldcarry) << 1) & FLAG_H;

    *op1 = result;
    // Flags: n set / h, c, z handled
    f = FLAG_N | halfcarry;
    if (*op1 == 0) f |= FLAG_Z;
    if (result & 0xFF00) f |= FLAG_C;
}

static inline void alu_sub16(uint16_t *op1, uint16_t op2) {
    uint32_t result = *op1 - op2;
    uint16_t halfcarry = (((*op1 & 0x0FFF) + (op2 & 0x0FFF)) >> (8-1)) & FLAG_H;

    // Flags: z unchanged / n reset / h, c handled
    f &= FLAG_Z;
    f |= halfcarry;
    if (result & 0xFFFF0000) f |= FLAG_C;
}


static inline void alu_and(uint8_t *op1, uint8_t op2) {
    *op1 = *op1 & op2;
    f = FLAG_H; // Flags: h set / n, c reset / z handled
    if (*op1 == 0) f |= FLAG_Z;
}

static inline void alu_or(uint8_t *op1, uint8_t op2) {
    *op1 = *op1 | op2;
    f = (*op1 == 0) ? FLAG_Z : 0; // Flags: h, n, c reset / z handled
}

static inline void alu_xor(uint8_t *op1, uint8_t op2) {
    *op1 = *op1 ^ op2;
    f = (*op1 == 0) ? FLAG_Z : 0; // Flags: h, n, c reset / z handled
}

static inline void alu_inc8(uint8_t *op1) {
    // n flag is reset
    f &= FLAG_C; // Keep C flag intact
    f |= (((*op1 & 0x0F) + 1) << 1) & FLAG_H; // Half Carry
    (*op1)++;
    if (*op1 == 0) f |= FLAG_Z; // Zero flag
}

static inline void alu_inc16(uint16_t *op1) {
    *op1 += 1;
}

static inline void alu_dec8(uint8_t *op1) {
    f = (f & FLAG_C) | FLAG_N; // c flag is kept as is + set N flag
    f = f | ((((*op1 & 0x0F) - 1) << 1) & FLAG_H); // Half Carry
    *op1 -= 1;
    if (*op1 == 0) f |= FLAG_Z; // Zero flag
}

static inline void alu_dec16(uint16_t *op1) {
    *op1 -= 1;
}

static inline void alu_swap(uint8_t *op1) {
    *op1 = (*op1 << 4) | (*op1 >> 4);
    f = (*op1 == 0) ? FLAG_Z : 0; // Flags: h, n, c reset / z handled
}

// Rotates left

static inline void alu_rl(uint8_t *op1) {
    // rl 9bit rotate a left, b7 to carry
    uint8_t newcarry = (*op1 >> 3) & FLAG_C; // get bit 7 into bit 4, reset everything else
    *op1 = (*op1 << 1) | ((f & FLAG_C) >> 4); // Shift + old carry
    f = newcarry; // new carry
    if (*op1 == 0) f |= FLAG_Z; // set zero flag accordingly
}

static inline void alu_rlc(uint8_t *op1) {
    // RLC 8 bit left, b7 to carry and b0
    f = (*op1 >> 3) & FLAG_C; // get bit 7 into bit 4, reset everything else
    *op1 = (*op1 << 1) | (*op1 >> 7);
    if (*op1 == 0) f |= FLAG_Z; // set zero flag accordingly
}

static inline void alu_rr(uint8_t *op1) {
    // rr 9bit rotate a right, b0 to carry
    uint8_t newcarry = (*op1 << 4) & FLAG_C; // get bit 0 into bit 4, reset everything else
    *op1 = (*op1 >> 1) | ((f & FLAG_C) << 3); // Shift + old carry
    f = newcarry; // new carry
    if (*op1 == 0) f |= FLAG_Z; // set zero flag accordingly
}

static inline void alu_rrc(uint8_t *op1) {
    // RRC 8 bit right, b0 to carry and b7
    f = (*op1 << 4) & FLAG_C; // get bit 0 into bit 4, reset everything else
    *op1 = (*op1 >> 1) | (*op1 << 7);
    if (*op1 == 0) f |= FLAG_Z; // set zero flag accordingly
}


static inline void alu_sl(uint8_t *op1) {
    f = (*op1 >> 3) & FLAG_C; // Copy bit 7 to c flag
    *op1 = (*op1 << 1);
    if (*op1 == 0) f |= FLAG_Z;
}

static inline void alu_sra(uint8_t *op1) {
    // Arithmetic shift right
    f = (*op1 << 4) & FLAG_C; // Copy bit 0 to c flag
    *op1 = (*op1 & 0x80) | (*op1 >> 1); // Arithmetic shift: MSB doesn't change
    if (*op1 == 0) f |= FLAG_Z;
}

static inline void alu_srl(uint8_t *op1) {
    // Logic shift right
    f = (*op1 << 4) & FLAG_C; // Copy bit 0 to c flag
    *op1 = (*op1 >> 1); // Logic shift: MSB set to 0
    if (*op1 == 0) f |= FLAG_Z;
}

static inline void alu_bit(uint8_t *op1, uint8_t op2) {
    f = (f & FLAG_C) | FLAG_H; // C untouched, H set, N reset
    if ((*op1 & (1 << op2)) == 0) f |= FLAG_Z;
}

static inline void alu_set(uint8_t *op1, uint8_t op2) {
    // set bit
    *op1 |= (1 << op2);
}

static inline void alu_res(uint8_t *op1, uint8_t op2) {
    // reset bit
    *op1 &= ~(1 << op2);
}

static inline void alu_daa() {
    uint8_t f_n = f & FLAG_N;
    uint8_t f_c = f & FLAG_C;
    uint8_t f_h = f & FLAG_H;
    uint8_t f_z;

    if (!f_n) {
        // after addition, adjust if half carry or carry occured
        // or if result is out of bounds
        if (f_c || (a > 0x99)) {
            a += 0x60; f_c = FLAG_C;
        }
        if (f_h || ((a & 0x0F) > 0x09)) {
            a += 0x06;
        }
    } else {
        // after subtraction, only adjust if half carry or carry occured
        if (f_c) {
            a -= 0x60;
        }
        if (f_h) {
            a -= 0x06;
        }
    }

    f_z = (a == 0) ? FLAG_Z : 0;

    f = f_n | f_c | f_z;
}
#endif
