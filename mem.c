#include "mem.h"

/*
uint8_t rom1[0x4000];
uint8_t *rom2;
uint8_t vram[0x2000];
uint8_t *ram2;
uint8_t ram1[0x2000];

uint8_t ram_uio1[0x60]; // FEA0 - FEFF
uint8_t ram_io[0x4C];   // FF00 - FF4B
uint8_t ram_uio2[0x34]; // FE4C - FF7F
uint8_t ram_int[0x80];  // FF80 - FFFE

uint8_t ram_ie // FFFF

*/
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

uint8_t ram_ie;         // FFFF


int mem_init(uint8_t *file, int fsize) {

    romfile = file;

    if (fsize > 0x4000) fsize = 0x4000;

    memcpy(rom1, romfile, fsize);

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
        return &ram_io[addr - 0xFEA0];
    } else if   (addr < 0xFF80) {
        return &ram_uio2[addr - 0xFF4C];
    } else if   (addr < 0xFFFF) {
        return &ram_int[addr - 0xFF80];
    } else {
        return &ram_ie;
    }

    return NULL;

}

uint8_t cpu_read8(uint16_t addr) {
    if          (addr < 0x4000) {

        return rom1[addr];

    } else if   (addr < 0x8000) {

        return rom2[addr-0x4000];

    } else if   (addr < 0xA000) {
        // VRAM write 8000 - 9FFF
        return vram[addr - 0x8000];
    } else if   (addr < 0xC000) {
        // RAM 2 handler not implemented yet
        // addr - 0xA000
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
//            printf("JOYPAD_READ\n");
            return sys_read_joypad();
        }

        if (addr == MEM_LINE) {
            return video_line_num & 0xFF;
        }
        return ram_io[addr - 0xFF00];

    } else if   (addr < 0xFFFF) {
        if (addr == 0xFF81) {
            printf("drmario joy data: %02x\n", ram_int[0x01]);
            if (ram_int[0x01] != 0x00)  {
                printf("Joy_trigger\n");
//                cpu_verbose = 1;
            }
        }
        return ram_int[addr - 0xFF80];
    } else if   (addr == 0xFFFF) {
        return ram_ie;
    }

    return 0;
}


uint16_t cpu_read16(uint16_t addr) {

    if          (addr < 0x4000) {

        return *(uint16_t*) &rom1[addr];

    } else if   (addr < 0x8000) {

        return *(uint16_t*) &rom2[addr-0x4000];

    } else if   (addr < 0xA000) {
        // VRAM write 8000 - 9FFF
        return *(uint16_t*) &vram[addr - 0x8000];
    } else if   (addr < 0xC000) {
        // RAM 2 handler not implemented yet
        // addr - 0xA000
    } else if   (addr < 0xE000) {
        return *(uint16_t*) &ram1[addr-0xC000];
    } else if   (addr < 0xFE00) {
        return *(uint16_t*) &ram1[addr-0xE000];
    } else if   (addr < 0xFEA0) {
        return *(uint16_t*) &oam[addr-0xFE00];
    } else if   (addr < 0xFF00) {
        return *(uint16_t*) &ram_uio2[addr-0xFEA0];
    } else if   (addr < 0xFF80) {
        return *(uint16_t*) &ram_io[addr - 0xFF00];
    } else if   (addr < 0xFFFF) {
        return *(uint16_t*) &ram_int[addr - 0xFF80];
    } else if   (addr == 0xFFFF) {
        return ((uint16_t) ram_ie) << 8;
    }

    return 0;
}

void cpu_write8(uint16_t addr, uint8_t data) {

    if          (addr < 0x2000) {

    } else if   (addr < 0x4000) {
        if     ((sys_carttype == CT_MBC1)
            |   (sys_carttype == CT_MBC1RAM)
            |   (sys_carttype == CT_MBC1RAMBATT)) {

            // 0x2000 - 0x3FFF will select an appropriate ROM bank at 4000-7FFF

            if (data == 0x00) data = 0x01;

            uint32_t newaddr;

            if (sys_mbc1_s == MBC1_16_8) {
                // 16 MBit / 8 KiB  Mode
                data &= 0x1F;
                newaddr = data << 14;
            }

            rom2 = &romfile[newaddr];   // Set new bank window

        }
    } else if   (addr < 0x6000) {
        // MBC1 4_32 mode

        if     ((sys_carttype == CT_MBC1)
            |   (sys_carttype == CT_MBC1RAM)
            |   (sys_carttype == CT_MBC1RAMBATT)) {

            // 0x4000 - 0x5FFF will select an appropriate ROM bank at 4000-7FFF

            if (data == 0x00) data = 0x01;

            uint32_t newaddr;

            if (sys_mbc1_s == MBC1_4_32) {
                // 4 MBit / 32 KiB  Mode
                data &= 0x03;
                newaddr = data << 14;
            }

            rom2 = &romfile[newaddr];   // Set new bank window

        }

    } else if   (addr < 0x8000) {

        // Set MBC1 memory mode

        if     ((sys_carttype == CT_MBC1)
            |   (sys_carttype == CT_MBC1RAM)
            |   (sys_carttype == CT_MBC1RAMBATT)) {

            sys_mbc1_s = data & 0x01;

        }

    } else if   (addr < 0xA000) {
        // VRAM write 8000 - 9FFF
        vram[addr - 0x8000] = data;
    } else if   (addr < 0xC000) {
        // RAM 2 handler not implemented yet
        // addr - 0xA000
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
         *      JOYPAD
         */

/*        if (addr == MEM_JOYPAD) {
            sys_joypad_write(data);
        }
*/
        /*
         *      HANDLE TIMER STUFF
         */



        if (addr == MEM_TAC) {
            printf("TAC write: %02x\n", data);
            // If timer config reg has been written to, set config accordingly
            switch (data & 0x03) {
            case TIMER_4096:    sys_timer_speed = 1024; break;
            case TIMER_262144:  sys_timer_speed = 16;   break;
            case TIMER_65536:   sys_timer_speed = 64;   break;
            case TIMER_16384:   sys_timer_speed = 256;  break;
            }
            data = 0xF8 | (data & 0x07);
        }


        if (addr == 0xFF02 && data == 0x81) {
            printf("%c", ram_io[0x01]);
        }

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
