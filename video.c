#include "video.h"


int lines_per_frame = 154;
int cycles_per_line = 456;
int cycles_per_frame = 456 * 154;

int video_line_cycles = 0;
int video_frame_cycles = 0;
int video_line_num = 0;

SDL_Surface * video_surface;
SDL_Window * video_window;

uint8_t video_vbi = 0;

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

#define VID_LCDC    ram_io[0x40]
#define VID_SCY     ram_io[0x42]
#define VID_SCX     ram_io[0x43]
#define VID_LY      ram_io[0x44]
#define VID_LYC     ram_io[0x45]
#define VID_DMA     ram_io[0x46]
#define VID_BGP     ram_io[0x47]
#define VID_OBP0    ram_io[0x48]
#define VID_OBP1    ram_io[0x49]
#define VID_WY      ram_io[0x4a]
#define VID_WX      ram_io[0x4b]
#define VID_IE      ram_ie

#define vid_vbi     cpu_ints[0]

#define LCDC_LCDEN          (1<<7)
#define LCDC_WIN_TILEMAP    (1<<6)
#define LCDC_WINEN          (1<<5)
#define LCDC_BGWIN_TILES    (1<<4)
#define LCDC_BG_TILEMAP     (1<<3)
#define LCDC_SPRITESIZE     (1<<2)
#define LCDC_SPRITEEN       (1<<1)
#define LCDC_BGWINEN        (1<<0)


#define video_vbi cpu_ints[0]

//uint8_t     framebuffer [160*144*4];
uint32_t    framebuffer32 [160*144];


uint32_t linebuf_final[160];
uint8_t linebuf[160];

uint32_t     palette[4] = {0x00ffffff,0x00aaaaaa,0x00555555,0x00000000};

void video_init(SDL_Surface *init_surface, SDL_Window *init_window) {
    video_line_cycles = cycles_per_line;
    video_frame_cycles = cycles_per_frame;
//    framebuffer32 = (uint32_t*) framebuffer;
    video_surface = init_surface;
    video_window = init_window;
    VID_LY = 0x00;
}

void video_cycles(int cycles) {
    cycles = cycles >> 2; // Machine cycles, not CPU cycles
    video_line_cycles -= cycles;
    video_frame_cycles -= cycles;

    if (video_line_cycles <= 0) {


        if (video_line_num < 144) {
            // draw current line if we're in active display
            video_draw_line();
        }

        video_line_num++;
        VID_LY = video_line_num;

        video_line_cycles = cycles_per_line + video_line_cycles;

    }

    // if video line is higher than visible display, assert vblank
/*
    if (cpu_verbose) {
        printf("line_num: %u\n", VID_LY);
        printf("video_vbi: %u \n", video_vbi);
    }
*/
    if ((video_line_num >= 144) && (video_vbi != INT_SERVICED)) {
        video_vbi = INT_PENDING;
    } else if (video_line_num == 0) {
        if (video_vbi == INT_SERVICED) video_vbi = INT_NONE;
    }

    if (video_frame_cycles <= 0) {
        // We're done, reset & draw screen
        video_line_num = 0;
        VID_LY = 0x00;
//        video_vbi = INT_NONE;
        video_update_framebuffer();
        //sys_handle_joypads();
        video_frame_cycles = cycles_per_frame + video_frame_cycles;
    }


}

void video_draw_line() {


    // Calculate which tile SCX and SCY corresponds to
    uint16_t tileidx;
    if (VID_LCDC & LCDC_BG_TILEMAP) {
        tileidx = 0x1c00 + (((VID_SCY + video_line_num) >> 3) << 5) + (VID_SCX >> 3);
    } else {

        tileidx = 0x1800 + (((VID_SCY + video_line_num) >> 3) << 5) + (VID_SCX >> 3);
    }


    int pixels_to_draw = 160;

    // Get pixel in tile to start drawing from and draw first tile

    uint8_t tilerow[8];

    int xstart = (VID_SCX & 0x07) ;
    int linestart = ((VID_SCY + video_line_num) & 0x07) << 1; // Get byte offset in tile to start drawing from


    // Amount  of full tiles to render changes depending on whether
    // the X coordinate is evenly divisible by 8

    int fulltiles = 160/8;

    int lineidx = 0;

    int16_t tile_num;
    uint8_t p1;// = vram[tileidx+linestart];
    uint8_t p2;// = vram[tileidx+linestart+1];

    int16_t vram_idx;
    if (xstart != 0) {
        fulltiles -=2;


        // Get tile from map
        if (VID_LCDC & LCDC_BGWIN_TILES) {
            tile_num = vram[tileidx++];
            p1 = vram[(tile_num << 4) + linestart];
            p2 = vram[(tile_num << 4) + linestart + 1];
        } else {
            // signed if bit 4 of lcdc is 0, meaning 0x8800-0x97FF
            tile_num = (int8_t) vram[tileidx++];
            vram_idx = (0x1000 + (tile_num << 4) + linestart) & 0x1FFF;
            p1 = vram[vram_idx];
            p2 = vram[vram_idx+1];


        }



        for (int i = xstart; i < 8; i++) {
            linebuf[lineidx++] =
                    ((p2 & (1 << (7-i))) >> (7 - i))
                |   (((p1 & (1 << (7-i))) >> (7 - i)) << 1);
        }



    }

    // draw all the full tiles

    for (int h = 0; h < fulltiles; h++) {

        // Get tile from map
        if (VID_LCDC & LCDC_BGWIN_TILES) {
            tile_num = vram[tileidx++];
            p1 = vram[(tile_num << 4) + linestart];
            p2 = vram[(tile_num << 4) + linestart + 1];
        } else {
            // signed if bit 4 of lcdc is 0, meaning 0x8800-0x97FF
            tile_num = (int8_t) vram[tileidx++];
            vram_idx = (0x1000 + (tile_num << 4) + linestart) & 0x1FFF;
            p1 = vram[vram_idx];
            p2 = vram[vram_idx+1];
        }

        for (int i = 0; i < 8; i++) {
            linebuf[lineidx++] =
                        ((p2 & (1 << (7-i))) >> (7 - i))
                    |   (((p1 & (1 << (7-i))) >> (7 - i)) << 1);
        }
    }

    // draw any pixels that are left

    if (xstart != 0) {


        // Get tile from map
        if (VID_LCDC & LCDC_BGWIN_TILES) {
            tile_num = vram[tileidx++];
            p1 = vram[(tile_num << 4) + linestart];
            p2 = vram[(tile_num << 4) + linestart + 1];
        } else {
            // signed if bit 4 of lcdc is 0, meaning 0x8800-0x97FF
            tile_num = (int8_t) vram[tileidx++];
            vram_idx = (0x1000 + (tile_num << 4) + linestart) & 0x1FFF;
            p1 = vram[vram_idx];
            p2 = vram[vram_idx+1];
        }

        for (int i = 0; i <= (7 - xstart); i++) {
            linebuf[lineidx++] =
                    ((p2 & (1 << (7-i))) >> (7 - i))
                |   (((p1 & (1 << (7-i))) >> (7 - i)) << 1);

        }

    }


    // convert line buffer to actual line

    for (int i = 0; i < 160; i++) {
        linebuf_final[i] = palette[linebuf[i]];
    }
    memcpy(&framebuffer32[160*video_line_num], linebuf_final , 160*sizeof(uint32_t));

}

void video_update_framebuffer() {

    SDL_Delay(20);
    video_surface = SDL_GetWindowSurface(video_window);
    memcpy(video_surface->pixels, framebuffer32, 160 * 144 * sizeof(uint32_t));

    SDL_UpdateWindowSurface(video_window);

    SDL_PumpEvents();
    sys_handle_joypads();


}
