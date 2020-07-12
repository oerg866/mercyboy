#include "video.h"

#include <stdio.h>
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

    // Handle STAT register updates
    uint8_t oldstat = VID_STAT;

    // Cycles 456 - 376 = MODE_SCAN_OAM
    // Cycles 375 - 85 = MODE_RENDERING
    // Cycles 84 - 0 = MODE_HBL

    if (VID_LY >=144) {

        // If we are outside of visible display, we are in VBL period
        VID_STAT = MODE_VBL;

    } else {

        if          (video_line_cycles >= 376) {

            VID_STAT = MODE_SCAN_OAM;
            // Do we need to fire an OAM interrupt?
            if ((oldstat & STAT_IE_OAM) && ((oldstat & 0x03) != MODE_SCAN_OAM))
                sys_interrupt_req(INT_LCD);

        } else if   (video_line_cycles >= 85) {

            VID_STAT = MODE_RENDERING;

        } else {

            // Do we need to fire an OAM interrupt?
            if ((oldstat & STAT_IE_HBL) && ((oldstat & 0x03) != MODE_HBL))
                sys_interrupt_req(INT_LCD);
            VID_STAT = MODE_HBL;

        }

    }

    VID_STAT |= (oldstat & 0xFC);

    if (video_line_cycles <= 0) {

        if (VID_LY < 143) {
            // draw current line if we're in active display
            video_draw_line();
        } else if (VID_LY == 143) {
            // request vblank interrupt
            sys_interrupt_req(INT_VBI);
            // for some reason STAT can also trigger a int on vblank
            if (VID_STAT & STAT_IE_VBL)
                sys_interrupt_req(INT_LCD);
        }

        VID_LY++;

        // update STAT register coincidence flag
        if (VID_LYC == VID_LY) {

            VID_STAT |= STAT_COINCIDENCE;

            // if LY = LYC and interrupt for that is enabled, request it
            if (VID_STAT & STAT_IE_LY)
                sys_interrupt_req(INT_LCD);

        } else {

            VID_STAT &= ~STAT_COINCIDENCE;

        }


        video_line_cycles = cycles_per_line + video_line_cycles;

    }

    // if video line is higher than visible display, assert vblank

    if (video_frame_cycles <= 0) {
#ifdef VIDEO_VERBOSE
        printf("SCX: %02x, SCY: %02x\n", VID_SCX, VID_SCY);
#endif
        // We're done, reset & draw screen
        VID_LY = 0x00;
        video_update_framebuffer();
        //sys_handle_joypads();
        video_frame_cycles = cycles_per_frame + video_frame_cycles;
    }


}

uint8_t video_flip_tile_byte(uint8_t src) {
    return ((src >> 0) & 1) << (7-0)
        |  ((src >> 1) & 1) << (7-1)
        |  ((src >> 2) & 1) << (7-2)
        |  ((src >> 3) & 1) << (7-3)
        |  ((src >> 4) & 1) << (7-4)
        |  ((src >> 5) & 1) << (7-5)
        |  ((src >> 6) & 1) << (7-6)
        |  ((src >> 7) & 1) << (7-7);
}

void video_draw_tile(uint16_t tileidx, int yoffset, int linexoffset, int xstart, int count, uint8_t tiles_type, uint8_t sprite_attr) {
    // Render pixels of a tile into a line buffer
    // tileidx: index in vram of the map
    // yoffset: y offset inside the tile,
    // linexoffset: destination x position inside line buffer
    // xstart: x position inside the tile
    // count: amount of pixels to render
    // tiles_type: defines if tiles drawn are BG, Sprite or Window tiles.
    // sprites_attr: Sprite attributes (Xflip, yflip, prio, etc.)

    int16_t tile_num;
    uint8_t p1, p2; // tile bytes 1 and 2
    uint8_t priority = ~(sprite_attr & SPRITE_ATTR_PRIO); // 0 = forced priority, 1 = behind bg


    // Respect sprite VERTICAL FLIP attribute

    if (sprite_attr & SPRITE_ATTR_YFLIP)
        yoffset = 7 - yoffset;

    yoffset = yoffset << 1;


    // Get tile from map
    if (tiles_type == TILES_SPRITES) {
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
        if (tiles_type == TILES_SPRITES) {
            if ((priority || ( (!priority) && (linebuf[linexoffset] == 0x00) ) ) && newpixel) {
                linebuf[linexoffset] = newpixel;
            }
        } else {
            linebuf[linexoffset] = newpixel;
        }

        linexoffset++;
    }
}

void video_draw_tilemap(uint16_t tileidx, int draw_x, int draw_width, uint8_t tiles_type) {

    // Get pixel in tile to start drawing from and draw first tile

    int xstart;
    int yoffset;

    if (tiles_type == TILES_BG) {
        // Respect Scroll X and Y for BG tiles.
        xstart = (VID_SCX & 0x07) ;
        yoffset = ((VID_SCY + VID_LY) & 0x07); // Get y position in tile to start from
    } else {
        // For Window, scroll doesn't matter.
        xstart = 0;
        yoffset = (VID_LY & 0x07);
    }


    // Amount of full tiles to render changes depending on whether
    // the X coordinate is evenly divisible by 8

    int fulltiles = draw_width/8;

    int linexoffset = draw_x;


    uint8_t tileidx_lower = tileidx & 0x1F;
    uint16_t tileidx_upper = tileidx & 0xFFE0;

    if (xstart != 0) {
        fulltiles -= 1;
        // Wrap around after 32 tiles.
        tileidx = tileidx_upper | tileidx_lower;
        tileidx_lower = (tileidx_lower + 1) & 0x1F;
        video_draw_tile(tileidx, yoffset, linexoffset, xstart, 8, tiles_type, 0);
        linexoffset += 8 - xstart;
    }

    // draw all the full tiles

    for (int h = 0; h < fulltiles; h++) {

        // index in vram of the map, y position inside the tile, x position inside line buffer (dest), x position inside the tile, amount of pixels to render, prio

        // Wrap around after 32 tiles.
        tileidx = tileidx_upper | tileidx_lower;
        tileidx_lower = (tileidx_lower + 1) & 0x1F;
        video_draw_tile (tileidx, yoffset, linexoffset, 0, 8, tiles_type, 0);
        linexoffset += 8;

    }

    // draw any pixels that are left

    if (xstart != 0) {
        tileidx = tileidx_upper | tileidx_lower;
        video_draw_tile (tileidx, yoffset, linexoffset, 0, xstart, tiles_type, 0);
    }

}

void video_draw_sprites() {
    // check OAM if there are sprites that are visible, maximum of 10

    int drawn_sprites = 0;
    int sprite_idx = 0;
    int video_tile_height = 8;      // no idea how to figure out yet whether sprites are 8 or 16 pixels high??

    struct spritedata *cursprite = (struct spritedata*) oam;

    int xcount;
    int xstart;
    int yoffset;
    int linexoffset;

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
                xcount = 8;


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
}
void video_draw_line() {

#ifdef VIDEO_VERBOSE

    printf("========== Drawing LINE: %d\n", VID_LY);
#endif

    // clear the linebuffer before drawing anything

    memset(linebuf, 0, 160);

    // Calculate which tile SCX and SCY corresponds to

    uint16_t tileidx;

    if (VID_LCDC & LCDC_BG_TILEMAP) {
        tileidx = 0x1c00 + (((VID_SCY + VID_LY) >> 3) << 5) + (VID_SCX >> 3);
    } else {

        tileidx = 0x1800 + (((VID_SCY + VID_LY) >> 3) << 5) + (VID_SCX >> 3);
    }

    video_draw_tilemap(tileidx, 0, 160, TILES_BG);

    video_draw_sprites();

    // Draw window ONLY if it is enabled AND in visible range

    if ((VID_LCDC & LCDC_WINEN) && (VID_LY >= VID_WY)) {

        // Draw window

        if (VID_LCDC & LCDC_WIN_TILEMAP) {
            tileidx = 0x1c00 + (((VID_WY + VID_LY) >> 3) << 5);
        } else {

            tileidx = 0x1800 + (((VID_WY + VID_LY) >> 3) << 5);
        }

        uint16_t window_start = VID_WX - 7;

#ifdef VIDEO_VERBOSE
        printf("VIDEO: Drawing Window WX: %d WY: %d\n", VID_WX, VID_WY);
#endif
        video_draw_tilemap(tileidx, window_start, 160-window_start, TILES_WINDOW);

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
