#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "sys.h"
#include "audio.h"
#include "video.h"

uint8_t *romfile;
uint8_t *addonram;

uint8_t rom1[0x8000];
uint8_t *rom2;
uint8_t vram[0x2000];
uint8_t *ram2;
uint8_t ram1[0x2000];
uint8_t oam[0xA0];      // FE00 - FE9F
uint8_t ram_uio1[0x60]; // FEA0 - FEFF
uint8_t ram_io[0x4C];   // FFF0 - FF4B
uint8_t ram_uio2[0x34]; // FE4C - FF7F
uint8_t ram_int[0x80];  // FF80 - FFFE

uint8_t ram_ext[0x20000]; // Up to 128K

uint8_t ram_ie;         // FFFF

int mem_init(uint8_t *file, int fsize) {

    romfile = file;

    if (fsize > 0x4000) fsize = 0x4000;

    memcpy(rom1, romfile, fsize);

    ram2 = &ram_ext[0];
    rom2 = &romfile[0x4000];

    ram_io[0x05] = 0x00; // TIMA
    ram_io[0x06] = 0x00; // TMA
    ram_io[0x07] = 0x00; // TAC
    ram_io[0x10] = 0x80; // NR10
    ram_io[0x11] = 0xBF; // NR11
    ram_io[0x12] = 0xF3; // NR12
    ram_io[0x14] = 0xBF; // NR14
    ram_io[0x16] = 0x3F; // NR21
    ram_io[0x17] = 0x00; // NR22
    ram_io[0x19] = 0xBF; // NR24
    ram_io[0x1A] = 0x7F; // NR30
    ram_io[0x1B] = 0xFF; // NR31
    ram_io[0x1C] = 0x9F; // NR32
    ram_io[0x1E] = 0xBF; // NR33
    ram_io[0x20] = 0xFF; // NR41
    ram_io[0x21] = 0x00; // NR42
    ram_io[0x22] = 0x00; // NR43
    ram_io[0x23] = 0xBF; // NR30
    ram_io[0x24] = 0x77; // NR50
    ram_io[0x25] = 0xF3; // NR51
    ram_io[0x26] = 0xF1; // NR52
    ram_io[0x40] = 0x91; // LCDC
    ram_io[0x42] = 0x00; // SCY
    ram_io[0x43] = 0x00; // SCX
    ram_io[0x45] = 0x00; // LYC
    ram_io[0x47] = 0xFC; // BGP
    ram_io[0x48] = 0xFF; // OBP0
    ram_io[0x49] = 0xFF; // OBP1
    ram_io[0x4A] = 0x00; // WY
    ram_io[0x4B] = 0x00; // WX
    ram_ie = 0x00; // IE

    *flags = 0xB0;
    *pc = 0x100;
    regs8[REG_A] = 0x01;
    regs16[REG_BC] = bs(0x0013);
    regs16[REG_DE] = bs(0x00D8);
    regs16[REG_HL] = bs(0x014D);
    *sp = bs(0xFFFE);

    char title[0x0F];
    title[0x0E] = 0x00;
    memcpy(title, &romfile[0x134], 0x0C);


    printf("Game Name: %s\n", title);
    printf("Cartridge Type: ");

    sys_carttype = romfile[0x147];

    switch(sys_carttype) {
    case CT_ROMONLY:        printf("ROM ONLY\n"); break;
    case CT_MBC1:           printf("ROM + MBC1\n"); break;
    case CT_MBC1RAM:        printf("ROM + MBC1 + RAM\n"); break;
    case CT_MBC1RAMBATT:    printf("ROM + MBC1 + RAM + BATTERY\n"); break;
    case CT_MBC2:           printf("ROM + MBC2\n"); break;
    case CT_MBC2BATT:       printf("ROM + MBC2 + BATTERY\n"); break;
    default:                printf("Unknown: %x", sys_carttype);
    }

    printf("Reported ROM Size: ");

    // Figure out ROM size

    sys_romsize = romfile[0x148];

    int romsize = sys_romsize;
    if (romsize < 7) {
        romsize += 2;

        printf("%i banks (", romsize);

        printf("%i Bytes)\n", romsize << 14);


    }

    return 0;
}

uint8_t* mem_addr(uint16_t addr) {

    if          (addr < 0x4000) {
        return &rom1[addr];
    } else if   (addr < 0x8000) {
        return &rom2[addr - 0x4000];
    } else if   (addr < 0xA000) {
        return &vram[addr - 0x8000];
    } else if   (addr < 0xC000) {
        return &ram2[addr - 0xA000];
    } else if   (addr < 0xE000) {
        return &ram1[addr - 0xC000];
    } else if   (addr < 0xFE00) {
        return &ram1[addr - 0xE000];
    } else if   (addr < 0xFEA0) {
        return &oam[addr - 0xFE00];
    } else if   (addr < 0xFF00) {
        return &ram_uio1[addr-0xFEA0];
    } else if   (addr < 0xFF4C) {
        return &ram_io[addr - 0xFF00];
    } else if   (addr < 0xFF80) {
        return &ram_uio2[addr - 0xFF4C];
    } else if   (addr < 0xFFFF) {
        return &ram_int[addr - 0xFF80];
    } else {
        return &ram_ie;
    }

    return NULL;

}

uint8_t cpu_read8_force(uint16_t addr) {

    if          (addr < 0x4000) {

        return rom1[addr];

    } else if   (addr < 0x8000) {
        return rom2[addr-0x4000];
    } else if   (addr < 0xA000) {
        // VRAM write 8000 - 9FFF
        return vram[addr - 0x8000];
    } else if   (addr < 0xC000) {
        if (sys_extmem_en && (sys_ismbc1 || (sys_ismbc2 && (addr < 0xA1FF))))
            return ram2[addr-0xA000];
    } else if   (addr < 0xE000) {
        return ram1[addr-0xC000];
    } else if   (addr < 0xFE00) {
        return ram1[addr-0xE000];
    } else if   (addr < 0xFEA0) {
        return oam[addr-0xFE00];
    } else if   (addr < 0xFF00) {
        return ram_uio2[addr-0xFEA0];
    } else if   (addr < 0xFF80) {


        /*
         *  HANDLE JOYPAD
         */

        if (addr == MEM_JOYPAD) {
            return sys_read_joypad();
        }

        return ram_io[addr - 0xFF00];

    } else if   (addr < 0xFFFF) {
#ifdef SYS_VERBOSE
        if (addr == 0xFF81) {
            printf("drmario joy data: %02x\n", ram_int[0x01]);
            if (ram_int[0x01] != 0x00)  {
                printf("Joy_trigger\n");
            }
        }
#endif
        return ram_int[addr - 0xFF80];
    } else if   (addr == 0xFFFF) {
        return ram_ie;
    }

    return 0;

}

uint8_t cpu_read8(uint16_t addr) {

    // If OAM DMA is going on, ignore r/w to addresses below 0xFE00

    if ((sys_dma_busy) && (addr < 0xFE00)) {
        printf("!!!! WARNING: Ignored read from %x during DMA!\n", addr);
        return 0;
    }

    return cpu_read8_force(addr);

}

uint16_t cpu_read16(uint16_t addr) {

    // If OAM DMA is going on, ignore r/w to addresses below 0xFE00

    if ((sys_dma_busy) && (addr < 0xFE00)) {
        printf("!!!! WARNING: Ignored read from %x during DMA!\n", addr);
        return 0;
    }

    return cpu_read8_force(addr) | (cpu_read8_force(addr+1) << 8);
}

#define SYS_VERBOSE

void cpu_write8(uint16_t addr, uint8_t data) {

    // If OAM DMA is going on, ignore r/w to addresses below 0xFE00

    if ((sys_dma_busy) && (addr < 0xFE00)) {
        printf("!!!! WARNING: Ignored write to %x during DMA!\n", addr);
        return;
    }

    if          (addr < 0x2000) {

        if (sys_ismbc1 || (sys_ismbc2 && (~addr & 0x0100))) {
            // 0x0000 - 0x1FFF - RAM Enable
            sys_extmem_en = ((data & 0x0F) == 0x0A);   // xAh = enable, else disable
#ifdef SYS_VERBOSE
            printf("BANKING: MBC1 RAM Enable: %02x\n", sys_extmem_en);
#endif
        }

    } else if   (addr < 0x4000) {

        if (sys_ismbc1 || sys_ismbc2) {

            // 0x2000 - 0x3FFF ROM Bank Number

            if (sys_ismbc1) {
                // Selects lower 5 bits of the ROM bank number.
                if ((data & 0x1F) == 0) data++;     // 0x00 = 0x01, 0x20 = 0x21, ...)
                sys_rombank = (sys_rombank & (~0x1F)) | (data & 0x1F);
            } else if (sys_ismbc2) {
                sys_rombank = data;
            }

            uint32_t newaddr = sys_rombank << 14;

#ifdef SYS_VERBOSE
            printf("BANKING: New MBC ROM Address set: %04x (%02x)\n", newaddr, sys_rombank);
#endif
            rom2 = &romfile[newaddr];   // Set new bank window

        }
    } else if   (addr < 0x6000) {

        if (sys_ismbc1) {

            // 0x4000 - 0x5FFF RAM Bank Number *or* Upper bits of ROM bank
            // If we have ROM Banking mode, this selects upper two bits of ROM bank.

            uint32_t newaddr;

            if (sys_mbc1_s == MBC1_2048_8) {
                // 2048 KiB / 8 KiB Mode (ROM Banking), set upper bits of ROM bank
                if (data == 0x00) data = 0x01;
                data &= 0x03;
                sys_rombank = (sys_rombank & (~0x60)) | (data << 5);
                newaddr = sys_rombank << 14;

#ifdef SYS_VERBOSE
                printf("BANKING: New MBC1 ROM Address set: %04x (%02x)\n", newaddr, sys_rombank);
#endif

                rom2 = &romfile[newaddr];   // Set new bank window

            } else if (sys_mbc1_s == MBC1_512_32) {
                // 512 KiB / 32 KiB Mode (RAM Banking), set RAM bank
                sys_rambank = data & 0x03;

#ifdef SYS_VERBOSE
            printf("BANKING: New MBC1 RAM Address set: %04x\n", sys_rambank << 13);
#endif

                ram2 = &ram_ext[sys_rambank << 13];
            }

        }

    } else if   (addr < 0x8000) {

        // Set MBC1 memory mode

        if  (sys_ismbc1) {

            sys_mbc1_s = data & 0x01;

#ifdef SYS_VERBOSE
            printf("BANKING: MBC1 ROM/RAM Mode Set: %02x\n", sys_mbc1_s);
#endif
        }

    } else if   (addr < 0xA000) {
        // VRAM write 8000 - 9FFF
        vram[addr - 0x8000] = data;
    } else if   (addr < 0xC000) {
        // RAM 2, MBC2 only goes up to 0xA1FF
        if (sys_extmem_en && (sys_ismbc1 || (sys_ismbc2 && (addr < 0xA1FF))))
            ram2[addr-0xA000] = data;
    } else if   (addr < 0xE000) {
        ram1[addr-0xC000] = data;
    } else if   (addr < 0xFE00) {
        ram1[addr-0xE000] = data;
    } else if   (addr < 0xFEA0) {
        oam[addr-0xFE00] = data;
    } else if   (addr < 0xFF00) {
        ram_uio2[addr-0xFEA0] = data;

    } else if   (addr < 0xFF4C) {
        /*
         *      IO CONFIG REGION
         */


        /*
         *      SOUND
         */

        if ((addr >= 0xFF10) && (addr <= 0xFF26)) {
            audio_handle_write(addr, data);
        }

        /*
         *      HANDLE TIMER STUFF
         */

        if (addr == MEM_DMA) {
            // OAM DMA
            sys_dma_source = ((uint16_t) data) << 8;
            sys_dma_counter = 0;
            sys_dma_busy = 1;
        }

        if (addr == MEM_TAC) {

#ifdef SYS_VERBOSE
            printf("TAC write: %02x\n", data);
#endif

            // If timer config reg has been written to, set config accordingly

            sys_timer_interval = sys_timer_interval_list[data & 0x03];
            sys_timer_cycles = sys_timer_interval;
            data = 0xF8 | (data & 0x07);
        }


#ifdef SYS_VERBOSE
        if (addr == MEM_TMA)
            printf("TMA write: %02x\n", data);
#endif

        if (addr == MEM_LINE) {
            return;
        }

#ifdef DEBUG
        if (addr == 0xFF02 && data == 0x81)
            printf("%c", cpu_read8_force(0xFF01));
#endif

        // Palette registers require palettes to be updated

        if (addr == MEM_BGP)
            video_update_palette(PAL_OFFSET_BGP, data);

        if (addr == MEM_OBP0)
            video_update_palette(PAL_OFFSET_OBP0, data);

        if (addr == MEM_OBP1)
            video_update_palette(PAL_OFFSET_OBP1, data);


#ifdef VIDEO_VERBOSE
        if (addr == MEM_SCY) {
            printf("SCY Write: %02x\n", data);
        }

        if (addr == MEM_SCX) {
            printf("SCX Write: %02x\n", data);
        }
#endif

        ram_io[addr - 0xFF00] = data;

    } else if   (addr < 0xFF80) {
        // do nothing
    } else if   (addr < 0xFFFF) {
        ram_int[addr - 0xFF80] = data;
    } else if   (addr == 0xFFFF) {
        ram_ie = data;
    }

//    return NULL;

}

void cpu_write16(uint16_t addr, uint16_t data) {
    cpu_write8(addr, data & 0xFF);
    cpu_write8(addr+1, data >> 8);

}
