#include "backends.h"

#define NAME "v_sdl2"

#ifdef VIDEO_SDL2

/*
 *  Video Backend Implementation for SDL2 (Multiplatform)
 */

#include <SDL2/SDL.h>
#include <stdio.h>

#include "trace.h"
#include "video.h"

static SDL_Surface * framebuffer = NULL;
static SDL_Window * video_window = NULL;
static SDL_Surface * window_surface = NULL;

static uint32_t *framebuffer_pixels = NULL;
static uint32_t pal_rgb[4*3]; // BGP, OBP1 and OBP2 in RGB32 format

static const uint32_t bw_palette[4] = {0xffffffff,
                                       0xffaaaaaa,
                                       0xff555555,
                                       0xff000000};

int v_sdl2_init(video_config* cfg) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        print_msg("Error initializing SDL.\n");
        return -1;
    }

    video_window = SDL_CreateWindow("MercyBoy",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    cfg->screen_width,
                                    cfg->screen_height,
                                    SDL_WINDOW_RESIZABLE);

    if (!video_window) {
        print_msg("Error opening window\n");
        return -1;
    }

    window_surface = SDL_GetWindowSurface(video_window);

    if (!window_surface) {
        print_msg("Failed to get surface from the window\n");
        return -1;
    }

    framebuffer = SDL_CreateRGBSurface(0,
                                       LCD_WIDTH,
                                       LCD_HEIGHT,
                                       32,
                                       window_surface->format->Rmask,
                                       window_surface->format->Gmask,
                                       window_surface->format->Bmask,
                                       window_surface->format->Amask);

    if (!framebuffer) {
        print_msg("Error creating internal framebuffer\n");
        return -1;
    }

    // make a pointer to the actual pixel data of this surface
    framebuffer_pixels = framebuffer->pixels;

    return 0;
}

void v_sdl2_deinit() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
}


void v_sdl2_update_palette(uint8_t pal_offset, gameboy_palette palette) {
    pal_rgb[0+pal_offset] = bw_palette[palette.color[0]];
    pal_rgb[1+pal_offset] = bw_palette[palette.color[1]];
    pal_rgb[2+pal_offset] = bw_palette[palette.color[2]];
    pal_rgb[3+pal_offset] = bw_palette[palette.color[3]];
}

void v_sdl2_write_line(int line, uint8_t *linebuf) {
    int16_t i;
    uint32_t *dst_line = &framebuffer_pixels[LCD_WIDTH * line];
    for (i = 0; i < LCD_WIDTH; i++) {
        *(dst_line++) = pal_rgb[*(linebuf++)];
    }
}

void v_sdl2_frame_done() {
    SDL_Rect stretchRect;

    window_surface = SDL_GetWindowSurface(video_window);

    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = window_surface->w;
    stretchRect.h = window_surface->h;

    SDL_BlitScaled(framebuffer, NULL, window_surface, &stretchRect);
    SDL_UpdateWindowSurface(video_window);
    SDL_PumpEvents();
}

video_backend_status v_sdl2_event_handler() {
    video_backend_status ret = VIDEO_BACKEND_RUNNING;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            ret = VIDEO_BACKEND_EXIT;
        }
    }

    return ret;
}

const video_backend_t v_sdl2 = {
    NAME,                   // .name
    1,                      // .present
    v_sdl2_init,            // .init
    v_sdl2_deinit,          // .deinit
    v_sdl2_update_palette,  // .update_palette
    v_sdl2_write_line,      // .write_line
    v_sdl2_frame_done,      // .frame_done
    v_sdl2_event_handler    // .event_handler
};
#else
const video_backend_t v_sdl2 = { NAME, 0 };
#endif
