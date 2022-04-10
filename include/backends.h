#ifndef BACKENDS_H
#define BACKENDS_H

#include "compat.h"

/*
 *  Video backend header
 */


typedef struct {
    uint16_t screen_width;
    uint16_t screen_height;
    uint16_t bpp;
    uint16_t use_audio_timing;
} video_config;

typedef struct {
    uint8_t color[4];
} gameboy_palette;

typedef enum {
    VIDEO_BACKEND_RUNNING,
    VIDEO_BACKEND_EXIT,
} video_backend_status;

typedef int                     (*fv_init)              (video_config *cfg);
typedef void                    (*fv_deinit)            (void);
typedef void                    (*fv_update_palette)    (uint8_t pal_offset, gameboy_palette palette);
typedef void                    (*fv_write_line)        (int line, uint8_t *linebuf);
typedef void                    (*fv_frame_done)        (void);
typedef video_backend_status    (*fv_event_handler)     (void);

typedef struct {
    const char                  *name;
    const uint8_t               present;
    fv_init                     init;
    fv_deinit                   deinit;
    fv_update_palette           update_palette;
    fv_write_line               write_line;
    fv_frame_done               frame_done;
    fv_event_handler            event_handler;
} video_backend_t;

typedef struct {
    uint16_t sample_rate;
    uint16_t bits_per_sample;
    uint16_t channels;
    uint16_t buffer_size;
} audio_config;

typedef enum {
    AUDIO_ERROR,            // Buffer was not queued successfully. Will be free'd by emulator.
    AUDIO_BUFFER_COPIED,    // Buffer was copied and is no longer needed. Will be free'd by emulator.
    AUDIO_BUFFER_TAKEN      // Buffer ownership was transferred to the backend. Will **NOT** be free'd by emulator. Backend MUST free it.
} audio_buffer_status;

typedef int                     (*fa_init)              (audio_config *cfg);
typedef void                    (*fa_deinit)            (void);
typedef audio_buffer_status     (*fa_play_buffer)       (uint8_t *buffer, uint32_t length);

typedef struct {
    const char                  *name;
    const uint8_t               present;
    fa_init                     init;
    fa_deinit                   deinit;
    fa_play_buffer              play_buffer;
} audio_backend_t;

#define PAD_A       0
#define PAD_B       1
#define PAD_SEL     2
#define PAD_START   3
#define PAD_RIGHT   4
#define PAD_LEFT    5
#define PAD_UP      6
#define PAD_DOWN    7

typedef void                    (*fi_init)              (void);
typedef void                    (*fi_deinit)            (void);
typedef uint8_t                 (*fi_get_buttons)       (void);

typedef struct {
    const char                  *name;
    const uint8_t               present;
    fi_init                     init;
    fi_deinit                   deinit;
    fi_get_buttons              get_buttons;
} input_backend_t;

#define DEFAULT_VIDEO_CONFIG { LCD_WIDTH * 3, LCD_HEIGHT * 3, 32, 1 }
#define DEFAULT_AUDIO_CONFIG { 44100, 16, 2, 512 }

extern audio_backend_t *get_audio_backend(const char *name);
extern video_backend_t *get_video_backend(const char *name);
extern input_backend_t *get_input_backend(const char *name);

#endif
