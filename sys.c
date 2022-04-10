#include "sys.h"

#include <stdlib.h>
#include <stdio.h>

#include "compat.h"

#include "mem.h"
#include "cpu.h"
#include "trace.h"
#include "video.h"
#include "audio.h"

#include "backends.h"

uint8_t sys_carttype;
uint8_t sys_mbc1_s;
uint8_t sys_romsize;
uint8_t sys_extmem_en;
uint8_t sys_mbc_bank_bits;

uint8_t sys_ismbc1 = 0;
uint8_t sys_ismbc2 = 0;

int16_t sys_div_cycles;

int16_t sys_timer_cycles;
int16_t sys_timer_interval;

uint16_t sys_dma_source;
uint8_t sys_dma_counter;
uint8_t sys_dma_busy;

uint8_t sys_running = 1;

int16_t sys_timer_interval_list[4] = {
    SYS_TIMER_CYCLES_4096HZ,
    SYS_TIMER_CYCLES_262144HZ,
    SYS_TIMER_CYCLES_65536HZ,
    SYS_TIMER_CYCLES_16384HZ,
};

uint8_t sys_buttons_old;
uint8_t sys_buttons_all;

input_backend_t *s_input_backend = NULL;

void sys_init(input_backend_t *backend) {

    s_input_backend = backend;

    if (s_input_backend->init)
        s_input_backend->init();

    sys_carttype = CT_ROMONLY;
    sys_mbc1_s = MBC1_2048_8;
    sys_romsize = 0;
    sys_extmem_en = 0;
    sys_mbc_bank_bits = 0;

    sys_div_cycles = SYS_DIV_INTERVAL;
    sys_timer_cycles = SYS_TIMER_CYCLES_4096HZ;

    sys_timer_interval = SYS_TIMER_CYCLES_4096HZ;

    sys_buttons_all = 0;
    sys_buttons_old = 0;

    sys_dma_source = 0;
    sys_dma_counter = 0;
    sys_dma_busy = 0;

}

void sys_deinit() {
    if (s_input_backend->deinit)
        s_input_backend->deinit();
}

void sys_run() {
    while (1) {

        while (!video_is_frame_done())
            cpu_step();

        video_ack_frame_done_and_draw();

        audio_render_frame();

        // Time the vsync, can be tricky

#if !defined(BENCHMARK)
        if (video_get_config()->use_audio_timing) {
            audio_wait_for_vsync();
        } else {
            sys_manual_vsync();
        }
#endif
        sys_handle_system();
        sys_handle_joypad();

        if (video_handle_events() == VIDEO_BACKEND_EXIT)
            break;

    }
}

void sys_dma_cycles(int cycles) {

    int32_t i;

    // Process dma cycles, one byte per cycle

    if (sys_dma_busy) {

        trace(TRACE_SYS, "DMA copying %x bytes from %x\n", cycles, sys_dma_source + sys_dma_counter);

        for (i = 0; i < cycles; i++) {
            oam[sys_dma_counter] = cpu_read8_force(sys_dma_source + sys_dma_counter);
            if (++sys_dma_counter == SYS_DMA_LENGTH)
                break;
        }

        if (sys_dma_counter == SYS_DMA_LENGTH) {
            // DMA has ended
            sys_dma_source = 0;
            sys_dma_counter = 0;
            sys_dma_busy = 0;
        }
    }

}

void sys_cycles(int cycles) {

    // Handle DIV counter which is always active

    cycles = cycles >> 2;   // 4 CPU cycles = 1 Machine Cycle

    sys_div_cycles -= cycles;
    if (sys_div_cycles <= 0) {
        // Increment Divider register
        SYS_DIV++;
        sys_div_cycles = SYS_DIV_INTERVAL + sys_div_cycles;
    }

    if (SYS_TIMER_CFG & SYS_TIMER_ENABLED)  {

        // Do timer shenanigans

        sys_timer_cycles -= cycles;

        trace(TRACE_SYS, "sys_timer_interval = %i, sys_timer_cycles = %i, sys_timer = %i, sys_timer_mod = %i\n", sys_timer_interval, sys_timer_cycles, SYS_TIMER, SYS_TIMER_MOD);

        if (sys_timer_cycles <= 0) {
            // Increment timer when the amount of cycles per "tick" have been reached

            sys_timer_cycles = sys_timer_interval + sys_timer_cycles; // Reset amount of cycles

            SYS_TIMER++;

            if (SYS_TIMER == 0) {
                // Timer overflowed
                SYS_TIMER = SYS_TIMER_MOD;
                sys_interrupt_req(INT_TIMER);

                trace(TRACE_SYS, "Requesting Timer Interrupt\n");
            }

        }


    }

}

uint8_t sys_read_joypad() {

    uint8_t result = 0x0F;

    // Handle DPAD if enabled
    if (SYS_JOYPAD & JOY_DPAD) {
        result &= (sys_buttons_all >> 4);
    }

    // Handle buttons if enabled
    if (SYS_JOYPAD & JOY_BUTTONS) {
        result &= (sys_buttons_all & 0x0F);
    }

    trace(TRACE_SYS, "Joypad status %02x, joy_int %02x, mem_ie %02x, buttons %02x\n", result, SYS_IF & INT_JOYPAD, ram_ie, sys_buttons_all);

    return result;
}

void sys_handle_system() {
    // Nothing here yet...
}

void sys_update_buttons() {
    sys_buttons_all = s_input_backend->get_buttons();
}


void sys_handle_joypad() {
    // Call backend function for this

    sys_buttons_old = sys_buttons_all;
    sys_update_buttons();

    // Interrupt on high-low transitions
    // refreshing my high school boolean maths... Optimized, equivalent old code below...

    if ((sys_buttons_old ^ sys_buttons_all) & sys_buttons_old)
        sys_interrupt_req(INT_JOYPAD);
    /*
    for (i = 0; i < 8; i++) {
        if (((sys_buttons_old >> i) & 0x01) && !(((sys_buttons_all >> i) & 0x01))) {
            // Req a joypad interupt
            sys_interrupt_req(INT_JOYPAD);
            break;
        }
    }
    */
}

void sys_manual_vsync() {
    // TODO
    // long old_timestamp_ms;
    // while ((s_last_timestamp_ms - old_timestamp_ms) > 16)) {
    //     old_timestamp_ms = s_last_timestamp_ms;
    //     s_last_timestamp_ms = get_current_timestamp_ms;
    // }

}
