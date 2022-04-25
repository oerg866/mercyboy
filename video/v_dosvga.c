#include "backends.h"

#define NAME "v_dosvga"

#ifdef VIDEO_DOSVGA

/*
 *  Video Backend Implementation for MS-DOS Mode 13h VGA
 */

#include <stdio.h>
#include <string.h>

#include <dpmi.h>
#include <pc.h>
#include <sys/nearptr.h>

#include "console.h"
#include "video.h"

#pragma pack(1)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} vgacolor;
#pragma pack()

static const vgacolor bw_palette[4] = { { 0x3F, 0x3F, 0x3F },
                                        { 0x2A, 0x2A, 0x2A },
                                        { 0x15, 0x15, 0x15 },
                                        { 0x00, 0x00, 0x00 } };

// we overwrite 4*3 + 1 colors, this is the previous palette.
#define COLOR_COUNT (4 * 3 + 1)
#define BG_COLOR_INDEX (COLOR_COUNT - 1)
static vgacolor old_palette[COLOR_COUNT];
static uint8_t old_mode = 0;

// Macro to set a color palette entry
#define _SETCOLOR(idx, r, g, b) outportb(0x3c8, idx); \
    outportb(0x3c9, r); \
    outportb(0x3c9, g); \
    outportb(0x3c9, b);

#define VGA_WIDTH 320
#define VGA_HEIGHT 200

// X and Y offsets to center the screen
#define X_OFFSET ((VGA_WIDTH-LCD_WIDTH)/2)
#define Y_OFFSET ((VGA_HEIGHT-LCD_HEIGHT)/2)

static void v_dosvga_read_palette(vgacolor *pal, uint8_t index, int length) {
    int i;
    outportb(0x3c7, index); // Signal a read from VGA pal
    for (i = 0; i < length; ++i) { // Read 3 color components at a time
        pal->r = inportb(0x3c9);
        pal->g = inportb(0x3c9);
        pal->b = inportb(0x3c9);
        pal++;
    }
}
static void v_dosvga_write_palette(vgacolor *pal, uint8_t index, int length) {
    int i;
    outportb(0x3c8, index);
    for (i = 0; i < length; ++i) {
        outportb(0x3c9, pal->r);
        outportb(0x3c9, pal->g);
        outportb(0x3c9, pal++->b);
    }
}

}

int v_dosvga_init(video_config* cfg) {
    uint8_t *screen = (uint8_t *) 0xa0000 + __djgpp_conventional_base;
    __dpmi_regs regs;

    // Enable near ptrs so we can write to conventional mem quickly
    if (__djgpp_nearptr_enable() == 0)
        return -1;

    // Read old palette so we can restore it afterwards
    v_dosvga_read_palette(old_palette, 0, COLOR_COUNT);

    // Save current video mode for later
    memset(&regs, 0x00, sizeof regs);
    regs.x.ax = 0x0F;
    __dpmi_int(0x10, &regs);

    old_mode = (uint8_t) regs.x.ax;


    // Set display to Mode 13h
    memset(&regs, 0x00, sizeof regs);
    regs.x.ax = 0x13;
    __dpmi_int(0x10, &regs);

    // Make the background guaranteed black, set palette and clear the screen
    _SETCOLOR(BG_COLOR_INDEX, 0x00, 0x00, 0x00);
    memset(screen, BG_COLOR_INDEX, 320*200);

    return 0;
}

void v_dosvga_deinit() {
    __dpmi_regs regs;

    // Restore old palette & video mode

    v_dosvga_write_palette(old_palette, 0, COLOR_COUNT);

    memset(&regs, 0x00, sizeof regs);
    regs.x.ax = old_mode;
    __dpmi_int(0x10, &regs);

    __djgpp_nearptr_disable();

    printf("%s successful!\n", __func__);
}

static inline void v_dosvga_set_palette_rgb(uint8_t index, uint8_t color) {
    const vgacolor *new_color = &bw_palette[color];
    _SETCOLOR(index, new_color->r, new_color->g, new_color->b);
}

void v_dosvga_update_palette(uint8_t pal_offset, gameboy_palette palette) {
    v_dosvga_set_palette_rgb(0+pal_offset, palette.color[0]);
    v_dosvga_set_palette_rgb(1+pal_offset, palette.color[1]);
    v_dosvga_set_palette_rgb(2+pal_offset, palette.color[2]);
    v_dosvga_set_palette_rgb(3+pal_offset, palette.color[3]);
}

void v_dosvga_write_line(int line, uint8_t *linebuf) {
    // Center the screen
    uint8_t *screen = (uint8_t *) 0xa0000 + __djgpp_conventional_base
                        + 320 * (Y_OFFSET + line)
                        + X_OFFSET;

    // Since VGA paletting works like we do, this is a memcpy :-)
    memcpy(screen, linebuf, LCD_WIDTH);
}

void v_dosvga_frame_done() {
    /* nothing for now I guess? */
}

video_backend_status v_dosvga_event_handler() {
    video_backend_status ret = VIDEO_BACKEND_RUNNING;
    return ret;
}

const video_backend_t v_dosvga = {
    NAME,                       // .name
    1,                          // .present
    v_dosvga_init,              // .init
    v_dosvga_deinit,            // .deinit
    v_dosvga_update_palette,    // .update_palette
    v_dosvga_write_line,        // .write_line
    v_dosvga_frame_done,        // .frame_done
    v_dosvga_event_handler      // .event_handler
};
#else
const video_backend_t v_dosvga = { NAME, 0 };
#endif
