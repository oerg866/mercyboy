#include "sys.h"

uint8_t sys_carttype = CT_ROMONLY;
uint8_t sys_mbc1_s = MBC1_16_8;
uint8_t sys_romsize = 0;


int16_t sys_timer_cycles = 1024;
int16_t sys_timer_speed = 1024;

uint8_t sys_buttons_all = 0;

uint16_t sys_dma_source = 0;
uint8_t sys_dma_counter = 0;
uint8_t sys_dma_busy = 0;

#define sys_timer_int cpu_ints[2]
#define sys_joypad_int cpu_ints[4]

#define TIMER_ENABLE (1<<2)


void sys_dma_cycles(int cycles) {

    // Process dma cycles, one byte per cycle
    if (sys_dma_busy) {

#ifdef SYS_VERBOSE
        printf(">>>> DMA COPYING %x BYTES FROM %x\n", cycles, sys_dma_source + sys_dma_counter);
#endif
        for (int i = 0; i < cycles; i++) {
            oam[sys_dma_counter] = cpu_read8_force(sys_dma_source + sys_dma_counter);
            if (++sys_dma_counter == 160)
                break;
        }

        if (sys_dma_counter == 160) {
            // DMA has ended
            sys_dma_source = 0;
            sys_dma_counter = 0;
            sys_dma_busy = 0;
        }
    }

}

void sys_cycles(int cycles) {

    if (sys_timer_cfg & TIMER_ENABLE)  {

        // Do timer shenanigans

        sys_timer_cycles -= cycles;

#ifdef SYS_VERBOSE
        printf("sys_timer_speed = %i, sys_timer_cycles = %i, sys_timer = %i, sys_timer_mod = %i\n", sys_timer_speed, sys_timer_cycles, sys_timer, sys_timer_mod);
#endif

        if (sys_timer_cycles <= 0) {
            // Increment timer when the amount of cycles per "tick" have been reached

            sys_timer_cycles = sys_timer_speed + sys_timer_cycles; // Reset amount of cycles

            sys_timer++;

            if (sys_timer == 0) {
                // Timer overflowed
                sys_timer = sys_timer_mod;
                sys_timer_int = INT_PENDING; // Flag interrupt as pending



            } else {

                if (sys_timer_int == INT_SERVICED) sys_timer_int = INT_NONE;
            }

            sys_timer_cycles = sys_timer_speed;

        }


    }

}

uint8_t sys_read_joypad() {

    uint8_t result = 0x0F;



    // Handle DPAD if enabled
    if (sys_joypad & JOY_DPAD) {
        result &= (sys_buttons_all >> 4);
    }

    // Handle buttons if enabled
    if (sys_joypad & JOY_BUTTONS) {
        result &= (sys_buttons_all & 0x0F);
    }

#ifdef SYS_VERBOSE
    printf("Joypad status %02x, joy_int %02x, mem_ie %02x, buttons %02x\n", result, sys_joypad_int, ram_ie, sys_buttons_all);
#endif

    return result;
}

void sys_handle_joypads() {

    const uint8_t *state = SDL_GetKeyboardState(NULL);

    // 0xFF00
    /*
    Bit 7 - Not used
    Bit 6 - Not used
    Bit 5 - P15 Select Button Keys      (0=Select)
    Bit 4 - P14 Select Direction Keys   (0=Select)
    Bit 3 - P13 Input Down  or Start    (0=Pressed) (Read Only)
    Bit 2 - P12 Input Up    or Select   (0=Pressed) (Read Only)
    Bit 1 - P11 Input Left  or Button B (0=Pressed) (Read Only)
    Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
    */

    uint8_t sys_buttons_old = sys_buttons_all; // old joypad state for later

    sys_buttons_all = 0
            | ((~state[SDL_SCANCODE_DOWN] & 0x01)    << 7)
            | ((~state[SDL_SCANCODE_UP] & 0x01)      << 6)
            | ((~state[SDL_SCANCODE_LEFT] & 0x01)    << 5)
            | ((~state[SDL_SCANCODE_RIGHT] & 0x01)   << 4)
            | ((~state[SDL_SCANCODE_RETURN] & 0x01)  << 3)
            | ((~state[SDL_SCANCODE_SPACE] & 0x01)   << 2)
            | ((~state[SDL_SCANCODE_S] & 0x01)       << 1)
            | ((~state[SDL_SCANCODE_A] & 0x01)       << 0)
            ;

    // Interrupt on high-low transitions

    for (int i = 0; i < 8; i++) {
        if (((sys_buttons_old >> i) & 0x01) && !(((sys_buttons_all >> i) & 0x01))) {
            sys_joypad_int = INT_PENDING;
            break;
        }
    }

}
