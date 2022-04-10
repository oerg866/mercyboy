#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "sys.h"
#include "audio.h"
#include "video.h"
#include "trace.h"

uint8_t *romfile;
uint8_t *addonram;

uint32_t romsize;
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

uint8_t ram_mbc_bank_bits;

uint16_t ram_extram_mask;

void mem_init_float(float* dest, int count) {
    memset(dest, 0, sizeof(float) * count);
}
void mem_init_uint8(uint8_t* dest, int count) {
    memset(dest, 0, count);
}

int mem_init(uint8_t *file, int fsize) {

    char title[0x0F];

	romfile = file;
    romsize = fsize;

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

    regs8[REG_F] = 0xB0;
    regs16[REG_PC] = 0x100;
    regs8[REG_A] = 0x01;
    regs16[REG_BC] = 0x0013;
    regs16[REG_DE] = 0x00D8;
    regs16[REG_HL] = 0x014D;
    regs16[REG_SP] = 0xFFFE;

    title[0x0E] = 0x00;
    memcpy(title, &romfile[0x134], 0x0C);


    print_msg("Game Name: %s\n", title);
    print_msg("Cartridge Type: ");

    sys_carttype = romfile[0x147];

    switch(sys_carttype) {
    case CT_ROMONLY:        print_msg("ROM ONLY\n"); break;
    case CT_MBC1:           print_msg("ROM + MBC1\n"); break;
    case CT_MBC1RAM:        print_msg("ROM + MBC1 + RAM\n"); break;
    case CT_MBC1RAMBATT:    print_msg("ROM + MBC1 + RAM + BATTERY\n"); break;
    case CT_MBC2:           print_msg("ROM + MBC2\n"); break;
    case CT_MBC2BATT:       print_msg("ROM + MBC2 + BATTERY\n"); break;
    default:                print_msg("Unknown: %x", sys_carttype);
    }

    // init global variables that indicate MBC presence

    sys_ismbc1  = (sys_carttype == CT_MBC1)
                | (sys_carttype == CT_MBC1RAM)
                | (sys_carttype == CT_MBC1RAMBATT);
    sys_ismbc2  = (sys_carttype == CT_MBC2)
                | (sys_carttype == CT_MBC2BATT);

    print_msg("Reported ROM Size: ");

    // Figure out ROM size

    sys_romsize = romfile[0x148];

    if (sys_romsize < 7) {
        print_msg("%i banks (", (sys_romsize + 2));
        print_msg("%i Bytes)\n", (sys_romsize + 2) << 14);
    }

    ram_mbc_bank_bits = 0;

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

void mem_update_banks_mbc1() {
    // Update RAM/ROM bank(s/pointers) based on modes

    uint32_t tmp_bank = (uint32_t) sys_mbc_bank_bits;
    uint32_t romaddr;

    // 00, 10, 20, 30 select bank 01, 11, 21, 31
    if ((tmp_bank & 0x1F) == 0x00)
        ++tmp_bank;

    // 512KB 0x01-0x1F / 8KB RAM banks mode
    if (sys_mbc1_s == MBC1_512_32) {
        ram2 = &ram_ext[(tmp_bank >> 5) << 13];
        tmp_bank &= 0x1F;
    } else {
        // 2MB / 8KB RAM, reset RAM bank to 0.
        ram2 = &ram_ext[0];
    }

	romaddr = tmp_bank << 14;

    trace(TRACE_MBC, "New MBC ROM Address set: %08x (%02x), max %08x \n", romaddr, tmp_bank, romsize - 0x4000);

    rom2 = &romfile[romaddr];   // Set new bank window

    if (romaddr > (romsize - 0x4000)) {
        print_msg("MBC bank OUTSIDE ROM area %08x (max. %08x)! Forcing crash...", romaddr, romsize - 0x4000);
        memset(rom1, 0xff, 0x8000);
        rom2 = rom1;
    }

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
        // MBC ext RAM, quit if ram disabled
        // OR address for MBC2 (only up to a1ff) is impossible.
        if (!sys_extmem_en || (sys_ismbc2 && (addr > 0xA1FF)))
            return 0;
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

        if        (addr == MEM_JOYPAD) {
            return sys_read_joypad();
        } else if (addr == MEM_LINE) {
            return video_get_line();
        }

        return ram_io[addr - 0xFF00];

    } else if   (addr < 0xFFFF) {
        return ram_int[addr - 0xFF80];
    } else if   (addr == 0xFFFF) {
        return ram_ie;
    }

    return 0;

}

uint8_t cpu_read8(uint16_t addr) {

    // If OAM DMA is going on, ignore r/w to addresses below 0xFE00

    if ((sys_dma_busy) && (addr < 0xFE00)) {
        trace(TRACE_ALL, "!!!! WARNING: Ignored read from %x during DMA!\n", addr);
        return 0;
    }

    return cpu_read8_force(addr);

}

int8_t cpu_read8_signed(uint16_t addr) {
    // Just a copy of read8 but returns signed int... makes cpu opcode code a bit cleaner
    if ((sys_dma_busy) && (addr < 0xFE00)) {
        trace(TRACE_ALL, "!!!! WARNING: Ignored read from %x during DMA!\n", addr);
        return 0;
    }
    return (int8_t) cpu_read8_force(addr);
}

uint16_t cpu_read16(uint16_t addr) {

    // If OAM DMA is going on, ignore r/w to addresses below 0xFE00

    if ((sys_dma_busy) && (addr < 0xFE00)) {
        trace(TRACE_ALL, "!!!! WARNING: Ignored read from %x during DMA!\n", addr);
        return 0;
    }

    return cpu_read8_force(addr) | (cpu_read8_force(addr+1) << 8);
}

void cpu_write8(uint16_t addr, uint8_t data) {

    // If OAM DMA is going on, ignore r/w to addresses below 0xFE00

    if ((sys_dma_busy) && (addr < 0xFE00)) {
        print_msg("!!!! WARNING: Ignored write to %x during DMA!\n", addr);
        return;
    }

    if          (addr < 0x2000) {

        if (sys_ismbc1 || (sys_ismbc2 && (~addr & 0x0100))) {
            // 0x0000 - 0x1FFF - RAM Enable
            sys_extmem_en = ((data & 0x0F) == 0x0A);   // xAh = enable, else disable

            trace(TRACE_MBC,"MBC1 RAM Enable: %02x\n", sys_extmem_en);
        }

    } else if   (addr < 0x4000) {

        if (sys_ismbc1 || sys_ismbc2) {

            // 0x2000 - 0x3FFF ROM Bank Number

            if (sys_ismbc1) {
                // Selects lower 5 bits of the ROM bank number.
                sys_mbc_bank_bits = (sys_mbc_bank_bits & (~0x1F)) | (data & 0x1F);
            } else if (sys_ismbc2) {
                sys_mbc_bank_bits = data;
            }

            mem_update_banks_mbc1();

        }
    } else if   (addr < 0x6000) {

        if (sys_ismbc1) {
            // 0x4000 - 0x5FFF RAM Bank Number *or* Upper bits of ROM bank
            // If we have ROM Banking mode, this selects upper two bits of ROM bank.

            sys_mbc_bank_bits = (sys_mbc_bank_bits & (~0x60)) | (data << 5);
            mem_update_banks_mbc1();
        }

    } else if   (addr < 0x8000) {

        // Set MBC1 memory mode

        if  (sys_ismbc1) {
            sys_mbc1_s = data & 0x01;
            trace(TRACE_MBC, "MBC1 ROM/RAM Mode Set: %02x\n", sys_mbc1_s);
        }

    } else if   (addr < 0xA000) {
        // VRAM write 8000 - 9FFF
        vram[addr - 0x8000] = data;
    } else if   (addr < 0xC000) {
        // RAM 2, MBC2 only goes up to 0xA1FF
        if (!sys_extmem_en || (sys_ismbc2 && (addr > 0xA1FF)))
                return;
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
            return;
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

            trace(TRACE_SYS, "TAC write: %02x\n", data);

            // If timer config reg has been written to, set config accordingly

            sys_timer_interval = sys_timer_interval_list[data & 0x03];
            sys_timer_cycles = sys_timer_interval;
            data = 0xF8 | (data & 0x07);
        }


        if (addr == MEM_TMA)
            trace(TRACE_SYS, "TMA write: %02x\n", data);

        if (addr == MEM_LINE) {
            return;
        }

        if (addr == MEM_LCDC) {
            if (!(data & LCDC_LCDEN)) {
                VID_LY = 0x00;
            } else if (!(VID_LCDC & LCDC_LCDEN)) {
                video_reset_lcd();
            }
        }

        // Palette registers require palettes to be updated

        if (addr == MEM_BGP)
            video_update_palette(PAL_OFFSET_BGP, data);

        if (addr == MEM_OBP0)
            video_update_palette(PAL_OFFSET_OBP0, data);

        if (addr == MEM_OBP1)
            video_update_palette(PAL_OFFSET_OBP1, data);

#ifdef DEBUG
        if (addr == 0xFF02 && data == 0x81)
            print_msg("%c", cpu_read8_force(0xFF01));

        if (addr == MEM_SCY)
            trace(TRACE_SYS, "SCY Write: %02x\n", data);

        if (addr == MEM_SCX)
            trace(TRACE_SYS, "SCX Write: %02x\n", data);
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
