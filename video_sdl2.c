#ifdef VIDEO_SDL2

#include "video.h"

/*
 *  Video Backend Implementation for GDI Win32 (for very old systems)
 *
 */

#include <SDL2/SDL.h>
#include <stdio.h>

#ifdef USE_AUDIO_TIMING
#include "audio.h"
#endif

SDL_Surface * framebuffer = NULL;
SDL_Window * video_window = NULL;
SDL_Surface * window_surface = NULL;

uint32_t *framebuffer_pixels = NULL;
uint32_t pal_rgb[4*3]; // BGP, OBP1 and OBP2 in RGB32 format
const uint32_t bw_palette[4] = {0x00ffffff,0x00aaaaaa,0x00666666,0x00000000};

uint8_t video_backend_status;

int video_backend_init(int width, int height, int bitdepth) {

    video_window = SDL_CreateWindow("MercyBoy",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          width,
                                          height,
                                          SDL_WINDOW_RESIZABLE);

    if (!video_window) {
        printf("Error opening window\n");
        return -1;
    }

    window_surface = SDL_GetWindowSurface(video_window);

    if (!window_surface) {
        printf("Failed to get surface from the window\n");
        return -1;
    }

    framebuffer = SDL_CreateRGBSurface(0,
                                       160, 144,
                                       32,
                                       window_surface->format->Rmask,
                                       window_surface->format->Gmask,
                                       window_surface->format->Bmask,
                                       window_surface->format->Amask);

    if (!framebuffer) {
        printf("Error creating internal framebuffer\n");
        return -1;
    }

    // make a pointer to the actual pixel data of this surface
    framebuffer_pixels = framebuffer->pixels;

    video_backend_status = VID_BACKEND_STATUS_RUNNING;
    return 0;
}


inline void video_backend_update_palette(uint8_t pal_offset, uint8_t reg) {
    pal_rgb[0+pal_offset] = bw_palette[pal_int[0+pal_offset]];
    pal_rgb[1+pal_offset] = bw_palette[pal_int[1+pal_offset]];
    pal_rgb[2+pal_offset] = bw_palette[pal_int[2+pal_offset]];
    pal_rgb[3+pal_offset] = bw_palette[pal_int[3+pal_offset]];
}

void video_backend_draw_line(int line, uint8_t *linebuf) {
    for (int i = 0; i < 160; i++) {
        framebuffer_pixels[i + 160*line] = pal_rgb[linebuf[i]];
    }
}

void video_backend_update_framebuffer() {

    window_surface = SDL_GetWindowSurface(video_window);
    SDL_Rect stretchRect;
    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = window_surface->w;
    stretchRect.h = window_surface->h;

    // For some reason, the window surface has a different format even though it's supposed to be 32 bit rgb...

//    SDL_Surface *opt = SDL_ConvertSurface(framebuffer, window_surface->format, 0);
    SDL_BlitScaled(framebuffer, NULL, window_surface, &stretchRect);

    SDL_UpdateWindowSurface(video_window);

#ifdef USE_AUDIO_TIMING
    while (audio_timer < 0.016);
    audio_timer = audio_timer - 0.016;
#else
    SDL_Delay(16);
#endif

    SDL_PumpEvents();
}


void video_backend_handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            video_backend_status = VID_BACKEND_STATUS_EXIT;
        }
    }
}

uint8_t video_backend_get_status() {
    return video_backend_status;
}

#endif
