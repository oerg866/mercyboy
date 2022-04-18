#pragma once

#ifndef VIDEO_H
#define VIDEO_H

#include "compat.h"
#include "backends.h"

#define LCD_WIDTH           160
#define LCD_HEIGHT          144

#define LINES_PER_FRAME     154
#define CYCLES_PER_LINE     456
#define CYCLES_PER_FRAME    LINES_PER_FRAME*CYCLES_PER_LINE

#define PRIORITY_FALSE      0
#define PRIORITY_TRUE       1

#define TILES_BG            0
#define TILES_SPRITES       1
#define TILES_WINDOW        2

#define SPRITE_ATTR_PRIO    (1<<7)
#define SPRITE_ATTR_YFLIP   (1<<6)
#define SPRITE_ATTR_XFLIP   (1<<5)
#define SPRITE_ATTR_PALETTE (1<<4)

#define VID_LCDC            ram_io[0x40]
#define VID_STAT            ram_io[0x41]
#define VID_SCY             ram_io[0x42]
#define VID_SCX             ram_io[0x43]
#define VID_LY              ram_io[0x44]
#define VID_LYC             ram_io[0x45]
#define VID_DMA             ram_io[0x46]
#define VID_BGP             ram_io[0x47]
#define VID_OBP0            ram_io[0x48]
#define VID_OBP1            ram_io[0x49]
#define VID_WY              ram_io[0x4a]
#define VID_WX              ram_io[0x4b]
#define VID_IE              ram_ie

#define LCDC_LCDEN          (1<<7)
#define LCDC_WIN_TILEMAP    (1<<6)
#define LCDC_WINEN          (1<<5)
#define LCDC_BGWIN_TILES    (1<<4)
#define LCDC_BG_TILEMAP     (1<<3)
#define LCDC_SPRITESIZE     (1<<2)
#define LCDC_SPRITEEN       (1<<1)
#define LCDC_BGWINEN        (1<<0)

#define MODE_HBL            0x00
#define MODE_VBL            0x01
#define MODE_SCAN_OAM       0x02
#define MODE_RENDERING      0x03

#define STAT_IE_LY          (1<<6)
#define STAT_IE_OAM         (1<<5)
#define STAT_IE_VBL         (1<<4)
#define STAT_IE_HBL         (1<<3)
#define STAT_COINCIDENCE    (1<<2)

// Array offsets for video_palette

#define PAL_OFFSET_BGP      0
#define PAL_OFFSET_OBP0     4
#define PAL_OFFSET_OBP1     8


void video_reset_lcd();
void video_init(video_backend_t *backend, video_config *cfg);
void video_deinit();
video_config *video_get_config();
uint8_t video_get_line();
void video_update_palette(uint8_t pal_offset, uint8_t reg);
void video_cycles(int32_t cycles);
int32_t video_get_idle_cycle_count();
uint8_t video_flip_tile_byte(uint8_t src);
void video_draw_line();
void video_ack_frame_done_and_draw();
video_backend_status video_handle_events();
int video_is_frame_done();

#endif
