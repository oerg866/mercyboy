#ifndef BENDLIST_H
#define BENDLIST_H

#include "backends.h"

// extern audio_backend_t a_alsa; or oss maybe for SUPER OLD linux
// extern audio_backend_t a_dsound5;
// extern audio_backend_t a_dsound7;
extern audio_backend_t a_sdl2;
extern audio_backend_t a_waveout;
extern audio_backend_t a_sb16dj;
extern audio_backend_t a_dummy;

// extern input_backend_t i_dinput;
extern input_backend_t i_sdl2;
extern input_backend_t i_win32;
extern input_backend_t i_dos32;

extern video_backend_t v_gdi;
extern video_backend_t v_sdl2;
extern video_backend_t v_dosvga;
// extern video_backend_t v_ddraw7
// extern video_backend_t v_opengl

audio_backend_t *const audio_backends[] = {
    &a_sdl2,
    &a_waveout,
    &a_sb16dj,

    // Lowest priority, dummy backend...
    &a_dummy,
};

video_backend_t *const video_backends[] = {
    &v_sdl2,
    &v_gdi,
    &v_dosvga,
};

input_backend_t *const input_backends[] = {
    &i_sdl2,
    &i_win32,
    &i_dos32,
};

#endif
