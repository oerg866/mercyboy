#ifdef AUDIO_SDL2

/*
 *  Audio Backend Implementation for SDL 2.x
 *
 */

#include "audio_sdl2.h"

#include <stdio.h>
#include <SDL2/SDL.h>

static uint8_t *audio_chunk;
static uint8_t *audio_pos;

SDL_AudioSpec audio_spec;
uint32_t audio_sample_rate;
uint32_t audio_buffer_size;
uint32_t audio_amount_channels;

void audio_sdl_callback(void *udata, uint8_t *stream, int len) {

    /* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
    */

    audio_process_chunk((SAMPLE*) stream, len / (sizeof(SAMPLE) * audio_amount_channels));

}

void audio_backend_init() {

    // Initialize SDL2 Audio Backend

    audio_buffer_size = AUDIO_BUFFER_SIZE;
    audio_sample_rate = AUDIO_SAMPLE_RATE;
    audio_amount_channels = 2;

    // Audio Spec Structure with format detaisl

    audio_spec.freq = audio_sample_rate;
    audio_spec.format = AUDIO_S16;
    audio_spec.channels = audio_amount_channels;
    audio_spec.samples = audio_buffer_size;
    audio_spec.callback = audio_sdl_callback;
    audio_spec.userdata = NULL;


    audio_chunk = malloc(audio_buffer_size * sizeof(SAMPLE) * audio_amount_channels);

    memset(audio_chunk, 0, audio_buffer_size * sizeof(SAMPLE) * audio_amount_channels);

    audio_pos = audio_chunk;

    if ( SDL_OpenAudio(&audio_spec, NULL) < 0 ) {
        printf("AUDIO: Couldn't open audio: %s\n", SDL_GetError());
    }

    SDL_PauseAudio(0);

}

void audio_backend_deinit() {

    free(audio_chunk);

}

#endif
