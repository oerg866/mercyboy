#pragma once

#ifndef VIDEO_H
#define VIDEO_H

#include "cpu.h"
#include "mem.h"
#include <SDL2/SDL.h>

/*
extern uint8_t video_vbi;
extern uint8_t video_hbi;
*/

extern int lines_per_frame;
extern int cycles_per_line;
extern int cycles_per_frame;

extern int video_line_cycles;
extern int video_line_num;

extern SDL_Surface *video_surface;
extern SDL_Window *video_window;

#define PRIORITY_FALSE      0
#define PRIORITY_TRUE       1

#define TILES_BG            0
#define TILES_SPRITES       1

#define SPRITE_ATTR_PRIO    (1<<7)
#define SPRITE_ATTR_YFLIP   (1<<6)
#define SPRITE_ATTR_XFLIP   (1<<5)


void video_init(SDL_Surface *init_surface, SDL_Window *init_window);
void video_cycles(int cycles);
uint8_t video_flip_tile_byte(uint8_t src);
void video_draw_tile(uint16_t tileidx, int yoffset, int linexoffset, int xstart, int count, uint8_t sprites, uint8_t sprite_attr);
void video_draw_line();
void video_update_framebuffer();

struct spritedata {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attr;
};

#endif
