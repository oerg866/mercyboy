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

int32_t sys_div_cycles;

int32_t sys_timer_cycles;
int32_t sys_timer_interval;

uint16_t sys_dma_source;
uint8_t sys_dma_counter;
uint8_t sys_dma_busy;

uint8_t sys_running = 1;

int32_t sys_timer_interval_list[4] = {
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

static inline void sys_cycles_dma(int32_t cycles) {
    int i;

    // Process dma cycles, one byte per cycle
    if (sys_dma_busy) {

        trace(TRACE_SYS, "DMA copying %x bytes from %x\n", cycles, sys_dma_source + sys_dma_counter);

        for (i = 0; i < cycles; ++i){
            oam[sys_dma_counter] = cpu_read8(sys_dma_source + sys_dma_counter);
            sys_dma_counter++;
            if (sys_dma_counter == SYS_DMA_LENGTH) {
                // DMA has ended
                sys_dma_source = 0;
                sys_dma_counter = 0;
                sys_dma_busy = 0;
                break;
            }
        }
    }

}

void sys_cycles(int32_t cycles) {
    // Process DIV / TIMER / DMA cycles, slightly faster version for small cycle numbers.

    sys_cycles_dma(cycles << 2);

    // Handle DIV counter which is always active

    sys_div_cycles -= cycles;
    if (sys_div_cycles <= 0) {
        // Increment Divider register
        SYS_DIV++;
        sys_div_cycles = SYS_DIV_INTERVAL + sys_div_cycles;
    }
    // Do timer shenanigans

    if (!(SYS_TIMER_CFG & SYS_TIMER_ENABLED))
        return;

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

void sys_cycles_idle(int32_t cycles) {
    // Slow cycles routine for large cycle numbers (for idling in HALT state).
    int div_cycles_elapsed = SYS_DIV_INTERVAL - sys_div_cycles;
    int timer_cycles_elapsed;
    int timer_counts;
    int timer_counts_per_division;


    // Step 1: DMA cycles
    sys_cycles_dma(cycles << 2);


    // Step 2: DIV
    sys_div_cycles = SYS_DIV_INTERVAL - ((div_cycles_elapsed + cycles) % SYS_DIV_INTERVAL);
    SYS_DIV += (div_cycles_elapsed + cycles) / SYS_DIV_INTERVAL;

    if (!(SYS_TIMER_CFG & SYS_TIMER_ENABLED))
        return;

    // Step 3: TIMER

    timer_cycles_elapsed = sys_timer_interval - sys_timer_cycles;
    timer_counts = (timer_cycles_elapsed + cycles) / sys_timer_interval;
    timer_counts_per_division = 256 - (int) SYS_TIMER_MOD;

    if (((int) SYS_TIMER + timer_counts) > 255) {
        sys_interrupt_req(INT_TIMER);
    }

    // Figure out final TIMER & timer_cycles value, keep in mind that the modulo for the timer is the other way around
    // so we have to consider what the value would be after wrapping back to the TIMER_MOD value, not to 0!!
    SYS_TIMER = (((SYS_TIMER - SYS_TIMER_MOD) + timer_counts) % timer_counts_per_division) + SYS_TIMER_MOD;
    sys_timer_cycles = sys_timer_interval - ((timer_cycles_elapsed + cycles) % sys_timer_interval);
}


int32_t sys_get_idle_cycle_count() {
    // returns maximum amount of cycles the machine can idle in one go before a timer inerrupt occurs

    // Find how many cycles left until the next timer interrupt
    // time_to_next_timer_increase = sys_timer_cycles;
    // if SYS_TIMER = 0xFF then that's the max
    // else it's sys_timer_cycles + (0xFF - SYS_TIMER) * sys_timer_interval

    if (SYS_TIMER_CFG & SYS_TIMER_ENABLED)
        return sys_timer_cycles + (0xFF - SYS_TIMER) * sys_timer_interval;

    return INT32_MAX;
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
    sys_buttons_all = s_input_backend->get_buttons();

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
