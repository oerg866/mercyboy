#include "cpu.h"


inline void alu_flags(uint8_t z, uint8_t n, uint8_t h, uint8_t c) {
    *flags = z|n|h|c;
}

void alu_add8(uint8_t *a, uint8_t b) {
    uint16_t result = *a + b;
    *flags = 0;
    uint8_t halfcarry = (((*a & 0x0F) + (b & 0x0F)) << 1) & FLAG_H;
    *a += b;
    // z flag
    if (*a == 0) *flags |= FLAG_Z;
    // n is reset
    // h flag
    *flags |= halfcarry;
    // c flag
    if ((result >> 8) > 0) *flags |= FLAG_C;
}

void alu_add16(uint16_t *a, uint16_t b) {
    uint32_t result = *a + b;
    *flags &= FLAG_Z;
    *a += b;
    uint16_t halfcarry = (((*a & 0x0FFF) + (b & 0x0FFF)) >> (8-1)) & FLAG_H;
    // z flag is ignored
    // n is reset
    // h flag
    *flags |= halfcarry;
    // c flag
    if ((result >> 16) > 0) *flags |= FLAG_C;
}

void alu_sub8(uint8_t *a, uint8_t b) {
    uint16_t result = *a - b;
    *flags = FLAG_N;
    uint8_t halfcarry = (((*a & 0x0F) - (b & 0x0F)) << 1) & FLAG_H;
    *a -= b;
    // z flag

    if (*a == 0) *flags |= FLAG_Z;
    // n is reset
    // h flag
    *flags |= halfcarry;
    // c flag
    if ((result >> 8) > 0) *flags |= FLAG_C;
  //  return (result & 0xFF);
}

void alu_sub16(uint16_t *a, uint16_t b) {
    uint32_t result = *a - b;
    *flags &= FLAG_Z;
    uint16_t halfcarry = (((*a & 0x0FFF) + (b & 0x0FFF)) >> (8-1)) & FLAG_H;
    // z flag
    // n is reset
    // h flag
    *flags |= halfcarry;
    // c flag
    if ((result >> 16) > 0) *flags |= FLAG_C;
  //  return (result & 0xFF);
}


void alu_and(uint8_t *a, uint8_t b) {
    *a = *a & b;
    uint8_t z = (*a == 0) << 7;
    alu_flags(z, 0, FLAG_H, 0);
}

void alu_or(uint8_t *a, uint8_t b) {
    *a = *a | b;
    uint8_t z = (*a == 0) << 7;
    alu_flags(z, 0, 0 ,0);
}

void alu_xor(uint8_t *a, uint8_t b) {
    *a = *a ^ b;
    uint8_t z = (*a == 0) << 7;
    alu_flags(z, 0, 0 ,0);
}


void alu_inc8(uint8_t *a) {
//    uint16_t result = *a + 1;
    *flags = *flags & FLAG_C; // Keep C flag intact
    uint8_t halfcarry = (((*a & 0x0F) + 1) << 1) & FLAG_H;
    // z flag
    (*a)++;
    if (*a == 0) *flags |= FLAG_Z;
    // n is reset
    // h flag
    *flags |= halfcarry;
    // c flag is kept as is

}


inline void alu_inc16(uint16_t *a) {
    *a = bs(bs(*a) + 1);
}

void alu_dec8(uint8_t *a) {
    uint16_t result = *a - 1;
    *a -= 1;
    *flags = (*flags & FLAG_C) | FLAG_N;
    uint8_t halfcarry = (((*a & 0x0F) - 1) << 1) & FLAG_H;
    // z flag
    if (*a == 0) *flags |= FLAG_Z;
    // n is reset
    // h flag
    *flags |= halfcarry;
    // c flag is kept as is
}

inline void alu_dec16(uint16_t *a) {
    *a = bs(bs(*a) - 1);
}

void alu_swap(uint8_t *a) {
    uint8_t b = *a >> 4;
    *a = (*a << 4) | b;
    *flags = 0;
    if (*a == 0) *flags = FLAG_Z;
}



// Rotates left

void alu_rl(uint8_t *a) {
    // rl 9bit rotate a left, b7 to carry
    uint8_t newcarry = (*a >> 3) & FLAG_C; // get bit 7 into bit 4, reset everything else
    *a = (*a << 1) | ((*flags & FLAG_C) >> 4); // Shift + old carry
    *flags = newcarry; // new carry
    if (*a == 0) *flags |= FLAG_Z; // set zero flag accordingly
}

void alu_rlc(uint8_t *a) {
    // RLC 8 bit left, b7 to carry and b0
    *flags = (*a >> 3) & FLAG_C; // get bit 7 into bit 4, reset everything else
    *a = (*a << 1) | (*a >> 7);
    if (*a == 0) *flags |= FLAG_Z; // set zero flag accordingly
}

void alu_rr(uint8_t *a) {
    // rr 9bit rotate a right, b0 to carry
    uint8_t newcarry = (*a << 4) & FLAG_C; // get bit 7 into bit 4, reset everything else
    *a = (*a >> 7) | ((*flags & FLAG_C) << 3); // Shift + old carry
    *flags = newcarry; // new carry
    if (*a == 0) *flags |= FLAG_Z; // set zero flag accordingly
}

void alu_rrc(uint8_t *a) {
    // RRC 8 bit right, b0 to carry and b7
    *flags = (*a << 4) & FLAG_C; // get bit 0 into bit 4, reset everything else
    *a = (*a >> 1) | (*a << 7);
    if (*a == 0) *flags |= FLAG_Z; // set zero flag accordingly
}


void alu_sl(uint8_t *a) {
    *flags = (*a >> 3); // Copy bit 7 to c flag
    *a = (*a << 1);
    if (*a == 0) *flags |= FLAG_Z;
}

void alu_sra(uint8_t *a) {
    // Arithmetic shift right
    *flags = (*a << 4) & FLAG_C; // Copy bit 0 to c flag
    *a = (*a & 0x80) | (*a >> 1); // Arithmetic shift: MSB doesn't change
    if (*a == 0) *flags |= FLAG_Z;
}

void alu_srl(uint8_t *a) {
    // Logic shift right
    *flags = (*a << 4) & FLAG_C; // Copy bit 0 to c flag
    *a = (*a >> 1); // Logic shift: MSB set to 0
    if (*a == 0) *flags |= FLAG_Z;
}

inline void alu_bit(uint8_t *a, uint8_t b) {
    *flags = (*flags & FLAG_C) | FLAG_H; // C untouched, H set, N reset
    if ((*a & (1 << b)) == 0) *flags |= FLAG_Z;
}

inline void alu_set(uint8_t *a, uint8_t b) {
    // set bit
    *a |= (1 << b);
}

inline void alu_res(uint8_t *a, uint8_t b) {
    // reset bit
    *a &= ~(1 << b);
}


