#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include "mem.h"

#define AUDIO_NR10    ram_io[0x10]
#define AUDIO_NR11    ram_io[0x11]
#define AUDIO_NR12    ram_io[0x12]
#define AUDIO_NR13    ram_io[0x13]
#define AUDIO_NR14    ram_io[0x14]

#define AUDIO_NR21    ram_io[0x16]
#define AUDIO_NR22    ram_io[0x17]
#define AUDIO_NR23    ram_io[0x18]
#define AUDIO_NR24    ram_io[0x19]


#define AUDIO_NR30    ram_io[0x1a]
#define AUDIO_NR31    ram_io[0x1b]
#define AUDIO_NR32    ram_io[0x1c]
#define AUDIO_NR33    ram_io[0x1d]
#define AUDIO_NR34    ram_io[0x1e]

#define AUDIO_NR41    ram_io[0x20]
#define AUDIO_NR42    ram_io[0x21]
#define AUDIO_NR43    ram_io[0x22]
#define AUDIO_NR44    ram_io[0x23]

#define AUDIO_NR50    ram_io[0x24]
#define AUDIO_NR51    ram_io[0x25]
#define AUDIO_NR52    ram_io[0x26]

#define AUDIO_TRIGGER_BIT (1<<7)

#define AUDIO_BUFFER_SIZE 512
#define AUDIO_SAMPLE_RATE 48000
#define BYTES_PER_SAMPLE  2

#define AUDIO_ENV_ATTENUATE (1<<3)
#define AUDIO_ENV_SWEEP_N (0x07)

#define AUDIO_NOISE_MODE (1<<3)

#define AUDIO_TIMED (1<<6)

#define SAMPLE int16_t     // S16

struct audio_channel {
    uint8_t nr0,nr1,nr2,nr3,nr4;
};

extern struct audio_channel * audio_chans;
extern float audio_timer;
extern uint8_t *audio_wavedata;

void square(unsigned int i, SAMPLE *buffer);
void audio_handle_write(uint16_t addr, uint16_t data);
void audio_update_volume(int i);
void audio_sdl_init();
void audio_sdl_callback(void *udata, uint8_t *stream, int len);

#endif // AUDIO_H
