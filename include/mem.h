#pragma once

#ifndef MEM_H
#define MEM_H

#include "compat.h"

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

extern uint8_t ram_ext[0x20000]; // Up to 128K

extern uint8_t ram_ie;         // FFFF

#define MEM_JOYPAD  0xFF00
#define MEM_DIV     0xFF04
#define MEM_TMA     0xFF06
#define MEM_TAC     0xFF07
#define MEM_IF      0xFF0F

#define MEM_NR10    0xFF10
#define MEM_NR11    0xFF11
#define MEM_NR12    0xFF12
#define MEM_NR13    0xFF13
#define MEM_NR14    0xFF14

#define MEM_NR21    0xFF16
#define MEM_NR22    0xFF17
#define MEM_NR23    0xFF18
#define MEM_NR24    0xFF19

#define MEM_NR30    0xFF1a
#define MEM_NR31    0xFF1b
#define MEM_NR32    0xFF1c
#define MEM_NR33    0xFF1d
#define MEM_NR34    0xFF1e

#define MEM_NR41    0xFF20
#define MEM_NR42    0xFF21
#define MEM_NR43    0xFF22
#define MEM_NR44    0xFF23

#define MEM_NR50    0xFF24
#define MEM_NR51    0xFF25
#define MEM_NR52    0xFF26

#define MEM_LCDC    0xFF40
#define MEM_SCY     0xFF42
#define MEM_SCX     0xFF43
#define MEM_LINE    0xFF44
#define MEM_DMA     0xFF46
#define MEM_BGP     0xFF47
#define MEM_OBP0    0xFF48
#define MEM_OBP1    0xFF49


#define AUDIO_NR50 ram_io[0x24]
#define AUDIO_NR51 ram_io[0x25]
#define AUDIO_NR52 ram_io[0x26]

void mem_init_float(float* dest, int count);
void mem_init_uint8(uint8_t* dest, int count);

int mem_init(uint8_t *romfile, int fsize);
uint8_t* mem_addr(uint16_t addr);

// These force a read without DMA checks. Used by DMA (so we don't block transfers blocked by DMA for the DMA...)

uint8_t cpu_read8_force(uint16_t addr);
uint16_t cpu_read16_force(uint16_t addr);

#endif // MEM_H
