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

void video_init(SDL_Surface *init_surface, SDL_Window *init_window);
void video_cycles(int cycles);
void video_draw_line();
void video_update_framebuffer();

#endif
