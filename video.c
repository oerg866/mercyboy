#include "video.h"

#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "sys.h"
#include "audio.h"
#include "trace.h"

#include "backends.h"

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

uint32_t linebuf_final[LCD_WIDTH];
uint8_t linebuf[LCD_WIDTH];

uint8_t video_current_line = 0;

uint8_t video_sprites_xcoords_rendered[LCD_WIDTH+8];

uint8_t video_window_y_position_internal;
uint8_t video_window_y_counter_internal;
uint8_t video_window_enabled_internal;

int video_frame_done = 0;

static video_backend_t *s_video_backend = NULL;
static video_config *s_config;

struct spritedata {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attr;
};

static const unsigned char *flip_byte_lut = (const unsigned char *)
  "\x00\x80\x40\xc0\x20\xa0\x60\xe0\x10\x90\x50\xd0\x30\xb0\x70\xf0"
  "\x08\x88\x48\xc8\x28\xa8\x68\xe8\x18\x98\x58\xd8\x38\xb8\x78\xf8"
  "\x04\x84\x44\xc4\x24\xa4\x64\xe4\x14\x94\x54\xd4\x34\xb4\x74\xf4"
  "\x0c\x8c\x4c\xcc\x2c\xac\x6c\xec\x1c\x9c\x5c\xdc\x3c\xbc\x7c\xfc"
  "\x02\x82\x42\xc2\x22\xa2\x62\xe2\x12\x92\x52\xd2\x32\xb2\x72\xf2"
  "\x0a\x8a\x4a\xca\x2a\xaa\x6a\xea\x1a\x9a\x5a\xda\x3a\xba\x7a\xfa"
  "\x06\x86\x46\xc6\x26\xa6\x66\xe6\x16\x96\x56\xd6\x36\xb6\x76\xf6"
  "\x0e\x8e\x4e\xce\x2e\xae\x6e\xee\x1e\x9e\x5e\xde\x3e\xbe\x7e\xfe"
  "\x01\x81\x41\xc1\x21\xa1\x61\xe1\x11\x91\x51\xd1\x31\xb1\x71\xf1"
  "\x09\x89\x49\xc9\x29\xa9\x69\xe9\x19\x99\x59\xd9\x39\xb9\x79\xf9"
  "\x05\x85\x45\xc5\x25\xa5\x65\xe5\x15\x95\x55\xd5\x35\xb5\x75\xf5"
  "\x0d\x8d\x4d\xcd\x2d\xad\x6d\xed\x1d\x9d\x5d\xdd\x3d\xbd\x7d\xfd"
  "\x03\x83\x43\xc3\x23\xa3\x63\xe3\x13\x93\x53\xd3\x33\xb3\x73\xf3"
  "\x0b\x8b\x4b\xcb\x2b\xab\x6b\xeb\x1b\x9b\x5b\xdb\x3b\xbb\x7b\xfb"
  "\x07\x87\x47\xc7\x27\xa7\x67\xe7\x17\x97\x57\xd7\x37\xb7\x77\xf7"
  "\x0f\x8f\x4f\xcf\x2f\xaf\x6f\xef\x1f\x9f\x5f\xdf\x3f\xbf\x7f\xff";

#define video_flip_tile_byte(x) (flip_byte_lut[x])

void video_reset_lcd() {
    // Called when LCD is disabled and getting enabled again.
    video_line_cycles = cycles_per_line;
    video_frame_cycles = cycles_per_frame;
    video_current_line = 0x00;
}

void video_init(video_backend_t *video_backend, video_config *cfg) {
    s_video_backend = video_backend;
    s_config = cfg;

    s_video_backend->init(s_config);

    video_reset_lcd();

    video_frame_done = 0;

    video_window_y_position_internal = 0;
    video_window_y_counter_internal = 0;
    video_window_enabled_internal = 0;
    lines_per_frame = LINES_PER_FRAME; //154;
    cycles_per_line = CYCLES_PER_LINE; //456;
    cycles_per_frame = CYCLES_PER_FRAME; //456 * 154;
}

void video_deinit() {
    s_video_backend->deinit();
}

video_config *video_get_config() {
    return s_config;
}

uint8_t video_get_line() {
    if (!(VID_LCDC & LCDC_LCDEN)) {
        return 0;
    } else {
        return video_current_line;
    }
}

void video_update_palette(uint8_t pal_offset, uint8_t reg) {
    // Update a palette. pal_offset is offset in palette arrays to take. 0 for bgp, 1*4 for obp0, 2*4 for obp1
    gameboy_palette new_palette = { { (reg & 0x03) >> 0,
                                      (reg & 0x0C) >> 2,
                                      (reg & 0x30) >> 4,
                                      (reg & 0xC0) >> 6 } };

    // Allow video backend to update palette in its own format
    s_video_backend->update_palette(pal_offset, new_palette);
}

void video_cycles(int cycles) {

    // Handle STAT register updates
    uint8_t oldstat = VID_STAT;

    cycles = cycles >> 2; // Machine cycles, not CPU cycles
    video_line_cycles -= cycles;
    video_frame_cycles -= cycles;


    // Cycles 456 - 376 = MODE_SCAN_OAM
    // Cycles 375 - 85 = MODE_RENDERING
    // Cycles 84 - 0 = MODE_HBL

    if (VID_LCDC & LCDC_LCDEN) {
        // LCD is ENABLED, process STAT updates & interrupts
        if              (video_current_line >= 144) {

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
            video_frame_done = 1;

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

void video_draw_tile(uint16_t tileidx, int yoffset, int linexoffset, int xstart, int count, uint8_t tiles_type, uint8_t sprite_attr) {
    // Render pixels of a tile into a line buffer
    // tileidx: index in vram of the map
    // yoffset: y offset inside the tile,
    // linexoffset: destination x position inside line buffer
    // xstart: x position inside the tile
    // count: amount of pixels to render
    // tiles_type: defines if tiles drawn are BG, Sprite or Window tiles.
    // sprites_attr: Sprite attributes (Xflip, yflip, prio, etc.)

    int32_t i;

    int16_t tile_num;
    uint8_t p1, p2; // tile bytes 1 and 2
    uint8_t priority = !(sprite_attr & SPRITE_ATTR_PRIO); // 0 = forced priority, 1 = behind bg

    // Palette offset, BGP by default. Will be set for sprites further down.

    uint8_t pal_offset;

    uint8_t newpixel;

    int16_t vram_idx;

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

    // Get tile from map    // Get tile from map

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
        vram_idx = (0x1000 + (tile_num << 4) + yoffset) & 0x1FFF;
        p1 = vram[vram_idx];
        p2 = vram[vram_idx+1];
    }

    // Respect sprite HORIZONTAL FLIP attribute

    if (sprite_attr & SPRITE_ATTR_XFLIP) {
        p1 = video_flip_tile_byte(p1);
        p2 = video_flip_tile_byte(p2);
    }

    for (i = xstart; i < count; i++) {

        if (linexoffset >= LCD_WIDTH) {
            linexoffset++;
            continue;
        }

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



void video_draw_tile_bg_partial(uint16_t tileidx, int yoffset, uint8_t *linebuf_ptr, int xstart, int count) {
    // Render pixels of a tile into a line buffer
    // tileidx: index in vram of the map
    // yoffset: y offset inside the tile,
    // linexoffset: destination x position inside line buffer
    // xstart: x position inside the tile
    // count: amount of pixels to render

    int32_t i;
    uint8_t p1, p2; // tile bytes 1 and 2

    // Get tile from map
    if (VID_LCDC & LCDC_BGWIN_TILES) { // Get tile from map
        tileidx = (vram[tileidx] << 4) + yoffset + yoffset;
    } else {
        // signed if bit 4 of lcdc is 0, meaning 0x8800-0x97FF
        tileidx = ((int8_t) vram[tileidx]);
        tileidx = (0x1000 + (tileidx<< 4) + yoffset + yoffset) & 0x1FFF;
    }

    p1 = vram[tileidx++];
    p2 = vram[tileidx];

    for (i = xstart; i < count; i++) {
        *linebuf_ptr++ = ((((p2 & (1 << (7 - i))) >> (7 - i)) << 1)
                       | ((p1 & (1 << (7 - i))) >> (7 - i)));
    }
}

void video_draw_tile_bg_full(uint16_t tileidx, int yoffset, uint8_t *linebuf_ptr) {
    // Render ALL pixels of a tile into a line buffer
    // tileidx: index in vram of the map
    // yoffset: y offset inside the tile,
    // linexoffset: destination x position inside line buffer
    uint8_t p1, p2; // tile bytes 1 and 2

    if (VID_LCDC & LCDC_BGWIN_TILES) { // Get tile from map
        tileidx = (vram[tileidx] << 4) + yoffset + yoffset;
    } else {
        // signed if bit 4 of lcdc is 0, meaning 0x8800-0x97FF
        tileidx = ((int8_t) vram[tileidx]);
        tileidx = (0x1000 + (tileidx<< 4) + yoffset + yoffset) & 0x1FFF;
    }

    p1 = vram[tileidx++];
    p2 = vram[tileidx];

#define drawpixel(index)   ((((p2 & (1 << (7 - index))) >> (7 - index)) << 1) \
                         | ((p1 & (1 << (7 - index))) >> (7 - index)))

    *linebuf_ptr++ = drawpixel(0);
    *linebuf_ptr++ = drawpixel(1);
    *linebuf_ptr++ = drawpixel(2);
    *linebuf_ptr++ = drawpixel(3);
    *linebuf_ptr++ = drawpixel(4);
    *linebuf_ptr++ = drawpixel(5);
    *linebuf_ptr++ = drawpixel(6);
    *linebuf_ptr++ = drawpixel(7);
}



void video_draw_tilemap(uint16_t tileidx, int draw_x, int draw_width, uint8_t tiles_type) {

    int h;
    int xrest;
    int yoffset;

    // Amount of full tiles to render changes depending on whether
    // the X coordinate is evenly divisible by 8

    int fulltiles = draw_width/8;

    uint8_t *linebuf_ptr = &linebuf[draw_x];
    uint8_t tileidx_lower = tileidx & 0x1F;
    uint16_t tileidx_upper = tileidx & 0xFFE0;

    // Get pixel in tile to start drawing from and draw first tile

    if (tiles_type == TILES_BG) {
        // Respect Scroll X and Y for BG tiles.
        xrest = (VID_SCX & 0x07) ;
        yoffset = ((VID_SCY + video_current_line) & 0x07); // Get y position in tile to start from
    } else {
        // For Window, scroll doesn't matter.
        xrest = draw_width & 0x07;
        yoffset = (video_window_y_counter_internal & 0x07);
    }


    if ((xrest != 0) && (tiles_type == TILES_BG)) {
        fulltiles -= 1;
        tileidx = tileidx_upper | tileidx_lower;
        tileidx_lower = (tileidx_lower + 1) & 0x1F; // Wrap around after 32 tiles.
        video_draw_tile_bg_partial(tileidx, yoffset, linebuf_ptr, xrest, 8);
        linebuf_ptr += 8 - xrest;
    }

    // draw all the full tiles

    for (h = 0; h < fulltiles; h++) {
        // index in vram of the map, y position inside the tile, x position inside line buffer (dest), x position inside the tile, amount of pixels to render, prio
        tileidx = tileidx_upper | tileidx_lower;
        tileidx_lower = (tileidx_lower + 1) & 0x1F; // Wrap around after 32 tiles.
        video_draw_tile_bg_full(tileidx, yoffset, linebuf_ptr);
        linebuf_ptr += 8;
    }

    // draw any pixels that are left

    if (xrest != 0) {
        tileidx = tileidx_upper | tileidx_lower;
        video_draw_tile_bg_partial(tileidx, yoffset, linebuf_ptr, 0, xrest);
    }

}

void video_draw_tile_sprite(unsigned tileidx, unsigned yoffset, unsigned xstart, unsigned linexoffset, unsigned sprite_height, unsigned count, uint8_t sprite_attr) {
    // Render pixels of a sprite tile into a line buffer
    // tileidx: index in vram of the map
    // yoffset: y offset inside the tile,
    // xstart: x position inside the tile
    // linexoffset: destination x position inside line buffer
    // sprite_height: sprite height in pixels
    // count: amount of pixels to render
    // sprites_attr: Sprite attributes (Xflip, yflip, prio, etc.)
    uint8_t p1, p2; // tile bytes 1 and 2
    uint8_t priority = !(sprite_attr & SPRITE_ATTR_PRIO); // 0 = forced priority, 1 = behind bg
    uint8_t newpixel;
    unsigned pal_offset = (sprite_attr & SPRITE_ATTR_PALETTE) ? PAL_OFFSET_OBP1 : PAL_OFFSET_OBP0; // Palette offset for sprites.
    unsigned i;

    tileidx = (tileidx << 4) + yoffset + yoffset;

    // Respect sprite VERTICAL and HORIZONTAL FLIP attributes
    if (sprite_attr & SPRITE_ATTR_YFLIP) yoffset = (sprite_height - 1) - yoffset;
    if (sprite_attr & SPRITE_ATTR_XFLIP) {
        p1 = video_flip_tile_byte(vram[tileidx++]);
        p2 = video_flip_tile_byte(vram[tileidx++]);
    } else {
        p1 = vram[tileidx++];
        p2 = vram[tileidx];
    }

    for (i = xstart; i < count; i++) {
        newpixel =  ((((p2 & (1 << (7 - i))) >> (7 - i)) << 1)
                |   ((p1 & (1 << (7 - i))) >> (7 - i)));

        if (newpixel && (priority || !linebuf[linexoffset]))
                linebuf[linexoffset] = newpixel + pal_offset;

        linexoffset++;
    }
}

void video_draw_sprites() {
    // check OAM if there are sprites that are visible, maximum of 10

    int drawn_sprites = 0;
    int sprite_idx = 0;
    int video_tile_height = 8 + ((VID_LCDC & LCDC_SPRITESIZE) << 1);

    struct spritedata *cursprite = (struct spritedata*) oam;

    int xcount;
    int xstart;
    int yoffset;
    int linexoffset;

    memset(video_sprites_xcoords_rendered, 0, 168);

    while ((drawn_sprites < 10) && (sprite_idx < (160/4))) {

        // iterate through OAM

        // Figure out Y position
        yoffset = video_current_line - (cursprite->y - 16);

        // in 8x8 mode, a tile is visible if current line - Y position - 16 is less than 8 (or 16 in 16 mode)
        // and it is also visible if X position != 0 and X position < 168
        if ((yoffset >= 0) && (yoffset < video_tile_height) && (cursprite->x > 0) && (cursprite->x < 168)) {

            // print_msg (">>> DRAWING SPRITE IDX %d, X:%d - Y: %d - N: %d - A: %x\n", sprite_idx, cursprite->x, cursprite->y, cursprite->tile, cursprite->attr );

            // if we already rendered a sprite with the same x coordinate, we don't draw it. (thanks to dmg-acid2)

            if (!video_sprites_xcoords_rendered[cursprite->x]) {
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

                if (video_tile_height == 16)    // Ignore bit 0 for 16 pixel high objects (thanks dmg-acid2)
                    // void video_draw_tile_sprite(unsigned tileidx, unsigned yoffset, unsigned xstart, unsigned linexoffset, unsigned sprite_height, unsigned count, uint8_t sprite_attr) {
                    video_draw_tile_sprite(cursprite->tile & 0xFE, yoffset, xstart, linexoffset, video_tile_height, xcount, cursprite->attr);
                else
                    video_draw_tile_sprite(cursprite->tile, yoffset, xstart, linexoffset, video_tile_height, xcount, cursprite->attr);
                video_sprites_xcoords_rendered[cursprite->x] = 1;

                drawn_sprites++;
            }

        }

        cursprite++;
        sprite_idx++;
    }
}
void video_draw_line() {

    uint16_t tileidx;

    // Window position is offset by 7 pixels.
    uint16_t window_start = VID_WX - 7;

    trace(TRACE_VIDEO, "Drawing LINE: %d\n", video_current_line);

    // If LCD is disabled we draw nothing!

    if (!(VID_LCDC & LCDC_LCDEN)) {
        // convert line buffer to actual line inside the backend
        memset(linebuf, 0, 160);
        s_video_backend->write_line(video_current_line, linebuf);
        return;
    }

    // Calculate which tile SCX and SCY corresponds to

    tileidx = (VID_LCDC & LCDC_BG_TILEMAP) ? 0x1c00 : 0x1800;
    tileidx += (((int16_t) VID_SCY + (int16_t) video_current_line) << 2) & 0xFBE0;
    tileidx += ((int16_t) VID_SCX >> 3) & 0x3FF;

    video_draw_tilemap(tileidx, 0, 160, TILES_BG);

    // Draw window ONLY if it is enabled AND in visible range

    if (video_window_enabled_internal) {
        if ((video_window_y_position_internal <= 143) && (VID_WX <= 166)) {
            if (video_current_line >= video_window_y_position_internal) {

                // Draw window

                tileidx = (VID_LCDC & LCDC_WIN_TILEMAP) ? 0x1c00 : 0x1800;
                tileidx += ((video_window_y_counter_internal >> 3) << 5);

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

    s_video_backend->write_line(video_current_line, linebuf);

    // Update window enabled y/n

    video_window_enabled_internal = VID_LCDC & LCDC_WINEN;

}

void video_ack_frame_done_and_draw() {
    trace(TRACE_VIDEO,"Drawing framebuffer to window\n");
    s_video_backend->frame_done();
    video_frame_done = 0;
}

video_backend_status video_handle_events() {
    return s_video_backend->event_handler();
}

int video_is_frame_done() {
    return video_frame_done;
}
