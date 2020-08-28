#include "audio.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "mem.h"
#include "sys.h"
#include "trace.h"

// Pointer to I/O audio region in memory. This is so that the code is easier to read when accessing registers all over the place.
struct audio_channel* audio_chans = (struct audio_channel*) &AUDIO_NR10;

// Pointer to arbitrary waveform data for waveform channel.
static uint8_t * audio_wavedata = &ram_io[0x30];

// Sweep timer variables.
static float audio_sweep_cycle = 0;
static uint8_t audio_sweep_shift = 0;
static float audio_sweep_counter = 0;
static uint8_t audio_sweep_enabled = 0;
static float audio_sweep_old = 0;
static float audio_sweep_frequency = 0;

// Predefined array of samples for each square wave duty cycle type.
const  uint8_t audio_square_waves[4] = {SAMPLE_D125,SAMPLE_D25,SAMPLE_D50,SAMPLE_D75};

volatile float audio_timer = 0.0;                   // Constant audio timer. Globally available variable (so that video can be timed with it for example.

static float audio_256hz_cycle;                     // Cycle length for 256hz with respect to current sample rate
static float audio_64hz_cycle;                      // Cycle length for 64hz with respect to current sample rate

static float audio_256hz_timer = 0.0;               // Timer counter for 256hz clock
static float audio_64hz_timer = 0.0;                // Timer counter for 64hz clock

const  float audio_divider[4] = {131072.0, 131072.0, 65536.0, 0.0};   // Frequency dividers for each channel.
                                                                      // Note that Noise has very different frequency calculation so it's a dummy value (0.0)

static float audio_counter[4] = {0.0, 0.0, 0.0, 0.0};           // Note frequency cycle counter for each channel
static float audio_envelope_count[4] = {0.0, 0.0, 0.0, 0.0};    // Envelope cycle counter

static float audio_cycle[4] = {0.0, 0.0, 0.0, 0.0};             // Note frequency cycle length
static float audio_envelope_cycle[4] = {0.0, 0.0, 0.0, 0.0};    // Envelope cycle length

static uint8_t audio_length[4] = {0,0,0,0};         // Length values for all channels
static uint8_t audio_playing[4] = {0,0,0,0};        // Playing Y/N flags for all channels

static uint8_t audio_volume[4] = {0,0,0,0};         // Current volume as updated by either initial volume or envelope.
static uint8_t audio_master_volume[2] = {0,0};      // Master volume for L and R channels.
static uint8_t audio_sample[4] = {0,0,0,0};         // Current type of square wave output. (One entry of audio_square_waves gets put here)

static uint8_t audio_noise_lut7[128];               // 7-stage LFSR LUT
static uint8_t audio_noise_lut15[32768];            // 15-stage LFSR LUT

static uint16_t audio_noise_idx = 0;                // index of current entry in LFSR LUT
static uint8_t audio_waveform_idx = 0;              // index of current sample in waveform data

static SAMPLE audio_waveform_output[2] = {0,0};
static SAMPLE audio_output_l[4] = {0,0,0,0};        // +1 state output in SAMPLE type form for L channel (Channel volume and Master volume respected)
static SAMPLE audio_output_r[4] = {0,0,0,0};        // +1 state output in SAMPLE type form for L channel (Channel volume and Master volume respected)


/************************************************************************************
 *                                AUDIO I/O FUNCTIONS                               *
 ************************************************************************************/

uint8_t audio_handle_read(uint16_t addr) {
    // Called by memory R/W handler
    // Handle reads from Audio I/O region.

    uint8_t data = ram_io[addr - 0xFF10];

    uint8_t cidx = (addr - 0xFF10) / 5; // 5 registers per channel, one dummy
    trace(TRACE_AUDIO, "Handle read, addr %04x, cidx %d, data %02x\n", addr, cidx, data);

    switch(addr) {

    case MEM_NR14:
    case MEM_NR24:
    case MEM_NR34:
    case MEM_NR44:
        return data & (1<<6);   // only bit 6 can be read;

    case MEM_NR30:
        return data & (1<<7);   // only bit 7 can be read

    case MEM_NR32:
        return data & (0x03<<5);   // only bits 6-5 can be read

    default:
        return data;
    }
}


void audio_handle_write(uint16_t addr, uint16_t data) {
    // Called by memory R/W handler
    // Handle writes to Audio I/O region.

    uint8_t cidx = (addr - 0xFF10) / 5; // 5 registers per channel, one dummy
    struct audio_channel * chan = &audio_chans[cidx];

    trace(TRACE_AUDIO, "Handle write, addr %04x, cidx %d, data %02x\n", addr, cidx, data);

    switch(addr) {

    // SWEEP register

    case MEM_NR10:
        audio_sweep_enabled = ((data >> 4) & 0x07);
        audio_sweep_cycle = ((float) audio_sample_rate * (float) ((data >> 4) & 0x07)) / 128.0;
        audio_sweep_shift = data & 0x07;
        break;

    // DUTY CYCLES

    case MEM_NR11:
    case MEM_NR21:
        audio_sample[cidx] = audio_square_waves[AUDIO_DUTY_CYCLE];
        audio_length[cidx] = 64 - AUDIO_LENGTH64;
        break;

    case MEM_NR41:
        audio_length[cidx] = 64 - AUDIO_LENGTH64;
        break;

    case MEM_NR31:
        // Noise channel note length can be up to 256 ticks
        audio_length[cidx] = 256 - data;
        break;



    // SET ENVELOPE

    case MEM_NR12:
    case MEM_NR22:
    case MEM_NR42:
        audio_envelope_cycle[cidx] = (float) (AUDIO_ENVELOPE);

        trace(TRACE_AUDIO, "CH %d SET envelope cycle %f, amplify mode %01x\n", cidx, audio_envelope_cycle[cidx], data & AUDIO_ENVELOPE_AMPLIFY);

        break;

    // WAVEFORM VOLUME

    case MEM_NR32:
        data &= (0x03 << 5);
        ram_io[addr-0xFF00] = data;
        break;

    case MEM_NR30:  // Channel 3 (waveform)
        data &= 0x80;
        ram_io[addr-0xFF00] = data;

        // If ON bit is reset then turn off the channel.
        if (!data)
            audio_disable_channel(cidx);

        break;

    // FREQUENCY CHANGES & NOTE TRIGGER

    case MEM_NR43:  // Channel 4 (Noise)
        ram_io[addr-0xFF00] = data;
        audio_set_noise_frequency(data);

        trace(TRACE_AUDIO, "Note on on channel %d:, cycle = %f", cidx, audio_cycle[cidx]);

        break;

    case MEM_NR13:  // Channel 1 (Square)
    case MEM_NR14:
    case MEM_NR23:  // Channel 2 (Square)
    case MEM_NR24:
    case MEM_NR33:  // Channel 3 (Waveform)
    case MEM_NR34:
    case MEM_NR44:  // Channel 4 (Noise)

        ram_io[addr-0xFF00] = data;

        if (cidx == 0)                  // Channel 1 can sweep, we need to save the frequency
            audio_sweep_frequency = (float) (((chan->nr4 & 0x07) << 8) | chan->nr3);

        if (cidx <= 1)
            audio_cycle[cidx] = ((float) audio_sample_rate / (audio_divider[cidx] / (2048.0 - (float) (((chan->nr4 & 0x07) << 8) | chan->nr3)))) / 8.0;
        else if (cidx == 2)
            audio_cycle[cidx] = ((float) audio_sample_rate / (audio_divider[cidx] / (2048.0 - (float) (((chan->nr4 & 0x07) << 8) | chan->nr3)))) / 32.0;

        if (chan->nr4 & AUDIO_TRIGGER_BIT) {
            audio_counter[cidx] = 0.0; // NOTE ON counter reset

            // Enable channel
            audio_enable_channel(cidx);

            // Trigger means load the volume from NR2 and start playing, except for waveform channel
            if (cidx != 2)
                audio_volume[cidx] = chan->nr2 >> 4;

            // If we're on the waveform channel we do have to reset our index. And set max volume
            if (cidx == 2) {
                audio_waveform_idx = 0;
                audio_volume[cidx] = 0x0f;
                audio_update_waveform_data();
            }

            // If we're on the noise channel we do have to reset our index.
            if (cidx == 3)
                audio_noise_idx = 0;


            audio_update_volume(cidx);
            chan->nr4 &= ~AUDIO_TRIGGER_BIT;  // Delete flag so we dont keep triggering

            trace(TRACE_AUDIO, "Note on on channel %d:, cycle = %f, freq: %04x\n", cidx, audio_cycle[cidx], (((chan->nr4 & 0x07) << 8) | chan->nr3));


        }
        return;

    case MEM_NR50:  // Global stereo volume control
        ram_io[addr-0xFF00] = data;
        audio_master_volume[1] = (data & 0x70) >> 4;
        audio_master_volume[0] = (data & 0x07);
        audio_update_volume(0);
        audio_update_volume(1);
        audio_update_volume(2);
        audio_update_volume(3);
        return;
    case MEM_NR52:  // Sound on/off, bit 3-0 are READ ONLY
        ram_io[addr-0xFF00] = (data & 0xF0) | (ram_io[addr-0xFF00] & 0x0F);
        return;
    }
    ram_io[addr-0xFF00] = data;
}


/************************************************************************************
 *                             CHANNEL UPDATE FUNCTIONS                             *
 ************************************************************************************/


inline void audio_disable_channel (int i) {
    // Disable a channel (Disable playing flag in emulator + disable in gameboy register)

    trace(TRACE_AUDIO, "Disabling channel %d\n", i);

    audio_playing[i] = 0;
    AUDIO_NR52 &= ~(1 << i);
}

inline void audio_enable_channel (int i) {
    // Enable a channel (Enable playing flag in emulator + enable in gameboy register)

    trace(TRACE_AUDIO, "Enabling channel %d\n", i);

    audio_playing[i] = 1;
    AUDIO_NR52 |= (1 << i);
}

void audio_set_noise_frequency(uint8_t data) {

    // Calculate the frequency for the noise channel and set cycle for timing

    uint8_t shiftclock = data>>4;
    float dividingratio = (float) MASTER_CLOCK / 8.0;

    // Calculate dividing ratio

    switch (data & 0x07) {
    case 0x00:
        dividingratio *= 2; break;
    case 0x01:
        dividingratio *= 1; break;
    case 0x02:
        dividingratio /= 2; break;
    case 0x03:
        dividingratio /= 3; break;
    case 0x04:
        dividingratio /= 4; break;
    case 0x05:
        dividingratio /= 5; break;
    case 0x06:
        dividingratio /= 6; break;
    case 0x07:
        dividingratio /= 7; break;
    }

    // Calculate cycle

    audio_cycle[3] = (float) audio_sample_rate / (dividingratio / (float) (2 << (shiftclock)));

}

inline void audio_update_volume(int i) {
                     // max 0x0f          max 0x07
    audio_output_l[i] = audio_volume[i] * audio_master_volume[0] * ((SAMPLE_MAX/4) / (0x07*0x0f));   // L Update
    audio_output_r[i] = audio_volume[i] * audio_master_volume[1] * ((SAMPLE_MAX/4) / (0x07*0x0f));   // R Update

    if (i==2) {
        audio_update_waveform_data();   // If we're updating volume for the waveform channel we must also update the waveform data
    }

    trace(TRACE_AUDIO, "CH %d - Volume update. VOL: %02x, MVOL_L: %02x, MVOL_L: %02x, OUT_L %04x, OUT_R %04x\n", i, audio_volume[i], audio_master_volume[0], audio_master_volume[1], audio_output_l[i], audio_output_r[i]);


}

inline void audio_update_waveform_data() {

    // Update waveform data when requested

    if ((AUDIO_NR32 >> 5)) {

        // If we are not muted

        SAMPLE tmp = audio_wavedata[audio_waveform_idx>>1];

        if (audio_waveform_idx & 1) {   // If LSB = 1 then we have to shift to the right by 4 additional bits.
            audio_waveform_output[0] = ((((tmp & 0x0F) - 8) >> ((AUDIO_NR32 >>5) - 1)) * (audio_output_l[2] / 0x0F)); //
            audio_waveform_output[1] = ((((tmp & 0x0F) - 8) >> ((AUDIO_NR32 >>5) - 1)) * (audio_output_r[2] / 0x0F)); //
        } else {                        // if not, we AND the lower 4 bits.
            audio_waveform_output[0] = ((((tmp >>   4) - 8) >> ((AUDIO_NR32 >>5) - 1)) * (audio_output_l[2] / 0x0F)); //
            audio_waveform_output[1] = ((((tmp >>   4) - 8) >> ((AUDIO_NR32 >>5) - 1)) * (audio_output_r[2] / 0x0F)); //
        }

    } else {

        // We are muted so we just null the output.
        audio_waveform_output[0] = 0;
        audio_waveform_output[1] = 0;
    }
}


/************************************************************************************
 *                                   TIMER FUNCTIONS                                *
 ************************************************************************************/

void audio_sweep_timer() {
    audio_sweep_counter += 1.0;
    if (audio_sweep_counter >= audio_sweep_cycle) {
        audio_sweep_counter -= audio_sweep_cycle;

        if (audio_sweep_enabled && audio_playing[0] && (audio_sweep_shift != 0)) {
            if (AUDIO_NR10 & AUDIO_SWEEP_DIRECTION) {
                audio_sweep_frequency = audio_sweep_frequency - audio_sweep_frequency / pow(2, audio_sweep_shift);
            } else {
                audio_sweep_frequency = audio_sweep_frequency + audio_sweep_frequency / pow(2, audio_sweep_shift);
            }

            if (audio_sweep_frequency > 2047.0) {   // If frequency is > 2047 then the channel gets disabled, otherwise write new cycle
                audio_disable_channel(0);
            } else {
                audio_cycle[0] = ((float) audio_sample_rate / (audio_divider[0] / (2048.0 - audio_sweep_frequency))) / 8.0;
            }
        }

    }
}

void audio_envelope_timer() {
    audio_64hz_timer += 1.0;

    if (audio_64hz_timer >= audio_64hz_cycle) {

        audio_64hz_timer -= audio_64hz_cycle;

        for (int i = 0; i < 4; i++) {

            // Don't do envelopes if the step is 0 or we're processing channel 3 (waveform)

            if ((i != 2) && (audio_envelope_cycle[i] != 0.0)) {
                audio_envelope_count[i] += 1.0;
                // If a cycle for this envelope has been reached, reset it and deduct volume.
                if (audio_envelope_count[i] >= audio_envelope_cycle[i]) {
                    audio_envelope_count[i] -= audio_envelope_cycle[i];
                    if (audio_chans[i].nr2 & AUDIO_ENVELOPE_AMPLIFY) {

                        audio_volume[i] = (audio_volume[i] + 1) & 0x0f;

#ifdef AUDIO_VERBOSE
                        printf("AUDIO: CH %d - increasing volume %02x\n", i, audio_volume[i]);
#endif
                    }
                    else {

                        if (audio_volume[i])
                            audio_volume[i]--;
                        else {
                            audio_disable_channel(i);
                        }

#ifdef AUDIO_VERBOSE
//                        printf("AUDIO: CH %d - decreasing volume %02x\n", i, audio_volume[i]);
#endif
                    }
                }
                audio_update_volume(i);
            }
        }

    }

}

void audio_length_timer() {
    audio_256hz_timer += 1.0;
    if (audio_256hz_timer >= audio_256hz_cycle) {
        // 256 Hz Timer triggered, reset it and deduct lengths.
        audio_256hz_timer -= audio_256hz_cycle;

        // Process lengths for each channel

        for (int i = 0; i < 4; i++) {
            if (audio_playing[i]) {                                 // Is channel active?
                if (audio_chans[i].nr4 & AUDIO_CONSECUTIVE) {    // Is it in length mode?
                    audio_length[i] -= 1;                           // decrease length

#ifdef AUDIO_VERBOSE
                    printf("AUDIO: Channel %d length deducted, remain %02x\n", i, audio_length[i]);
#endif
                    if (!audio_length[i])                           // if we're at zero, decduct
                    {
                        // Disable channel if length is over
                        audio_disable_channel(i);
                    }
                }
            }
        }

    }
}

/************************************************************************************
 *                             AUDIO GENERATOR FUNCTIONS                            *
 ************************************************************************************/

void audio_process_chunk(SAMPLE *stream, int len) {
    // This function should be called whenever a chunk of audio needs to be processed.
    // len is the length in SAMPLEs that is requested.
    // We use the global variable audio_sample_rate in some calculations so make sure this is correctly set.

#ifdef USE_AUDIO_TIMING
    audio_timer += (float) len / (float) audio_sample_rate;
#endif

    // if audio is disabled just zero the buffer and go away.
    if (!AUDIO_SOUND_ENABLED) {
        memset(stream, 0, len * sizeof(SAMPLE) * audio_amount_channels);
        return;
    }

    len *= audio_amount_channels; // Stereo (two channels), so double the length

    for (int i = 0; i < len; i += audio_amount_channels) {
        // Process length and envelope timers.
        audio_length_timer();
        audio_envelope_timer();
        audio_sweep_timer();

        stream[i+0] = 0;
        stream[i+1] = 0;

        square(0, &stream[i]);     // Mix in Channel 1 (Square)
        square(1, &stream[i]);     // Mix in Channel 2 (Square)
        waveform(&stream[i]);      // Mix in Channel 3 (Waveform)
        noise(&stream[i]);         // Mix in Channel 4 (Noise)

    }

}

inline void square(unsigned int i, SAMPLE *buffer) {
    if (audio_playing[i]) {
        audio_counter[i] += 1.0;
        if (audio_counter[i] >= audio_cycle[i]) {
            audio_counter[i] -= audio_cycle[i];
            audio_sample[i] = (audio_sample[i] >> 1) | (audio_sample[i] << 7);
        }

        if (AUDIO_NR51 & (1 << i)) {
            if (audio_sample[i] & 0x01) { buffer[0] += audio_output_l[i]; }
                                   else { buffer[0] -= audio_output_l[i]; }
        }

        if (AUDIO_NR51 & (1 << (i+4))) {
            if (audio_sample[i] & 0x01) { buffer[1] += audio_output_r[i]; }
                                   else { buffer[1] -= audio_output_r[i]; }
        }
    }
}

inline void waveform(SAMPLE *buffer) {

    if (audio_playing[2]) {
        audio_counter[2] += 1.0;
        if(audio_counter[2] >= audio_cycle[2]) {
            audio_counter[2] -= audio_cycle[2];
            audio_waveform_idx = (audio_waveform_idx + 1) & 0x1F;

            audio_update_waveform_data();

        }

        if (AUDIO_NR51 & (1 << 2))
            buffer[0] += audio_waveform_output[0];

        if (AUDIO_NR51 & (1 << (2+4)))
            buffer[1] += audio_waveform_output[1];
    }

}

inline void noise(SAMPLE *buffer) {

    if (audio_playing[3]) {

        audio_counter[3] += 1.0;

        if(audio_counter[3] >= audio_cycle[3]) {
            audio_counter[3] -= audio_cycle[3];
            audio_noise_idx++;
            if (AUDIO_NR43 & AUDIO_NOISE_MODE)
                audio_noise_idx &= 0x7F;
            else
                audio_noise_idx &= 0x7FFF;
        }

        uint8_t sample;

        if (AUDIO_NR43 & AUDIO_NOISE_MODE)
            sample = audio_noise_lut7[audio_noise_idx];
        else
            sample = audio_noise_lut15[audio_noise_idx];

        if (AUDIO_NR51 & (1 << 3)) {
            if (sample & 0x01) buffer[0] += audio_output_l[3];
                          else buffer[0] -= audio_output_l[3];
        }

        if (AUDIO_NR51 & (1 << (3+4))) {
            if (sample & 0x01) buffer[1] += audio_output_r[3];
                          else buffer[1] -= audio_output_r[3];
        }

    }

}

/************************************************************************************
 *                              INIT AND DEINIT FUNCTIONS                           *
 ************************************************************************************/

void audio_init() {
    audio_backend_init();
    audio_256hz_cycle = ((float) audio_sample_rate / 256.0);
    audio_64hz_cycle = ((float) audio_sample_rate / 64.0);
    audio_generate_luts();
}

void audio_deinit() {
    audio_backend_deinit();
}

void audio_generate_luts() {

    // Generate LUTs for the LFSR.

    uint8_t lfsr7 = 0x7F;
    uint16_t lfsr15 = 0x7FFF;

    // We have two LFSR LUTs for noise channel.
    // New MSB = Old Bit 1 XOR Old Bit 0a
    // Output = Bit 0

    // 7-Stage LFSR
    for (long i = 0; i < 128; i++) {
       audio_noise_lut7[i] = (lfsr7 & 1);
       lfsr7 = (lfsr7 >> 1) | ((((lfsr7 >> 1) & 1) ^ (lfsr7 & 1)) << 6);
    }

    // 15-Stage LFSR
    for (long i = 0; i < 32768; i++) {
       audio_noise_lut15[i] = (uint8_t) (lfsr15 & 1);
       lfsr15 = (lfsr15 >> 1) | ((((lfsr15 >> 1) & 1) ^ (lfsr15 & 1)) << 14);
    }

}


#ifdef AUDIO_NONE

uint32_t audio_sample_rate = 48000;
uint32_t audio_amount_channels = 2;

void audio_backend_init() {}
void audio_backend_deinit() {}

#endif
