#include "audio.h"

#include <stdio.h>
#include <limits.h>

#include "mem.c"
#include "sys.c"

static uint8_t *audio_chunk;
static uint32_t audio_len;
static uint8_t *audio_pos;

uint8_t * audio_wavedata = &ram_io[0x30];

struct audio_channel* audio_chans = (struct audio_channel*) &AUDIO_NR10;

static uint32_t audio_buffer_size = AUDIO_BUFFER_SIZE;
static uint32_t audio_sample_rate = AUDIO_SAMPLE_RATE;

static float audio_sweep_count[4] = {0.0, 0.0, 0.0, 0.0};

static const float audio_sweep_times[8] = {0.0078, 0.0156, 0.0234, 0.00313, 0.0391, 0.0469, 0.0547};


static float audio_divider[4] = {131072.0, 131072.0, 65536.0, 0.0};
static float audio_cycle[4] = {0.0, 0.0, 0.0, 0.0};

static float audio_counter[4] = {0.0, 0.0, 0.0, 0.0};
static float audio_envelope_cycle[4] = {0.0, 0.0, 0.0, 0.0};
static float audio_envelope_count[4] = {0.0, 0.0, 0.0, 0.0};

uint8_t audio_length[4] = {0,0,0,0};

uint8_t audio_playing[4] = {0,0,0,0};

#define ENV_STEP 1.0/64.0

SDL_AudioSpec audio_spec;

float audio_timer = 0.0;

float audio_sec_per_sample = 0.0;

#define SAMPLE int16_t     // S16

#define SAMPLE_D125     0x01
#define SAMPLE_D25      0x81
#define SAMPLE_D50      0x8F
#define SAMPLE_D75      0x7E

#define DUTY_125        0x00
#define DUTY_25         0x40
#define DUTY_50         0x80
#define DUTY_75         0xC0

const uint8_t audio_square_waves[4] = {SAMPLE_D125,SAMPLE_D25,SAMPLE_D50,SAMPLE_D75};

#define AUDIO_DUTY_CYCLE ((data & 0xC0) >> 6)
#define AUDIO_LENGTH64 (data & 0x3F)
#define AUDIO_VOLUME (data >> 4)

#define AUDIO_SOUND_ENABLED (AUDIO_NR52 & 0x89)
#define AUDIO_ENVELOPE (data & 0x07)
#define AUDIO_ENVELOPE_AMPLIFY (1<<3)
#define AUDIO_CONSECUTIVE (1<<6)

#define MASTER_CLOCK

#define NR1 0
#define NR2 1
#define NR3 2
#define NR4 3
#define NR5 4

static float audio_256hz_cycle; // = ((float) AUDIO_SAMPLE_RATE / 256.0) / (float) audio_buffer_size);
static float audio_64hz_cycle; // = ((float) AUDIO_SAMPLE_RATE / 64.0 / (float) audio_buffer_size);

static float audio_256hz_timer = 0.0;
static float audio_64hz_timer = 0.0;

#define AUDIO_VERBOSE

static uint8_t audio_volume[4] = {0,0,0,0};
static SAMPLE audio_output_l[4] = {0,0,0,0};
static SAMPLE audio_output_r[4] = {0,0,0,0};
static uint8_t audio_master_volume[2] = {0,0};
static uint8_t audio_sample[4] = {0,0,0,0};

uint8_t audio_handle_read(uint16_t addr) {


    uint8_t cidx = (addr - 0xFF10) / 5; // 5 registers per channel, one dummy
//    uint8_t creg = (addr - 0xFF10) % 5;

//    struct audio_channel * chan = &audio_chans[cidx];

    uint8_t data = ram_io[addr - 0xFF10];

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

    uint8_t cidx = (addr - 0xFF10) / 5; // 5 registers per channel, one dummy
    struct audio_channel * chan = &audio_chans[cidx];

#ifdef AUDIO_VERBOSE
    //printf("AUDIO: Handle write, addr %04x, cidx %d, data %02x\n", addr, cidx, data);
#endif

    switch(addr) {

    // DUTY CYCLES

    case MEM_NR11:
    case MEM_NR21:
        audio_sample[cidx] = audio_square_waves[AUDIO_DUTY_CYCLE];
    case MEM_NR41:
        audio_length[cidx] = 64 - AUDIO_LENGTH64;
        break;

    case MEM_NR31:
        audio_length[cidx] = 256 - data;
        break;

    // SET ENVELOPE

    case MEM_NR12:
    case MEM_NR22:
        audio_envelope_cycle[cidx] = (float) (AUDIO_ENVELOPE) * ((float) audio_sample_rate / 64.0);

#ifdef AUDIO_VERBOSE
        printf("AUDIO: CH %d SET envelope cycle %f, amplify mode %01x\n", cidx, audio_envelope_cycle[cidx], data & AUDIO_ENVELOPE_AMPLIFY);
#endif

        break;



        // FREQUENCY CHANGES & NOTE TRIGGER

    case MEM_NR13:  // Channel 1 (Square)
    case MEM_NR14:
    case MEM_NR23:  // Channel 2 (Square)
    case MEM_NR24:
    case MEM_NR33:  // Channel 3 (Waveform)
    case MEM_NR34:
        ram_io[addr-0xFF00] = data;
        audio_cycle[cidx] = ((float) audio_sample_rate / (audio_divider[cidx] / ((2048.0 - (float) (((chan->nr4 & 0x07) << 8) | chan->nr3))))) / 8.0;
        if (chan->nr4 & AUDIO_TRIGGER_BIT) {
            audio_playing[cidx] = 1;
            audio_counter[cidx] = 0.0; // NOTE ON
            // Trigger means load the volume from NR2 and start playing.
            audio_volume[cidx] = chan->nr2 >> 4;
            audio_update_volume(cidx);
            chan->nr4 &= ~AUDIO_TRIGGER_BIT;  // Delete flag so we dont keep triggering

#ifdef AUDIO_VERBOSE
            printf("AUDIO: Note on on channel %d:, cycle = %f, freq: %04x\n", cidx, audio_cycle[cidx], (((chan->nr4 & 0x07) << 8) | chan->nr3));
#endif

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


inline void audio_update_volume(int i) {
                     // max 0x0f          max 0x07
    audio_output_l[i] = audio_volume[i] * audio_master_volume[0] * (INT16_MAX / (0x07*0x0f));   // L Update
    audio_output_r[i] = audio_volume[i] * audio_master_volume[1] * (INT16_MAX / (0x07*0x0f));   // R Update
#ifdef AUDIO_VERBOSE
    printf("AUDIO: CH %d - Volume update. VOL: %02x, MVOL_L: %02x, MVOL_L: %02x, OUT_L %04x, OUT_R %04x\n", i, audio_volume[i], audio_master_volume[0], audio_master_volume[1], audio_output_l[i], audio_output_r[i]);
#endif

}

void audio_envelope_timer() {
    audio_64hz_timer += 1.0;

    if (audio_64hz_timer >= audio_64hz_cycle) {

        audio_64hz_timer -= audio_64hz_cycle;

        for (int i = 0; i < 4; i++) {
            if (audio_envelope_cycle[i] != 0.0) {
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

#ifdef AUDIO_VERBOSE
                        printf("AUDIO: CH %d - decreasing volume %02x\n", i, audio_volume[i]);
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
                if (!(audio_chans[i].nr4 & AUDIO_CONSECUTIVE)) {    // Is it in length mode?
                    audio_length[i] -= 1;                           // decrease length

#ifdef AUDIO_VERBOSE
//              printf("AUDIO: Channel %d length deducted, remain %02x\n", i, audio_length[i]);
#endif
                    if (!audio_length[i])                           // if we're at zero, decduct
                        audio_playing[i] = 0;                       // disable if length is over.
                }
            }
        }

    }
}

inline void square(unsigned int i, SAMPLE *buffer) {
    if (audio_playing[i]) {
        audio_counter[i] += 1.0;
        if (audio_counter[i] >= audio_cycle[i]) {
            audio_counter[i] -= audio_cycle[i];
            audio_sample[i] = (audio_sample[i] >> 1) | (audio_sample[i] << 7);
        }

        if (audio_sample[i] & 0x01) {
            buffer[0] = audio_output_l[i];
            buffer[1] = audio_output_r[i];
        } else {
            buffer[0] = (-audio_output_l[i]);
            buffer[1] = (-audio_output_r[i]);
        }

        // If channel L or R output is disabled just null it.
        if (!(AUDIO_NR51 & (1 << i)))
            buffer[0] = 0;

        if (!(AUDIO_NR51 & (1 << (i+4))))
            buffer[1] = 0;

    } else {
        buffer[0] = 0;
        buffer[1] = 0;
    }
}

void audio_sdl_callback(void *udata, uint8_t *stream, int len) {

    /* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
    */

    // Get int16t buffer

    SAMPLE* output = (SAMPLE*) stream;
    // increase audio timer

#ifdef USE_AUDIO_TIMING
    audio_timer += (float) audio_buffer_size / (float) audio_sample_rate;
#endif


    //    printf ("AUDIO: Filling buffer: udata = %x, stream = %x, len = %d\n", (unsigned int) udata, (unsigned int) stream, len);


    // if audio is disabled just zero the buffer and go away.
    if (!AUDIO_SOUND_ENABLED) {
        memset(stream, 0, len);
        return;
    }

    audio_length_timer();
    audio_envelope_timer();

    // TODO... No brain power for this at the moment
    SAMPLE tmp[2];

    for (int i = 0; i < (len>>1); i+=2) {
        output[i] = 0;
        output[i+1] = 0;
        square(0, tmp);
        output[i] += tmp[0]; output[i+1] += tmp[1];
        square(1, tmp);
        output[i] += tmp[0]; output[i+1] += tmp[1];
    }


}

void audio_sdl_init() {

    audio_sample_rate = AUDIO_SAMPLE_RATE;
    audio_sec_per_sample = 1.0 / (float) audio_sample_rate;
    audio_buffer_size = AUDIO_BUFFER_SIZE;
    audio_256hz_cycle = ((float) audio_sample_rate / 256.0) / (float) audio_buffer_size;
    audio_64hz_cycle = ((float) audio_sample_rate / 64.0) / (float) audio_buffer_size;

    printf ("AUDIO: Seconds per sample: %f\n", audio_sec_per_sample);

    audio_spec.freq = audio_sample_rate;
    audio_spec.format = AUDIO_S16;
    audio_spec.channels = 2;
    audio_spec.samples = audio_buffer_size;
    audio_spec.callback = audio_sdl_callback;
    audio_spec.userdata = NULL;

    printf ("Audio Channel 1 Address: %x\n", &audio_chans[0]);
    printf ("Audio Channel 2 Address: %x\n", &audio_chans[1]);
    printf ("Audio Channel 3 Address: %x\n", &audio_chans[2]);
    printf ("Audio Channel 4 Address: %x\n", &audio_chans[3]);

    if ( SDL_OpenAudio(&audio_spec, NULL) < 0 ) {
        printf("AUDIO: Couldn't open audio: %s\n", SDL_GetError());
    }

    audio_chunk = malloc(audio_buffer_size * sizeof(SAMPLE));

    memset(audio_chunk, 0, audio_buffer_size * sizeof(SAMPLE));

    audio_pos = audio_chunk;

    SDL_PauseAudio(0);

}

void audio_sdl_deinit() {

    free(audio_chunk);

}
