#pragma once

#ifndef MEM_H
#define MEM_H

#include <stdint.h>

extern uint8_t *romfile;
extern uint8_t *addonram;

extern uint8_t rom1[0x8000];
extern uint8_t *rom2;
extern uint8_t vram[0x2000];
extern uint8_t *ram2;
extern uint8_t ram1[0x2000];
extern uint8_t oam[0xA0];      // FE00 - FE9F
extern uint8_t ram_uio1[0x60]; // FEA0 - FEFF
extern uint8_t ram_io[0x4C];   // FFF0 - FF4B
extern uint8_t ram_uio2[0x34]; // FE4C - FF7F
extern uint8_t ram_int[0x80];  // FF80 - FFFE

extern uint8_t ram_ie;         // FFFF

#define MEM_JOYPAD  0xFF00
#define MEM_DIV     0xFF04
#define MEM_TMA     0xFF06
#define MEM_TAC     0xFF07
#define MEM_IF      0xFF0F
#define MEM_LINE    0xFF44
#define MEM_DMA     0xFF46

int mem_init(uint8_t *romfile, int fsize);
uint8_t* mem_addr(uint16_t addr);

// These force a read without DMA checks. Used by DMA (so we don't block transfers blocked by DMA for the DMA...)

uint8_t cpu_read8_force(uint16_t addr);
uint16_t cpu_read16_force(uint16_t addr);

#endif // MEM_H
