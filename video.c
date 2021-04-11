#include "video.h"

#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "sys.h"
#include "audio.h"
#include "trace.h"

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

int lines_per_frame;
int cycles_per_line;
int cycles_per_frame;

int video_line_cycles;
int video_frame_cycles;

uint32_t framebuffer32 [160*144];

uint32_t linebuf_final[160];
uint8_t linebuf[160];

uint8_t pal_int[4*3] = {3,2,1,0, 3,2,1,0, 3,2,1,0};

uint8_t video_current_line = 0;

uint8_t video_sprites_xcoords_rendered[168];

uint8_t video_window_y_position_internal;
uint8_t video_window_y_counter_internal;
uint8_t video_window_enabled_internal;

void video_reset_lcd() {
    // Called when LCD is disabled and getting enabled again.
    video_line_cycles = cycles_per_line;
    video_frame_cycles = cycles_per_frame;
    video_current_line = 0x00;
}

void video_init() {
    video_reset_lcd();
    video_window_y_position_internal = 0;
    video_window_y_counter_internal = 0;
    video_window_enabled_internal = 0;
    lines_per_frame = LINES_PER_FRAME; //154;
    cycles_per_line = CYCLES_PER_LINE; //456;
    cycles_per_frame = CYCLES_PER_FRAME; //456 * 154;
}

inline uint8_t video_get_line() {
    if (!(VID_LCDC & LCDC_LCDEN)) {
        return 0;
    } else {
        return video_current_line;
    }
}

void video_update_palette(uint8_t pal_offset, uint8_t reg) {
    // Update a palette. pal_offset is offset in palette arrays to take. 0 for bgp, 1*4 for obp0, 2*4 for obp1
    pal_int[pal_offset+0] = (reg & 0x03) >> 0;
    pal_int[pal_offset+1] = (reg & 0x0C) >> 2;
    pal_int[pal_offset+2] = (reg & 0x30) >> 4;
    pal_int[pal_offset+3] = (reg & 0xC0) >> 6;

    // Allow video backend to update palette in its own format
    video_backend_update_palette(pal_offset, reg);
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

    if (VID_LCDC & LCDC_LCDEN) {
        // LCD is ENABLED, process STAT updates & interrupts
        if (video_current_line >=144) {

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
    } else {
        // LCD is DISABLED - Mode 0, do nothing, no interrupts
        VID_STAT = MODE_HBL;
    }

    VID_STAT |= (oldstat & 0xFC);

    if (video_line_cycles <= 0) {

        if (video_current_line < 143) {

            // draw current line if we're in active display
            video_draw_line();

        } else if (video_current_line == 143) {

            // draw last line of active display
            video_draw_line();

            // last line was drawn, time to update the framebuffer and req interrupts
            video_update_framebuffer();

            // request vblank interrupt if LCD is enabled
            if (VID_LCDC & LCDC_LCDEN) {
                sys_interrupt_req(INT_VBI);

                // for some reason STAT can also trigger a int on vblank
                if (VID_STAT & STAT_IE_VBL)
                    sys_interrupt_req(INT_LCD);
            }
        }

        video_current_line++;

        if (VID_LCDC & LCDC_LCDEN) {
            // If LCD is enabled, update STAT register coincidence flag
            if (VID_LYC == video_current_line) {

                VID_STAT |= STAT_COINCIDENCE;

                // if LY = LYC and interrupt for that is enabled, request it
                if (VID_STAT & STAT_IE_LY)
                    sys_interrupt_req(INT_LCD);

            } else {

                VID_STAT &= ~STAT_COINCIDENCE;

            }
        }

        video_line_cycles = cycles_per_line + video_line_cycles;

    }

    // if video line is higher than visible display, assert vblank

    if (video_frame_cycles <= 0) {
        trace(TRACE_VIDEO, "SCX: %02x, SCY: %02x\n", VID_SCX, VID_SCY);

        // We're done, reset & draw screen
        video_current_line = 0x00;
        video_frame_cycles = cycles_per_frame + video_frame_cycles;

        // Buffer window Y position
        video_window_y_counter_internal = 0;
        video_window_y_position_internal = VID_WY;
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
    uint8_t priority = !(sprite_attr & SPRITE_ATTR_PRIO); // 0 = forced priority, 1 = behind bg

    // Respect sprite VERTICAL FLIP attribute

    if (tiles_type == TILES_SPRITES) {
        int sprite_height = 8 + ((VID_LCDC & LCDC_SPRITESIZE) << 1);
        if (sprite_attr & SPRITE_ATTR_YFLIP) {
            yoffset = (sprite_height - 1) - yoffset;
        }
    } else {
        // If BG/Window isn't enabled in LCDC, we blank it and leave.
        if (!(VID_LCDC & LCDC_BGWINEN)) {
            memset(linebuf, 0, 160);
            return;
        }
    }

    yoffset = yoffset << 1;

    // Palette offset, BGP by default. Will be set for sprites further down.

    uint8_t pal_offset;

    // Get tile from map

    if (tiles_type == TILES_SPRITES) {

        // For sprites set the palette offset.
        if (sprite_attr & SPRITE_ATTR_PALETTE) {
            pal_offset = PAL_OFFSET_OBP1;
        } else {
            pal_offset = PAL_OFFSET_OBP0;
        }

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

        newpixel =  ((((p2 & (1 << (7 - i))) >> (7 - i)) << 1)
                |   ((p1 & (1 << (7 - i))) >> (7 - i)));

        // respect priority parameter!
        if (tiles_type == TILES_SPRITES) {           
            if (newpixel) {
                if (priority || (!linebuf[linexoffset])) {
                    linebuf[linexoffset] = newpixel + pal_offset;
                }
            }
        } else {
            linebuf[linexoffset] = newpixel;
        }

        linexoffset++;
    }
}

void video_draw_tilemap(uint16_t tileidx, int draw_x, int draw_width, uint8_t tiles_type) {

    // Get pixel in tile to start drawing from and draw first tile

    int xrest;
    int yoffset;

    if (tiles_type == TILES_BG) {
        // Respect Scroll X and Y for BG tiles.
        xrest = (VID_SCX & 0x07) ;
        yoffset = ((VID_SCY + video_current_line) & 0x07); // Get y position in tile to start from
    } else {
        // For Window, scroll doesn't matter.
        xrest = draw_width & 0x07;
        yoffset = (video_window_y_counter_internal & 0x07);
    }


    // Amount of full tiles to render changes depending on whether
    // the X coordinate is evenly divisible by 8

    int fulltiles = draw_width/8;

    int linexoffset = draw_x;


    uint8_t tileidx_lower = tileidx & 0x1F;
    uint16_t tileidx_upper = tileidx & 0xFFE0;

    if ((xrest != 0) && (tiles_type == TILES_BG)) {
        // Wrap around after 32 tiles.
        fulltiles -= 1;
        tileidx = tileidx_upper | tileidx_lower;
        tileidx_lower = (tileidx_lower + 1) & 0x1F;
        video_draw_tile(tileidx, yoffset, linexoffset, xrest, 8, tiles_type, 0);
        linexoffset += 8 - xrest;
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

    if (xrest != 0) {
        tileidx = tileidx_upper | tileidx_lower;
        video_draw_tile (tileidx, yoffset, linexoffset, 0, xrest, tiles_type, 0);
    }

}

void video_draw_sprites() {
    // check OAM if there are sprites that are visible, maximum of 10

    int drawn_sprites = 0;
    int sprite_idx = 0;
    int video_tile_height = 8 + ((VID_LCDC & LCDC_SPRITESIZE) << 1);

    memset(video_sprites_xcoords_rendered, 0, 168);

    struct spritedata *cursprite = (struct spritedata*) oam;

    int xcount;
    int xstart;
    int yoffset;
    int linexoffset;

    while ((drawn_sprites < 10) && (sprite_idx < (160/4))) {

        // iterate through OAM

        // Figure out Y position
        yoffset = video_current_line - (cursprite->y - 16);

        // in 8x8 mode, a tile is visible if current line - Y position - 16 is less than 8 (or 16 in 16 mode)
        // and it is also visible if X position != 0 and X position < 168
        if (((unsigned) (yoffset) < video_tile_height) && (cursprite->x > 0) && (cursprite->x < 168)) {

            // printf (">>> DRAWING SPRITE IDX %d, X:%d - Y: %d - N: %d - A: %x\n", sprite_idx, cursprite->x, cursprite->y, cursprite->tile, cursprite->attr );

            // if we already rendered a sprite with the same x coordinate, we don't draw it. (thanks to dmg-acid2)

            if (video_sprites_xcoords_rendered[cursprite->x])
                break;

            // Figure out X position and count
            if (cursprite->x < 8) {

                // left edge of the screen
                linexoffset = 0;
                xstart = 8 - cursprite->x;
                xcount = 8;


            } else if (cursprite->x > (160 + 8 - 8)) {

                // right edge of the screen
                linexoffset = cursprite->x - 8;
                xstart = 0;
                xcount = 168 - cursprite->x;
            } else {
                // it's a normal, fully visible sprite
                linexoffset = cursprite->x - 8;
                xstart = 0;
                xcount = 8;
            }

            // We don't need to take care of the tile ID for 16-pixel-tall sprites because
            // the offset will be big enough to just reach into the correct tile's pixel data anyway.
            video_draw_tile(cursprite->tile, yoffset, linexoffset, xstart, xcount, TILES_SPRITES, cursprite->attr);
            video_sprites_xcoords_rendered[cursprite->x] = 1;

            drawn_sprites++;
        }

        cursprite++;
        sprite_idx++;
    }
}
void video_draw_line() {

    trace(TRACE_VIDEO, "Drawing LINE: %d\n", video_current_line);

    // clear the linebuffer before drawing anything

    memset(linebuf, 0, 160);

    // If LCD is disabled we draw nothing!

    if (!(VID_LCDC & LCDC_LCDEN)) {
        // convert line buffer to actual line inside the backend
        video_backend_draw_line(video_current_line, linebuf);
        return;
    }

    // Calculate which tile SCX and SCY corresponds to

    uint16_t tileidx;

    if (VID_LCDC & LCDC_BG_TILEMAP) {
        tileidx = 0x1c00;
    } else {

        tileidx = 0x1800;
    }

    tileidx += (((int16_t) VID_SCY + (int16_t) video_current_line) << 2) & 0xFBE0;
    tileidx += ((int16_t) VID_SCX >> 3) & 0x3FF;

    video_draw_tilemap(tileidx, 0, 160, TILES_BG);

    uint8_t window_visible = 0;

    if (video_window_enabled_internal) {
        if ((video_window_y_position_internal <= 143) && (VID_WX <= 166)) {

            window_visible = 1;

            // Draw window ONLY if it is enabled AND in visible range

            if (window_visible && (video_current_line >= video_window_y_position_internal)) {

                // Draw window

                if (VID_LCDC & LCDC_WIN_TILEMAP) {
                    tileidx = 0x1c00;
                } else {

                    tileidx = 0x1800;
                }
                tileidx += ((video_window_y_counter_internal >> 3) << 5);


                // Window position is offset by 7 pixels.
                uint16_t window_start = VID_WX - 7;

                trace(TRACE_VIDEO, "VIDEO: Drawing Window WX: %d WY: %d\n", VID_WX, video_window_y_position_internal);
                video_draw_tilemap(tileidx, window_start, 160-window_start, TILES_WINDOW);

                // If window is hidden by X/Y coordinates or disabled, then the internal line counter doesn't update.

                video_window_y_counter_internal++;

            }

        }

    }

    // Draw sprites if they are enabled

    if (VID_LCDC & LCDC_SPRITEEN)
        video_draw_sprites();


    // convert line buffer to actual line inside the backend

    video_backend_draw_line(video_current_line, linebuf);

    // Update window enabled y/n

    video_window_enabled_internal = VID_LCDC & LCDC_WINEN;

}

void video_update_framebuffer() {

    trace(TRACE_VIDEO,"Drawing framebuffer to window\n");

    video_backend_update_framebuffer();

    video_backend_handle_events();
    sys_handle_system();
    sys_handle_joypad();
    if (video_backend_get_status() == VID_BACKEND_STATUS_EXIT) {
        sys_running = 0;
    }

}
