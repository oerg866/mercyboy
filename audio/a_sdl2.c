#include "backends.h"

#define NAME "a_sdl2"

#ifdef AUDIO_SDL2

/*
 *  Audio Backend Implementation for SDL 2.x
 *
 */


#include <stdio.h>
#include <SDL2/SDL.h>

#include "trace.h"

#include "audio.h"

SDL_AudioSpec s_audio_spec;
char* s_audio_buffer = NULL;
uint32_t s_buffer_size = 0;

#define BUFFER_COUNT 8

static audio_config *s_config = NULL;

static ringbuffer *s_ringbuf = NULL;

static float s_secs_per_buffer = 0.0;

void a_sdl2_callback(void *udata, uint8_t *stream, int len) {
    /* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
    */
    ringbuffer_unblock_current(s_ringbuf);
    uint8_t *current_buffer = ringbuffer_get_data(s_ringbuf, ringbuffer_increment_fetch_and_block(s_ringbuf));

    // len is what we set up as buffer size in the config!
    memcpy(stream, current_buffer, len);

    audio_advance_timing(s_secs_per_buffer);
}

int a_sdl2_init(audio_config *cfg) {
    // Initialize SDL2 Audio Backend
    // Audio Spec Structure with format detaisl

    s_config = cfg;

    s_audio_spec.freq = cfg->sample_rate;

    switch (cfg->bits_per_sample) {
        case 8:
            s_audio_spec.format = AUDIO_S8;
            break;
        case 16:
            s_audio_spec.format = AUDIO_S16;
            break;
    }

    s_audio_spec.channels = cfg->channels;
    s_audio_spec.samples = cfg->buffer_size;
    s_audio_spec.callback = a_sdl2_callback;
    s_audio_spec.userdata = NULL;

    s_secs_per_buffer = (float) cfg->buffer_size / (float)cfg->sample_rate;

//    audio_pos = audio_chunk;

    s_ringbuf = ringbuffer_create((cfg->bits_per_sample / 8) * cfg->channels * cfg->buffer_size, BUFFER_COUNT);

    ringbuffer_increment_fetch_and_block(s_ringbuf);
    ringbuffer_increment_fetch_and_block(s_ringbuf);
    ringbuffer_increment_fetch_and_block(s_ringbuf);
    ringbuffer_increment_fetch_and_block(s_ringbuf);


    if (!s_ringbuf) {
        trace(TRACE_AUDIO, "Could not allocate audio buffer memory");
    }

    if ( SDL_OpenAudio(&s_audio_spec, NULL) < 0 ) {
        trace(TRACE_AUDIO, "AUDIO: Couldn't open audio: %s", SDL_GetError());
        return -1;
    }

    SDL_PauseAudio(0);

    return 0;
}

void a_sdl2_deinit() {
    ringbuffer_destroy(s_ringbuf);
}

audio_buffer_status a_sdl2_play_buffer(uint8_t *buffer, uint32_t length) {
    ringbuffer_insert_bytes(s_ringbuf, buffer, length);
    // s_ringbuf->is_blocked[s_ringbuf->w_buf] = 1;

    return AUDIO_BUFFER_COPIED;
}

const audio_backend_t a_sdl2 = {
    NAME,               // name
    1,                  // present
    a_sdl2_init,        // init
    a_sdl2_deinit,      // deinit
    a_sdl2_play_buffer, // play_buffer
};
#else
const audio_backend_t a_sdl2 = { NAME, 0 };
#endif
