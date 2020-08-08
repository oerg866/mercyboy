#ifdef VIDEO_SDL2

#include "video.h"

#include <stdio.h>
#include <SDL2/SDL.h>

#ifdef USE_AUDIO_TIMING
#include "audio.h"
#endif

SDL_Surface * framebuffer = NULL;
SDL_Window * video_window = NULL;
SDL_Surface * window_surface = NULL;

uint32_t *framebuffer_pixels = NULL;

uint32_t pal_rgb[4*3]; // BGP, OBP1 and OBP2 in RGB32 format

int video_backend_init(int width, int height) {

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

    uint32_t rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    framebuffer = SDL_CreateRGBSurface(0, 160, 144, 32,
                                         rmask, gmask, bmask, amask);

    if (!framebuffer) {
        printf("Error creating internal framebuffer\n");
        return -1;
    }

    // make a pointer to the actual pixel data of this surface

    framebuffer_pixels = framebuffer->pixels;

    window_surface = SDL_GetWindowSurface(video_window);

    if (!window_surface) {
        printf("Failed to get surface from the window\n");
        return -1;
    }

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

    SDL_Surface *opt = SDL_ConvertSurface(framebuffer, window_surface->format, 0);
    SDL_BlitScaled(opt, NULL, window_surface, &stretchRect);

    SDL_UpdateWindowSurface(video_window);

#ifdef USE_AUDIO_TIMING
    while (audio_timer < 0.016);
    audio_timer = audio_timer - 0.016;
#else
    SDL_Delay(16);
#endif

    SDL_PumpEvents();
}

#endif
