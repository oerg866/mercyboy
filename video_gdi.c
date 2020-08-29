#ifdef VIDEO_GDI

#include "video.h"

/*
 *  Video Backend Implementation for GDI Win16/32 (for very old systems. Or people who like GDI I guess :P)
 *
 */

#include <stdio.h>
#include <windows.h>
//#include <gdiplus.h>

#ifdef USE_AUDIO_TIMING
#include "audio.h"
#endif

void           *framebuffer = NULL;
HANDLE          hwnd = NULL;
int             video_gdi_bpp = 0;
void           *bw_palette = NULL;

uint16_t       *pal_rgb_16 = NULL; // BGP, OBP1 and OBP2 in proper format, depending on the bit depth
uint32_t       *pal_rgb_32 = NULL;

const uint16_t  bw_palette_16[4] = {0xFFFF,0xAD55,0x632C,0x0000};
const uint32_t  bw_palette_32[4] = {0x00ffffff,0x00aaaaaa,0x00666666,0x00000000};

uint16_t       *framebuffer_16;  // These will be allocated as needed so there is not unnecessary memory usage
uint32_t       *framebuffer_32;

BITMAPINFO      bmi;

// Pointer to the palette, line and framebuffer update functions
void (*video_backend_update_palette_bpp)(uint8_t, uint8_t);
void (*video_backend_draw_line_bpp)(int, uint8_t*);


void video_backend_update_palette_16(uint8_t pal_offset, uint8_t reg);
void video_backend_update_palette_32(uint8_t pal_offset, uint8_t reg);

void video_backend_draw_line_16(int line, uint8_t *linebuf);
void video_backend_draw_line_32(int line, uint8_t *linebuf);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (uMsg)
    {
    case WM_SIZE:

        break;// Handle window resizing

    case WM_PAINT:

        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rect);
        StretchDIBits(hdc,
                      0, 0, rect.right-rect.left, rect.bottom-rect.top, // Destination Coordinates, width and height
                      0, 0, LCD_WIDTH, LCD_HEIGHT,                      // Source coordinates, width and height
                      framebuffer_32,                                   // Data ptr
                      &bmi,                                             // Bitmap information haeder
                      DIB_RGB_COLORS,                                   // RGB mode
                      SRCCOPY);                                         // Copy from source, leave it intact
        EndPaint(hwnd,&ps);
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int video_backend_init(int width, int height, int bitdepth) {

    video_gdi_bpp = bitdepth;

    if (video_gdi_bpp == 16) { // 16 bpp graphics, might be helpful for Win3.x support later
        framebuffer_16 = (uint16_t*) LocalAlloc(LPTR, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));
        pal_rgb_16 = (uint16_t*) malloc(4 * sizeof(uint16_t));
        video_backend_update_palette_bpp = &video_backend_update_palette_16;
        video_backend_draw_line_bpp = &video_backend_draw_line_16;
    } else {
        video_gdi_bpp = 32; // assume everything else is 32 bit rgb for now
        pal_rgb_32 = (uint32_t*) malloc(4 * sizeof(uint32_t));
        framebuffer_32 = (uint32_t*) malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));
        video_backend_update_palette_bpp = &video_backend_update_palette_32;
        video_backend_draw_line_bpp = &video_backend_draw_line_32;
    }

    if ((framebuffer_32 == NULL) && (framebuffer_16 == NULL)) {
        printf("Error creating internal framebuffer\n");
        return -1;
    }

    HMODULE hInstance = GetModuleHandle(NULL);

    // Make and register windowclass


    WNDCLASS wc = { };

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT( "MercyBoy" );

    RegisterClass(&wc);

    // Get the client rectangle for a 160x144 display INCLUDING border and title bar

    RECT rect;
    rect.top = 0;
    rect.bottom = LCD_HEIGHT;
    rect.left = 0;
    rect.right = LCD_WIDTH;

    AdjustWindowRect(&rect, WS_CAPTION|WS_SYSMENU|WS_SIZEBOX|WS_VISIBLE|WS_MINIMIZEBOX, FALSE);

    // Bitmap information structure

    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth = LCD_WIDTH;
    bmi.bmiHeader.biHeight = -LCD_HEIGHT;
    bmi.bmiHeader.biSizeImage=4*LCD_WIDTH*LCD_HEIGHT;

    // Create actual window

    hwnd = CreateWindow (TEXT( "MercyBoy" ), TEXT( "MercyBoy" ),
                         WS_CAPTION|WS_SYSMENU|WS_SIZEBOX|WS_VISIBLE|WS_MINIMIZEBOX|WS_OVERLAPPED,
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         rect.right-rect.left, rect.bottom-rect.top,
                         NULL, NULL, hInstance, NULL) ;

    ShowWindow(hwnd, SW_SHOW);

    if (!hwnd) {
        printf("Error opening window: %lu\n", GetLastError());
        return -1;
    }

    return 0;
}



void video_backend_update_palette_16(uint8_t pal_offset, uint8_t reg) {
    pal_rgb_16[0+pal_offset] = bw_palette_16[pal_int[0+pal_offset]];
    pal_rgb_16[1+pal_offset] = bw_palette_16[pal_int[1+pal_offset]];
    pal_rgb_16[2+pal_offset] = bw_palette_16[pal_int[2+pal_offset]];
    pal_rgb_16[3+pal_offset] = bw_palette_16[pal_int[3+pal_offset]];
}

void video_backend_update_palette_32(uint8_t pal_offset, uint8_t reg) {
    pal_rgb_32[0+pal_offset] = bw_palette_32[pal_int[0+pal_offset]];
    pal_rgb_32[1+pal_offset] = bw_palette_32[pal_int[1+pal_offset]];
    pal_rgb_32[2+pal_offset] = bw_palette_32[pal_int[2+pal_offset]];
    pal_rgb_32[3+pal_offset] = bw_palette_32[pal_int[3+pal_offset]];
}

inline void video_backend_update_palette(uint8_t pal_offset, uint8_t reg) {
    (*video_backend_update_palette_bpp) (pal_offset, reg);
}

void video_backend_draw_line_16(int line, uint8_t *linebuf) {
    for (int i = 0; i < LCD_WIDTH; i++)
        framebuffer_16[i + LCD_WIDTH*line] = pal_rgb_16[linebuf[i]];
}

void video_backend_draw_line_32(int line, uint8_t *linebuf) {
    for (int i = 0; i < LCD_WIDTH; i++)
        framebuffer_32[i + LCD_WIDTH*line] = pal_rgb_32[linebuf[i]];
}

inline void video_backend_draw_line(int line, uint8_t *linebuf) {
    (*video_backend_draw_line_bpp) (line, linebuf);
}

void video_backend_update_framebuffer() {

    // For some reason, the window surface has a different format even though it's supposed to be 32 bit rgb...

    MSG msg;

    InvalidateRgn(hwnd,0,0);
    UpdateWindow (hwnd);

    while (PeekMessage(&msg, hwnd,  0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


#ifdef USE_AUDIO_TIMING
    while (audio_timer < 0.016);
    audio_timer = audio_timer - 0.016;
#else
    Sleep(16);
#endif

}

#endif
