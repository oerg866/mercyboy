#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include "mem.h"

///////////////////////////////
// Audio backends

#ifdef AUDIO_SDL2
#include "audio_sdl2.h"
#endif

///////////////////////////////

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

#define SAMPLE_D125     0x01
#define SAMPLE_D25      0x81
#define SAMPLE_D50      0x87
#define SAMPLE_D75      0x7E

#define DUTY_125        0x00
#define DUTY_25         0x40
#define DUTY_50         0x80
#define DUTY_75         0xC0

#define AUDIO_DUTY_CYCLE ((data & 0xC0) >> 6)
#define AUDIO_LENGTH64 (data & 0x3F)
#define AUDIO_VOLUME (data >> 4)
#define AUDIO_SOUND_ENABLED (AUDIO_NR52 & 0x89)
#define AUDIO_ENVELOPE (data & 0x07)
#define AUDIO_ENVELOPE_AMPLIFY (1<<3)
#define AUDIO_CONSECUTIVE (1<<6)
#define AUDIO_TRIGGER_BIT (1<<7)
#define AUDIO_ENV_ATTENUATE (1<<3)
#define AUDIO_ENV_SWEEP_N (0x07)
#define AUDIO_NOISE_MODE (1<<3)
#define AUDIO_TIMED (1<<6)
#define AUDIO_SWEEP_DIRECTION (1<<3)

#define MASTER_CLOCK 4194304

#ifdef AUDIO_NONE
#define SAMPLE int16_t
#define SAMPLE_MAX INT16_MAX
#endif

typedef int16_t SAMPLE;         // Internally we process audio as PCM S16 LE, the backend must handle conversion if necessary
#define SAMPLE_MAX INT16_MAX

struct audio_channel {
    uint8_t nr0,nr1,nr2,nr3,nr4;
};

extern uint32_t audio_sample_rate;
extern uint32_t audio_amount_channels;
extern volatile float audio_timer;

// I/O RW functions

void audio_handle_write(uint16_t addr, uint16_t data);
uint8_t audio_handle_read(uint16_t addr);

// Audio chunk generator functions

void square(unsigned int i, SAMPLE *buffer);
void noise(SAMPLE *buffer);
void waveform(SAMPLE *buffer);

// Channel operations

void audio_set_noise_frequency(uint8_t data);

void audio_disable_channel(int i);
void audio_enable_channel(int i);

void audio_update_waveform_data();
void audio_update_volume(int i);

extern void audio_process_chunk(SAMPLE *stream, int len);

// Init & deinit functions

void audio_init();
void audio_backend_init();
void audio_deinit();
void audio_backend_deinit();

void audio_generate_luts();

#endif // AUDIO_H
