#include "backends.h"

#define NAME "v_dosvga"

// #ifdef VIDEO_DOSVGA

#if (1)

/*
 *  Video Backend Implementation for MS-DOS Mode 13h VGA
 */

#include <stdio.h>
#include <string.h>

#include <dpmi.h>
#include <pc.h>
#include <sys/nearptr.h>

#include "trace.h"
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


char old_palette[768];
char vga_palette[768];

void v_read_palette (char *pal) {

}

int v_dosvga_init(video_config* cfg) {

    // Set display to Mode 13h

    if (__djgpp_nearptr_enable() == 0)
        return -1;

    v_read_palette(old_palette);

    __dpmi_regs regs;

    memset(&regs, 0, sizeof regs);
    regs.x.ax = 0x13;
    __dpmi_int(0x10, &regs);

    return 0;
}

void v_dosvga_deinit() {
}

static inline void v_dosvga_set_palette_rgb(uint8_t index, uint8_t color) {
    const vgacolor *new_color = &bw_palette[color];
    outportb(0x3c8, index); /* want to write */
    outportb(0x3c9, new_color->r);
    outportb(0x3c9, new_color->g);
    outportb(0x3c9, new_color->b);
}

void v_dosvga_update_palette(uint8_t pal_offset, gameboy_palette palette) {
    v_dosvga_set_palette_rgb(0+pal_offset, palette.color[0]);
    v_dosvga_set_palette_rgb(1+pal_offset, palette.color[1]);
    v_dosvga_set_palette_rgb(2+pal_offset, palette.color[2]);
    v_dosvga_set_palette_rgb(3+pal_offset, palette.color[3]);
}

void v_dosvga_write_line(int line, uint8_t *linebuf) {
    uint8_t *screen = (uint8_t *) 0xa0000 + 320 * line + __djgpp_conventional_base;
    memcpy(screen, linebuf, LCD_WIDTH);
/*    int16_t i;
    uint32_t *dst_line = &framebuffer_pixels[LCD_WIDTH * line];
    for (i = 0; i < LCD_WIDTH; i++) {
        *(dst_line++) = pal_rgb[*(linebuf++)];
    }*/
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
