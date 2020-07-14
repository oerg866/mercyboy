#include "audio.h"

#include <stdio.h>
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

static float audio_freqs[4] = {0.0, 0.0, 0.0, 0.0};

float audio_counter[4] = {0.0, 0.0, 0.0, 0.0};
float audio_length[4] = {0.0, 0.0, 0.0, 0.0};

#define ENV_STEP 1.0/64.0

SDL_AudioSpec audio_spec;

float audio_timer = 0.0;

float audio_sec_per_sample = 0;

#define SAMPLE int16_t     // S16

#define DUTY_125    0
#define DUTY_25     1
#define DUTY_50     2
#define DUTY_75     3

#define MASTER_CLOCK

inline SAMPLE square12() {

    // TODO
}

inline SAMPLE square25() {

    // TODO
}

inline SAMPLE square50() {

    // TODO
}

inline SAMPLE square75() {

    // TODO
}

inline void audio_do_sweep() {

    // TODO
}

inline void audio_do_env() {

    // TODO
}

inline void audio_do_counter(int i) {

    // Only for a certain length... increase counter and see if we have to end it
    audio_counter[i] += audio_sec_per_sample;

}


void audio_sdl_callback(void *udata, uint8_t *stream, int len) {

    /* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
    */

    // increase audio timer

#ifdef USE_AUDIO_TIMING
    audio_timer += (float) audio_buffer_size / (float) audio_sample_rate;
#endif

    printf ("AUDIO: Filling buffer: udata = %x, stream = %x, len = %d\n", (unsigned int) udata, (unsigned int) stream, len);

    /* TODO... No brain power for this at the moment
     *
    // Get frequencies for all channels
    for (int i = 0; i < 4; i++) {
        audio_freqs[i] = 131072.0 / (2048.0 - (float) ((audio_chans[i].freq_l & 0x07) | audio_chans[i].freq_h));
    }

    for (int i = 0; i < len; i++) {
        stream[i] = square50(i);
    }

    */

    memset(stream, 0, len);

}

void audio_sdl_init() {

    audio_sec_per_sample = 1.0 / (float) audio_sample_rate;

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

    audio_chunk = malloc(AUDIO_BUFFER_SIZE * sizeof(SAMPLE));

    memset(audio_chunk, 0, AUDIO_BUFFER_SIZE * sizeof(SAMPLE));

    audio_pos = audio_chunk;

    SDL_PauseAudio(0);

}

void audio_sdl_deinit() {

    free(audio_chunk);

}
