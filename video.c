#include "video.h"

#include "cpu.h"
#include "mem.h"
#include "sys.h"

/*
 * Mode 0:
000___000___000___000___000___000___000________________
Mode 1:
_______________________________________11111111111111__
Mode 2:
___2_____2_____2_____2_____2_____2___________________2_
Mode 3:
____33____33____33____33____33____33__________________3
 The Mode Flag goes through the values 0, 2,
 and 3 at a cycle of about 109uS. 0 is present
 about 48.6uS, 2 about 19uS, and 3 about 41uS.
 This is interrupted every 16.6ms by the VBlank
 (1). The mode flag stays set at 1 for about 1.08
 ms. (Mode 0 is present between 201-207 clks, 2
 about 77-83 clks, and 3 about 169-175 clks. A
 complete cycle through these states takes 456
 clks. VBlank lasts 4560 clks. A complete screen
 refresh occurs every 70224 clks.)

 */

int lines_per_frame = LINES_PER_FRAME; //154;
int cycles_per_line = CYCLES_PER_LINE; //456;
int cycles_per_frame = CYCLES_PER_FRAME; //456 * 154;

int video_line_cycles;
int video_frame_cycles;

uint32_t framebuffer32 [160*144];

uint32_t linebuf_final[160];
uint8_t linebuf[160];

uint32_t palette[4] = {0x00ffffff,0x00aaaaaa,0x00555555,0x00000000};

SDL_Surface * video_surface;
SDL_Window * video_window;

void video_init(SDL_Surface *init_surface, SDL_Window *init_window) {
    video_line_cycles = cycles_per_line;
    video_frame_cycles = cycles_per_frame;
    video_surface = init_surface;
    video_window = init_window;
    VID_LY = 0x00;
}

void video_cycles(int cycles) {
    cycles = cycles >> 2; // Machine cycles, not CPU cycles
    video_line_cycles -= cycles;
    video_frame_cycles -= cycles;

    if (video_line_cycles <= 0) {

        if (VID_LY < 144) {
            // draw current line if we're in active display
            video_draw_line();
        } else if (VID_LY == 144) {
            // request vblank interrupt
            sys_interrupt_req(INT_VBI);
        }

        VID_LY++;

        video_line_cycles = cycles_per_line + video_line_cycles;

    }

    // if video line is higher than visible display, assert vblank
/*
    if (cpu_verbose) {
        printf("line_num: %u\n", VID_LY);
        printf("video_vbi: %u \n", video_vbi);
    }
*/

    if (video_frame_cycles <= 0) {
        // We're done, reset & draw screen        
        VID_LY = 0x00;
        video_update_framebuffer();
        //sys_handle_joypads();
        video_frame_cycles = cycles_per_frame + video_frame_cycles;
    }


}

uint8_t video_flip_tile_byte(uint8_t src) {
    //    7 6 5 4 3 2 1 0
    // -> 1 0 3 2 5 4 7 6
    return ((src >> 0) & 1) << (7-1)
        |  ((src >> 1) & 1) << (7-0)
        |  ((src >> 2) & 1) << (7-3)
        |  ((src >> 3) & 1) << (7-2)
        |  ((src >> 4) & 1) << (7-5)
        |  ((src >> 5) & 1) << (7-4)
        |  ((src >> 6) & 1) << (7-7)
        |  ((src >> 7) & 1) << (7-6);
}

void video_draw_tile(uint16_t tileidx, int yoffset, int linexoffset, int xstart, int count, uint8_t sprites, uint8_t sprite_attr) {
    // Render pixels of a tile into a line buffer
    // tileidx: index in vram of the map
    // yoffset: y offset inside the tile,
    // linexoffset: destination x position inside line buffer
    // xstart: x position inside the tile
    // count: amount of pixels to render
    // priority: if 1, pixels will be forced, else only if previous value is 0
    // sprites: if 1, this will be treated as sprite data, i.e. unsigned tile IDs

    int16_t tile_num;
    uint8_t p1, p2; // tile bytes 1 and 2
    uint8_t priority = ~(sprite_attr & SPRITE_ATTR_PRIO); // 0 = forced priority, 1 = behind bg


    // Respect sprite VERTICAL FLIP attribute

    if (sprite_attr & SPRITE_ATTR_YFLIP)
        yoffset = 7 - yoffset;

    yoffset = yoffset << 1;


    // Get tile from map
    if (sprites == TILES_SPRITES) {
        p1 = vram[(tileidx << 4) + yoffset];
        p2 = vram[(tileidx << 4) + yoffset + 1];
    } else if (VID_LCDC & LCDC_BGWIN_TILES) {
        tile_num = vram[tileidx++];
        p1 = vram[(tile_num << 4) + yoffset];
        p2 = vram[(tile_num << 4) + yoffset + 1];
    } else {
        // signed if bit 4 of lcdc is 0, meaning 0x8800-0x97FF
        tile_num = (int8_t) vram[tileidx++];
        int16_t vram_idx = (0x1000 + (tile_num << 4) + yoffset) & 0x1FFF;
        p1 = vram[vram_idx];
        p2 = vram[vram_idx+1];
    }

    // Respect sprite HORIZONTAL FLIP attribute

    if (sprite_attr & SPRITE_ATTR_XFLIP) {
        p1 = video_flip_tile_byte(p1);
        p2 = video_flip_tile_byte(p2);
    }
    uint8_t newpixel;
    for (int i = xstart; i < count; i++) {

        newpixel =  ((p2 & (1 << (7-i))) >> (7 - i))
                |   (((p1 & (1 << (7-i))) >> (7 - i)) << 1);
        // respect priority parameter!
        if (sprites == TILES_SPRITES) {
            if ((priority || ( (!priority) && (linebuf[linexoffset] == 0x00) ) ) && newpixel) {
                linebuf[linexoffset] = newpixel;
            }
        } else {
            linebuf[linexoffset] = newpixel;
        }

        linexoffset++;
    }
}

void video_draw_line() {

#ifdef VIDEO_VERBOSE

    printf("========== Drawing LINE: %d\n", VID_LY);
#endif

    // Calculate which tile SCX and SCY corresponds to
    uint16_t tileidx;
    if (VID_LCDC & LCDC_BG_TILEMAP) {
        tileidx = 0x1c00 + (((VID_SCY + VID_LY) >> 3) << 5) + (VID_SCX >> 3);
    } else {

        tileidx = 0x1800 + (((VID_SCY + VID_LY) >> 3) << 5) + (VID_SCX >> 3);
    }

    // clear the linebuffer before drawing anything

    memset(linebuf, 0, 160);

    // Get pixel in tile to start drawing from and draw first tile

    int xstart = (VID_SCX & 0x07) ;
    int yoffset = ((VID_SCY + VID_LY) & 0x07); // Get y position in tile to start from


    // Amount  of full tiles to render changes depending on whether
    // the X coordinate is evenly divisible by 8

    int fulltiles = 160/8;

    int linexoffset = 0;

    if (xstart != 0) {
        fulltiles -=2;
        video_draw_tile(tileidx, yoffset, linexoffset, xstart, 8, TILES_BG, 0);
        linexoffset += 8;
    }

    // draw all the full tiles

    for (int h = 0; h < fulltiles; h++) {

        // index in vram of the map, y position inside the tile, x position inside line buffer (dest), x position inside the tile, amount of pixels to render, prio

        video_draw_tile (tileidx++, yoffset, linexoffset, 0, 8, TILES_BG, 0);
        linexoffset += 8;

    }

    // draw any pixels that are left

    if (xstart != 0) {
        video_draw_tile (tileidx++, yoffset, linexoffset, 0, 7-xstart+1, TILES_BG, 0);
    }


    // draw sprites

    // check OAM if there are sprites that are visible, maximum of 10

    int drawn_sprites = 0;
    int sprite_idx = 0;
    int video_tile_height = 8;      // no idea how to figure out yet whether sprites are 8 or 16 pixels high??

    struct spritedata *cursprite = (struct spritedata*) oam;

    int xcount;

    while ((drawn_sprites < 10) && (sprite_idx < (160/4))) {

        // iterate through OAM

        // in 8x8 mode, a tile is visible if current line - Y position - 16 is less than 8 (or 16 in 16 mode)
        // and it is also visible if X position != 0 and X position < 168
        if (((unsigned) (VID_LY - (cursprite->y - 16)) < video_tile_height) && (cursprite->x > 0) && (cursprite->x < 168)) {


            // printf (">>> DRAWING SPRITE IDX %d, X:%d - Y: %d - N: %d - A: %x\n", sprite_idx, cursprite->x, cursprite->y, cursprite->tile, cursprite->attr );

            // Figure out Y position
            yoffset = VID_LY - (cursprite->y - 16);

            // Figure out X position and count
            if (cursprite->x < 8) {

                // left edge of the screen
                linexoffset = 0;
                xstart = 8 - cursprite->x;
                xcount = cursprite->x;


            } else if (cursprite->x > (160 + 8 - 8)) {

                // right edge of the screen
                linexoffset = cursprite->x - 8;
                xstart = 1;
                xcount = 160 - cursprite->x;
            } else {
                // it's a normal, fully visible sprite
                linexoffset = cursprite->x - 8;
                xstart = 0;
                xcount = 8;
            }

            video_draw_tile(cursprite->tile, yoffset, linexoffset, xstart, xcount, TILES_SPRITES, cursprite->attr);

            drawn_sprites++;
        }

        cursprite++;
        sprite_idx++;
    }


    // convert line buffer to actual line

    for (int i = 0; i < 160; i++) {
        linebuf_final[i] = palette[linebuf[i]];
    }

    memcpy(&framebuffer32[160*VID_LY], linebuf_final , 160*sizeof(uint32_t));

}

void video_update_framebuffer() {

    SDL_Delay(20);

#ifdef VIDEO_VERBOSE
    printf("========== Drawing framebuffer to window ===============\n");
#endif

    video_surface = SDL_GetWindowSurface(video_window);
    memcpy(video_surface->pixels, framebuffer32, 160 * 144 * sizeof(uint32_t));

    SDL_UpdateWindowSurface(video_window);

    SDL_PumpEvents();
    sys_handle_joypads();


}
