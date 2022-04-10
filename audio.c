#include "audio.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "compat.h"

#include "mem.h"
#include "sys.h"
#include "trace.h"

#include "backends.h"

// Pointer to I/O audio region in memory. This is so that the code is easier to read when accessing registers all over the place.
struct audio_channel* audio_chans = (struct audio_channel*) &AUDIO_NR10;

// Pointer to arbitrary waveform data for waveform channel.
static uint8_t * audio_wavedata = &ram_io[0x30];

// Sweep timer variables.
static float audio_sweep_cycle = 0;
static uint8_t audio_sweep_shift = 0;
static float audio_sweep_counter = 0;
static uint8_t audio_sweep_enabled = 0;
static float audio_sweep_frequency = 0;

// Predefined array of samples for each square wave duty cycle type.
const  uint8_t audio_square_waves[4] = {SAMPLE_D125,SAMPLE_D25,SAMPLE_D50,SAMPLE_D75};

volatile float audio_timer = 0.0;                   // Constant audio timer. Globally available variable (so that video can be timed with it for example.
static float audio_timer_step = 0.0;

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

static uint8_t audio_length[4] = {0,0,0,0};         // Current length counter values for all channels
static uint8_t audio_length_latched[4] = {0,0,0,0};         // Length values for all channels
static uint8_t audio_playing[4] = {0,0,0,0};        // Playing Y/N flags for all channels

static uint8_t audio_volume[4] = {0,0,0,0};         // Current volume as updated by either initial volume or envelope.
static uint8_t audio_master_volume[2] = {0,0};      // Master volume for L and R channels.
static uint8_t audio_sample[4] = {0,0,0,0};         // Current type of square wave output. (One entry of audio_square_waves gets put here)

static uint8_t audio_noise_lut7[128];               // 7-stage LFSR LUT
static uint8_t audio_noise_lut15[32768];            // 15-stage LFSR LUT

static uint16_t audio_noise_idx = 0;                // index of current entry in LFSR LUT
static uint8_t audio_waveform_idx = 0;              // index of current sample in waveform data

static int16_t audio_waveform_output[2] = {0,0};
static int16_t audio_output_l[4] = {0,0,0,0};       // +1 state output in SAMPLE type form for L channel (Channel volume and Master volume respected)
static int16_t audio_output_r[4] = {0,0,0,0};       // +1 state output in SAMPLE type form for L channel (Channel volume and Master volume respected)

static audio_backend_t *s_backend = NULL;
static audio_config *s_config = NULL;

static uint8_t *s_audio_frame_buffer = NULL;

static float audio_samples_per_frame = 0.0;
static uint32_t audio_samples_last_frame = 0;

/************************************************************************************
 *                                AUDIO I/O FUNCTIONS                               *
 ************************************************************************************/

uint8_t audio_handle_read(uint16_t addr) {
    // Called by memory R/W handler
    // Handle reads from Audio I/O region.

    uint8_t data = ram_io[addr - 0xFF10];

    int cidx = (addr - 0xFF10) / 5; // 5 registers per channel, one dummy
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
        audio_sweep_cycle = ((float) s_config->sample_rate * (float) ((data >> 4) & 0x07)) / 128.0;
        audio_sweep_shift = data & 0x07;
        break;

    // DUTY CYCLES

    case MEM_NR11:
    case MEM_NR21:
        audio_sample[cidx] = audio_square_waves[AUDIO_DUTY_CYCLE];
        audio_length_latched[cidx] = 64 - AUDIO_LENGTH64;
        break;

    case MEM_NR41:
        audio_length_latched[cidx] = 64 - AUDIO_LENGTH64;
        break;

    case MEM_NR31:
        // Noise channel note length can be up to 256 ticks
        audio_length_latched[cidx] = 256 - data;
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
            audio_cycle[cidx] = ((float) s_config->sample_rate / (audio_divider[cidx] / (2048.0 - (float) (((chan->nr4 & 0x07) << 8) | chan->nr3)))) / 8.0;
        else if (cidx == 2)
            audio_cycle[cidx] = ((float) s_config->sample_rate / (audio_divider[cidx] / (2048.0 - (float) (((chan->nr4 & 0x07) << 8) | chan->nr3)))) / 32.0;

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

            audio_length[cidx] = audio_length_latched[cidx];

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

    audio_cycle[3] = (float) s_config->sample_rate / (dividingratio / (float) (2 << (shiftclock)));

}

void audio_update_volume(int i) {
                     // max 0x0f          max 0x07
    audio_output_l[i] = audio_volume[i] * audio_master_volume[0] * ((INT16_MAX/4) / (0x07*0x0f));   // L Update
    audio_output_r[i] = audio_volume[i] * audio_master_volume[1] * ((INT16_MAX/4) / (0x07*0x0f));   // R Update

    if (i==2) {
        audio_update_waveform_data();   // If we're updating volume for the waveform channel we must also update the waveform data
    }

    trace(TRACE_AUDIO, "CH %d - Volume update. VOL: %02x, MVOL_L: %02x, MVOL_L: %02x, OUT_L %04x, OUT_R %04x\n", i, audio_volume[i], audio_master_volume[0], audio_master_volume[1], audio_output_l[i], audio_output_r[i]);
}

void audio_update_waveform_data() {

    // Update waveform data when requested

    if ((AUDIO_NR32 >> 5)) {

        // If we are not muted

        uint16_t tmp = audio_wavedata[audio_waveform_idx>>1];

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
                audio_cycle[0] = ((float) s_config->sample_rate / (audio_divider[0] / (2048.0 - audio_sweep_frequency))) / 8.0;
            }
        }

    }
}

void audio_envelope_timer() {
    int32_t i;

    audio_64hz_timer += 1.0;

    if (audio_64hz_timer >= audio_64hz_cycle) {

        audio_64hz_timer -= audio_64hz_cycle;

        for (i = 0; i < 4; i++) {

            // Don't do envelopes if the step is 0 or we're processing channel 3 (waveform)

            if ((i != 2) && (audio_envelope_cycle[i] != 0.0)) {
                audio_envelope_count[i] += 1.0;
                // If a cycle for this envelope has been reached, reset it and deduct volume.
                if (audio_envelope_count[i] >= audio_envelope_cycle[i]) {
                    audio_envelope_count[i] -= audio_envelope_cycle[i];

                    if (audio_chans[i].nr2 & AUDIO_ENVELOPE_AMPLIFY) {
                        if (audio_volume[i] < 15) {
                            audio_volume[i] = audio_volume[i] + 1;
                            trace(TRACE_AUDIO,"AUDIO: CH %d - increasing volume %02x\n", i, audio_volume[i]);
                        }
                    } else {
                        if (audio_volume[i]) {
                            audio_volume[i]--;
                            trace(TRACE_AUDIO,"AUDIO: CH %d - decreasing volume %02x\n", i, audio_volume[i]);
                        }
                    }
                    audio_update_volume(i);
                }
            }
        }

    }

}

void audio_length_timer() {
    int i;

    audio_256hz_timer += 1.0;
    if (audio_256hz_timer >= audio_256hz_cycle) {
        // 256 Hz Timer triggered, reset it and deduct lengths.
        audio_256hz_timer -= audio_256hz_cycle;

        // Process lengths for each channel

        for (i = 0; i < 4; i++) {
            if (audio_playing[i]) {                                 // Is channel active?
                if (audio_length[i] && (audio_chans[i].nr4 & AUDIO_CONSECUTIVE)) {    // Is it in length mode?
                    audio_length[i] -= 1;                           // decrease length

                    trace(TRACE_AUDIO,"AUDIO: Channel %d length deducted, remain %02x\n", i, audio_length[i]);

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

void audio_advance_timing(float seconds_per_buffer) {
///    print_msg("%f += %f", audio_timer, seconds_per_buffer);
    audio_timer += seconds_per_buffer;
}

void audio_wait_for_vsync() {
    while (audio_timer < 0.016);
    audio_timer -= 0.016;
}


/************************************************************************************
 *                             AUDIO GENERATOR FUNCTIONS                            *
 ************************************************************************************/

void audio_render_frame() {
    uint8_t *stream;
    uint8_t *buffer;
    uint32_t i;
    uint32_t samples_this_frame;
    int16_t left;
    int16_t right;
    int16_t *right_ptr; // This exists because in case of mono we set it to the left channel's address
    uint32_t bytes_per_sample = s_config->bits_per_sample / 8;
    int stereo = (s_config->channels == 2);
    audio_buffer_status result;

    right_ptr = (stereo) ? &right : &left;


    // This function should be called whenever a frameof audio needs to be processed.

    // get amount of samples to be processed this frame. Needs to be done so that on average
    // we end up with the correct amount even if this number is not an integer
    samples_this_frame = (uint32_t) (audio_samples_per_frame);
    if (audio_samples_last_frame != 0)
        samples_this_frame += ceil(audio_samples_per_frame) - audio_samples_last_frame;

    //print_msg("samples this frame: %lu, samples_per_frame: %f, sample_counter = %lu\n", samples_this_frame, audio_samples_per_frame, audio_samples_last_frame);
    audio_samples_last_frame = samples_this_frame;

    buffer = alloc_mem(samples_this_frame * bytes_per_sample * s_config->channels);
    stream = buffer;

    // if audio is disabled just zero the buffer and go away.
    if (!AUDIO_SOUND_ENABLED) {
        memset(stream, 0, samples_this_frame);
        return;
    }

    for (i = 0; i < samples_this_frame; i++) {
        // Process length and envelope timers.

        audio_length_timer();
        audio_envelope_timer();
        audio_sweep_timer();

        left = 0;
        *right_ptr = 0;

        square(0, &left, right_ptr);     // Mix in Channel 1 (Square)
        square(1, &left, right_ptr);     // Mix in Channel 2 (Square)
        waveform(&left, right_ptr);      // Mix in Channel 3 (Waveform)
        noise(&left, right_ptr);         // Mix in Channel 4 (Noise)

        if      (bytes_per_sample == 1) *((int8_t*) stream)  = left >> 8;
        else if (bytes_per_sample == 2) *((int16_t*) stream) = left;
        stream += bytes_per_sample;

        if (stereo) {
            if      (bytes_per_sample == 1) *((int8_t*) stream)  = right >> 8;
            else if (bytes_per_sample == 2) *((int16_t*) stream) = right;
            stream += bytes_per_sample;
        }
    }

    result = s_backend->play_buffer(buffer, samples_this_frame * bytes_per_sample * s_config->channels);

    if (result != AUDIO_BUFFER_TAKEN) {
        free(buffer);
    }

}

void square(unsigned int i, int16_t *left, int16_t *right) {
    if (audio_playing[i]) {
        audio_counter[i] += 1.0;
        if (audio_counter[i] >= audio_cycle[i]) {
            audio_counter[i] -= audio_cycle[i];
            audio_sample[i] = (audio_sample[i] >> 1) | (audio_sample[i] << 7);
        }

        if (AUDIO_NR51 & (1 << i)) {
            if (audio_sample[i] & 0x01) { *left += audio_output_l[i]; }
                                   else { *left -= audio_output_l[i]; }
        }

        if (AUDIO_NR51 & (1 << (i+4))) {
            if (audio_sample[i] & 0x01) { *right += audio_output_r[i]; }
                                   else { *right -= audio_output_r[i]; }
        }
    }
}

void waveform(int16_t *left, int16_t *right) {
    if (audio_playing[2]) {
        audio_counter[2] += 1.0;
        if(audio_counter[2] >= audio_cycle[2]) {
            audio_counter[2] -= audio_cycle[2];
            audio_waveform_idx = (audio_waveform_idx + 1) & 0x1F;
            audio_update_waveform_data();
        }

        if (AUDIO_NR51 & (1 << 2))
            *left += audio_waveform_output[0];

        if (AUDIO_NR51 & (1 << (2+4)))
            *right += audio_waveform_output[1];
    }

}

void noise(int16_t *left, int16_t *right) {

    uint8_t sample;

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

        if (AUDIO_NR43 & AUDIO_NOISE_MODE)
            sample = audio_noise_lut7[audio_noise_idx];
        else
            sample = audio_noise_lut15[audio_noise_idx];

        if (AUDIO_NR51 & (1 << 3)) {
            if (sample & 0x01) *left += audio_output_l[3];
                          else *left -= audio_output_l[3];
        }

        if (AUDIO_NR51 & (1 << (3+4))) {
            if (sample & 0x01) *right += audio_output_r[3];
                          else *right -= audio_output_r[3];
        }

    }

}

/************************************************************************************
 *                              INIT AND DEINIT FUNCTIONS                           *
 ************************************************************************************/

void mem_init_sample(int16_t *dest, int count) {
    memset(dest, 0, sizeof(int16_t) * count);
}

void audio_init(audio_backend_t *backend, audio_config *cfg) {
    uint32_t bytes_per_sample = cfg->bits_per_sample / 8;

    s_backend = backend;
    s_config = cfg;

    s_backend->init(s_config);

    audio_timer_step = (float) s_config->buffer_size / (float) s_config->sample_rate;
    audio_256hz_cycle = ((float) s_config->sample_rate / 256.0);
    audio_64hz_cycle = ((float) s_config->sample_rate / 64.0);

    audio_samples_per_frame = cfg->sample_rate / 60.0;
    audio_samples_last_frame = 0;

    // Allocate "framebuffer" based on samples per frame's maximum possible integer value

    s_audio_frame_buffer = alloc_mem((uint32_t) audio_samples_per_frame * bytes_per_sample * cfg->channels);

    mem_init_float(audio_counter, 4);           // Note frequency cycle counter for each channel
    mem_init_float(audio_envelope_count, 4);    // Envelope cycle counter
    mem_init_float(audio_cycle, 4);             // Note frequency cycle length
    mem_init_float(audio_envelope_cycle, 4);    // Envelope cycle length

    mem_init_uint8(audio_length, 4);            // Current length counter values for all channels
    mem_init_uint8(audio_length_latched, 4);    // Length values for all channels
    mem_init_uint8(audio_playing, 4);           // Playing Y/N flags for all channels

    mem_init_uint8(audio_volume, 4);            // Current volume as updated by either initial volume or envelope.
    mem_init_uint8(audio_master_volume, 4);     // Master volume for L and R channels.
    mem_init_uint8(audio_sample, 4);            // Current type of square wave output. (One entry of audio_square_waves gets put here)

    audio_noise_idx = 0;                        // index of current entry in LFSR LUT
    audio_waveform_idx = 0;                     // index of current sample in waveform data

    mem_init_sample(audio_waveform_output, 2);
    mem_init_sample(audio_output_l, 4);            // +1 state output in SAMPLE type form for L channel (Channel volume and Master volume respected)
    mem_init_sample(audio_output_r, 4);            // +1 state output in SAMPLE type form for L channel (Channel volume and Master volume respected)
    audio_generate_luts();

    audio_sample[0] = audio_square_waves[((AUDIO_NR11 & 0xC0) >> 6)];
    audio_sample[1] = audio_square_waves[((AUDIO_NR21 & 0xC0) >> 6)];
}

void audio_deinit() {
    s_backend->deinit();
    free(s_audio_frame_buffer);
}

void audio_generate_luts() {

    int32_t i;

    // Generate LUTs for the LFSR.

    uint8_t lfsr7 = 0x7F;
    uint16_t lfsr15 = 0x7FFF;

    // We have two LFSR LUTs for noise channel.
    // New MSB = Old Bit 1 XOR Old Bit 0a
    // Output = Bit 0

    // 7-Stage LFSR
    for (i = 0; i < 128; i++) {
       audio_noise_lut7[i] = (lfsr7 & 1);
       lfsr7 = (lfsr7 >> 1) | ((((lfsr7 >> 1) & 1) ^ (lfsr7 & 1)) << 6);
    }

    // 15-Stage LFSR
    for (i = 0; i < 32768; i++) {
       audio_noise_lut15[i] = (uint8_t) (lfsr15 & 1);
       lfsr15 = (lfsr15 >> 1) | ((((lfsr15 >> 1) & 1) ^ (lfsr15 & 1)) << 14);
    }

}
