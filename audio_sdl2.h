#pragma once

#ifdef AUDIO_SDL2

#ifndef AUDIO_SDL2_H
#define AUDIO_SDL2_H

#include <stdint.h>
#include "SDL2/SDL.h"

#define SAMPLE int16_t     // PCM S16 LE

#include "audio.h"

// Defines for SDL2 backend

#define SAMPLE int16_t     // S16
#define SAMPLE_MAX INT16_MAX

#define AUDIO_BUFFER_SIZE 512
#define AUDIO_SAMPLE_RATE 48000


#endif // AUDIO_SDL_H


#endif
