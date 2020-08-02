#pragma once

#ifndef SYS_H
#define SYS_H

#include <stdint.h>

#define CT_ROMONLY 0
#define CT_MBC1 1
#define CT_MBC1RAM 2
#define CT_MBC1RAMBATT 3
#define CT_MBC2 5
#define CT_MBC2BATT 6

#define MBC1_2048_8 0
#define MBC1_512_32 1


#define TIMER_4096  0x00
#define TIMER_262144  0x01
#define TIMER_65536  0x02
#define TIMER_16384  0x03

#define JOY_BUTTONS 0x10
#define JOY_DPAD 0x20

#define SYS_JOYPAD    ram_io[0x00]
#define SYS_DIV       ram_io[0x04]
#define SYS_TIMER     ram_io[0x05]
#define SYS_TIMER_MOD ram_io[0x06]
#define SYS_TIMER_CFG ram_io[0x07]
#define SYS_IF        ram_io[0x0f]

#define SYS_TIMER_CYCLES_4096HZ     1024
#define SYS_TIMER_CYCLES_262144HZ   16
#define SYS_TIMER_CYCLES_65536HZ    64
#define SYS_TIMER_CYCLES_16384HZ    256

#define SYS_TIMER_ENABLED (1<<2)

#define SYS_DIV_INTERVAL 256

#define SYS_DMA_LENGTH 160

#define INT_VBI       (1<<0)
#define INT_LCD       (1<<1)
#define INT_TIMER     (1<<2)
#define INT_SERIAL    (1<<3)
#define INT_JOYPAD    (1<<4)

extern uint8_t sys_carttype;
extern uint8_t sys_romsize;
extern uint8_t sys_rombank;
extern uint8_t sys_rambank;
extern uint8_t sys_extmem_en;
extern uint8_t sys_mbc1_s;

extern uint8_t sys_buttons_all;

extern uint8_t sys_dma_busy;
extern uint16_t sys_dma_source;
extern uint8_t sys_dma_counter;

extern int16_t sys_timer_interval;
extern int16_t sys_timer_interval_list[];
extern int16_t sys_timer_cycles;

extern uint8_t sys_ismbc1;
extern uint8_t sys_ismbc2;

void sys_init();

void sys_dma_cycles(int cycles);
void sys_cycles(int cycles);
void sys_handle_joypads();

void sys_interrupt_req(uint8_t index);
void sys_interrupt_clear(uint8_t index);

uint8_t sys_read_joypad();

#endif // SYS_H
