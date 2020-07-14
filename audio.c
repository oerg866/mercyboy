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

float audio_counter[4] = {0.0, 0.0, 0.0, 0.0};
float audio_length[4] = {0.0, 0.0, 0.0, 0.0};

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
#define AUDIO_VOLUME (data >> 4)

#define MASTER_CLOCK

#define AUDIO_VERBOSE

static SAMPLE audio_volume[4] = {0,0,0,0};
static uint8_t audio_sample[4] = {0,0,0,0};



void audio_handle_write(uint16_t addr, uint16_t data) {

    uint8_t cidx = (addr - 0xFF10) / 5; // 5 registers per channel, one dummy
    struct audio_channel * chan = &audio_chans[cidx];

#ifdef AUDIO_VERBOSE
    printf("AUDIO: Handle write, addr %04x, cidx %d, data %02x\n", addr, cidx, data);
#endif

    switch(addr) {

        // DUTY CYCLES

        case MEM_NR11:
        case MEM_NR21:
            audio_sample[cidx] = audio_square_waves[AUDIO_DUTY_CYCLE];
            break;

        // SET ENVELOPE

        case MEM_NR12:
        case MEM_NR22:
            audio_volume[cidx] = AUDIO_VOLUME * (INT16_MAX/2)/15;
            break;



        // FREQUENCY CHANGES & NOTE TRIGGER

        case MEM_NR13:  // Channel 1 (Square)
        case MEM_NR14:
        case MEM_NR23:  // Channel 2 (Square)
        case MEM_NR24:
        case MEM_NR33:  // Channel 3 (Waveform)
        case MEM_NR34:
            audio_cycle[cidx] = ((float) audio_sample_rate / (audio_divider[cidx] / ((2048.0 - (float) (((chan->nr4 & 0x07) << 8) | chan->nr3))))) / 8.0;
            if (chan->nr4 & AUDIO_TRIGGER_BIT) {
                audio_playing[cidx] = 1;
                audio_counter[cidx] = 0.0; // NOTE ON
                chan->nr4 &= ~AUDIO_TRIGGER_BIT;  // Delete flag so we dont keep triggering

#ifdef AUDIO_VERBOSE
                printf("AUDIO: Note on on channel %d:, cycle = %f, freq: %04x\n", cidx, audio_cycle[cidx], (((chan->nr4 & 0x07) << 8) | chan->nr3));
#endif

            }
            break;

    }

}

inline SAMPLE square(unsigned int i) {
    if (1) {
        audio_counter[i] += 1.0;
        if (audio_counter[i] >= audio_cycle[i]) {
            audio_counter[i] -= audio_cycle[i];
            audio_sample[i] = (audio_sample[i] >> 1) | (audio_sample[i] << 7);
        }

        if (audio_sample[i] & 0x01)
            return audio_volume[i];
        return -audio_volume[i];


    } else {
        return 0;
    }
}


inline void audio_do_sweep() {

    // TODO
}

inline void audio_do_env() {

    // TODO
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

    // TODO... No brain power for this at the moment
    SAMPLE ch1, ch2, ch3, ch4;
    for (int i = 0; i < (len>>1); i+=2) {
        ch1 = square(0);
        ch1 += square(1);
        output[i] = ch1;
        output[i+1] = ch1;
    }

//    memset(stream, 0, len);

}

void audio_sdl_init() {

    audio_sec_per_sample = 1.0 / (float) audio_sample_rate;

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
