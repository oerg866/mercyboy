#pragma once

#ifndef SYS_H
#define SYS_H

#include <stdlib.h>
#include <stdint.h>
#include "mem.h"
#include "cpu.h"

#define CT_ROMONLY 0
#define CT_MBC1 1
#define CT_MBC1RAM 2
#define CT_MBC1RAMBATT 3
#define CT_MBC2 5
#define CT_MBC2BATT 6

#define MBC1_16_8 0
#define MBC1_4_32 1


#define TIMER_4096  0x00
#define TIMER_262144  0x01
#define TIMER_65536  0x02
#define TIMER_16384  0x03

#define JOY_BUTTONS 0x10
#define JOY_DPAD 0x20

#define sys_timer     ram_io[0x05]
#define sys_timer_mod ram_io[0x06]
#define sys_timer_cfg ram_io[0x07]

#define sys_joypad    ram_io[0x00]

extern uint16_t sys_dma_source;
extern uint8_t sys_dma_counter;
extern uint8_t sys_dma_busy;


extern uint8_t sys_carttype;
extern uint8_t sys_mbc1_s;
extern uint8_t sys_romsize;
extern int16_t sys_timer_speed;

extern uint8_t sys_buttons_all;

void sys_dma_cycles(int cycles);
void sys_cycles(int cycles);
void sys_handle_joypads();

uint8_t sys_read_joypad();

#endif // SYS_H
