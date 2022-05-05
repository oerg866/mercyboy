#include "backends.h"

#define NAME "v_gdi"

#ifdef VIDEO_GDI

#include "compat.h"
#include "video.h"
#include "trace.h"
#include "console.h"

/*
 *  Video Backend Implementation for GDI Win16/32 (for very old systems. Or people who like GDI I guess :P)
 *
 */

#include <windows.h>

#define FRAME_COUNT_INTERVAL 60

HANDLE hwnd = NULL;
static BITMAPINFO bmi;

static const uint16_t bw_palette_16[4] = {0xFFFF, 0xAD55, 0x632C, 0x0000};
static const uint32_t bw_palette_32[4] = {0x00ffffff, 0x00aaaaaa, 0x00666666, 0x00000000};

static uint32_t s_last_frame_count_time = 0;
static uint16_t frame_count = 0;


union framebuffer
{
    uint32_t *pixels32;
    uint16_t *pixels16;
    uint8_t *pixels8;
};

static union framebuffer s_framebuffer;

// BGP, OBP1 and OBP2 in proper format, depending on the bit depth

union palette
{
    uint32_t *pal32;
    uint16_t *pal16;
    uint8_t *pal8;
};

static union palette s_palette;

static video_config *s_video_config;

static video_backend_status s_status;

int v_gdi_init(video_config *cfg);
void v_gdi_deinit(void) { /* dummy */ }
void v_gdi_update_palette_16bpp(uint8_t pal_offset, gameboy_palette palette);
void v_gdi_update_palette_32bpp(uint8_t pal_offset, gameboy_palette palette);
void v_gdi_write_line_32bpp(int line, uint8_t *linebuf);
void v_gdi_write_line_16bpp(int line, uint8_t *linebuf);
void v_gdi_frame_done(void);
video_backend_status v_gdi_event_handler(void);

video_backend_t v_gdi =
// NOT const because we overwrite some pointers if we're in 16bpp mode ...
{
    NAME,                       // .name
    1,                          // .present
    v_gdi_init,                 // .init
    v_gdi_deinit,               // .deinit
    v_gdi_update_palette_32bpp, // .update_palette
    v_gdi_write_line_32bpp,     // .write_line
    v_gdi_frame_done,           // .frame_done
    v_gdi_event_handler         // .event_handler
};

static char s_fps_text[256];

void doFPScount() {

#ifndef __WIN16__
    uint32_t current_time = GetTickCount ();
    double fps;

    fps = 1.0 / ((double) (current_time - s_last_frame_count_time) / 1000 / (double) FRAME_COUNT_INTERVAL);
    s_last_frame_count_time = current_time;
    sprintf(s_fps_text, "MercyBoy: FPS: %2f", fps);
    SetWindowText(hwnd, s_fps_text);
#endif

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (uMsg)
    {
    case WM_SIZE:

        break; // Handle window resizing

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rect);
        StretchDIBits(hdc,
                        0, 0, rect.right - rect.left, rect.bottom - rect.top, // Destination Coordinates, width and height
                        0, 0, LCD_WIDTH, LCD_HEIGHT,                          // Source coordinates, width and height
                        s_framebuffer.pixels32,                               // Data ptr
                        &bmi,                                                 // Bitmap information haeder
                        DIB_RGB_COLORS,                                       // RGB mode
                        SRCCOPY);                                             // Copy from source, leave it intact
//        SetWindowText(hwnd, TEXT())
//        doFPScount();
        EndPaint(hwnd, &ps);
        break;

    case WM_DESTROY:
        s_status = VIDEO_BACKEND_EXIT;
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int v_gdi_init(video_config *cfg)
{

    // Initialize framebuffers and palettes

    HMODULE hInstance;
    WNDCLASS wc;
    RECT rect;

    s_video_config = cfg;

    switch (cfg->bpp)
    {
    case 16:
        s_framebuffer.pixels16 = (uint16_t *)alloc_mem(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));
        s_palette.pal16 = (uint16_t *)alloc_mem(4 * 3 *sizeof(uint16_t));
        v_gdi.update_palette = v_gdi_update_palette_16bpp;
        v_gdi.write_line = v_gdi_write_line_16bpp;
        break;
    case 32:
        s_framebuffer.pixels32 = (uint32_t *)alloc_mem(LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));
        s_palette.pal32 = (uint32_t *)alloc_mem(4 * 3 * sizeof(uint32_t));
        v_gdi.update_palette = v_gdi_update_palette_32bpp;
        v_gdi.write_line = v_gdi_write_line_32bpp;
        break;
    default:
        print_msg("Unsupported bit depth!");
        return -1;
    }

    hInstance = GetModuleHandle(NULL);

    // Make and register windowclass

    ZeroMemory(&wc, sizeof(WNDCLASS));

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("MercyBoy");

    RegisterClass(&wc);

    // Get the client rectangle for a display INCLUDING border and title bar

    rect.top = 0;
    rect.bottom = cfg->screen_height;
    rect.left = 0;
    rect.right = cfg->screen_width;

    AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_VISIBLE | WS_MINIMIZEBOX, FALSE);

    // Bitmap information structure

    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = cfg->bpp;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth = LCD_WIDTH;
    bmi.bmiHeader.biHeight = -LCD_HEIGHT;
    bmi.bmiHeader.biSizeImage = (cfg->bpp / 8) * LCD_WIDTH * LCD_HEIGHT;

    // Create actual window

    hwnd = CreateWindow(TEXT("MercyBoy"), TEXT("MercyBoy"),
                        WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_VISIBLE | WS_MINIMIZEBOX | WS_OVERLAPPED,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        rect.right - rect.left, rect.bottom - rect.top,
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);

    if (!hwnd)
    {
        print_msg("Error opening window: %lu\n", GetLastError());
        return -1;
    }

    s_status = VIDEO_BACKEND_RUNNING;

    return 0;
}

/* update_palette */

void v_gdi_update_palette_32bpp(uint8_t pal_offset, gameboy_palette palette)
{
    s_palette.pal32[0 + pal_offset] = bw_palette_32[palette.color[0]];
    s_palette.pal32[1 + pal_offset] = bw_palette_32[palette.color[1]];
    s_palette.pal32[2 + pal_offset] = bw_palette_32[palette.color[2]];
    s_palette.pal32[3 + pal_offset] = bw_palette_32[palette.color[3]];
}

void v_gdi_update_palette_16bpp(uint8_t pal_offset, gameboy_palette palette)
{
    s_palette.pal16[0 + pal_offset] = bw_palette_16[palette.color[0]];
    s_palette.pal16[1 + pal_offset] = bw_palette_16[palette.color[1]];
    s_palette.pal16[2 + pal_offset] = bw_palette_16[palette.color[2]];
    s_palette.pal16[3 + pal_offset] = bw_palette_16[palette.color[3]];
}

/* write_line */

void v_gdi_write_line_32bpp(int line, uint8_t *linebuf)
{
    int i;
    uint32_t *fb_pixels = &s_framebuffer.pixels32[LCD_WIDTH * line];

    for (i = 0; i < LCD_WIDTH; i++)
        *(fb_pixels++) = s_palette.pal32[*(linebuf++)];
}

void v_gdi_write_line_16bpp(int line, uint8_t *linebuf)
{
    int i;
    uint16_t *fb_pixels = &s_framebuffer.pixels16[LCD_WIDTH * line];

    for (i = 0; i < LCD_WIDTH; i++)
        *(fb_pixels++) = s_palette.pal16[*(linebuf++)];
}

void v_gdi_frame_done()
{
    //  Invalidate region to schedule repaint
    InvalidateRgn(hwnd, 0, 0);
    UpdateWindow(hwnd);

    frame_count++;

    if (frame_count == FRAME_COUNT_INTERVAL) {
        doFPScount();
        frame_count = 0;
    }

}

video_backend_status v_gdi_event_handler()
{
    // Process all messages queued since the last frame

    MSG msg;

    while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    Sleep(0);
    return s_status;
}

#else
const video_backend_t v_gdi = {NAME, 0};
#endif
