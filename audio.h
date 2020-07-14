#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include "mem.h"

#define AUDIO_NR10 ram_io[0x10]


#define AUDIO_NR50 ram_io[0x24]
#define AUDIO_NR51 ram_io[0x25]
#define AUDIO_NR52 ram_io[0x26]

#define AUDIO_BUFFER_SIZE 512
#define AUDIO_SAMPLE_RATE 48000
#define BYTES_PER_SAMPLE  2

#define AUDIO_ENV_ATTENUATE (1<<3)
#define AUDIO_ENV_SWEEP_N (0x07)

#define AUDIO_TIMED (1<<6)

struct audio_channel {
    uint8_t sweep;
    uint8_t duty;
    uint8_t env;
    uint8_t freq_l;
    uint8_t freq_h;
};

extern struct audio_channel * audio_chans;
extern float audio_timer;
extern uint8_t *audio_wavedata;

void audio_sdl_init();
void audio_sdl_callback(void *udata, uint8_t *stream, int len);

#endif // AUDIO_H
