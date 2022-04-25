#include <stdio.h>

#include "compat.h"

#include "sys.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include "audio.h"
#include "console.h"

#include "string.h"

#include "backends.h"

/*
 * Simple main function implementation.
 * For pc-ish systems that load files from a file system.
 * If it is used, it is called from the actual platform-specific main function.
 */

static audio_backend_t *s_audio_backend = NULL;
static video_backend_t *s_video_backend = NULL;
static input_backend_t *s_input_backend = NULL;

static audio_config s_audio_config = DEFAULT_AUDIO_CONFIG;
static video_config s_video_config = DEFAULT_VIDEO_CONFIG;

int main_default (int argc, char *argv[])
{
    FILE *infile;
    unsigned long fsize;
    uint8_t *romfile;
    int result;

#ifdef DEBUG
//    FILE *dbg = NULL;
//    dbg = fopen("dbg.txt", "rw");
    trace_init(0, 0, NULL);
#endif

    if (argc < 2) {
        print_msg("Usage: %s rom-image.gb\n", argv[0]);
        return 1;
    }

    infile = fopen(argv[1], "rb");

    fseek(infile, 0, SEEK_END);
    fsize = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    romfile = alloc_mem(fsize);

    print_msg("File read: %lu\n", (unsigned long) fread(romfile, 1, fsize, infile));

    fclose(infile);

    print_msg("File size: %lu bytes.\n", (unsigned long) fsize);
    cpu_init();

    // Initialize backends

    s_audio_backend = get_audio_backend(NULL);
    s_video_backend = get_video_backend(NULL);
    s_input_backend = get_input_backend(NULL);

    sys_init(s_input_backend);


    result = mem_init(romfile, fsize);

    audio_init(s_audio_backend, &s_audio_config);

    if (s_audio_config.audio_timing_override)
        s_video_config.use_audio_timing = 0;

    video_init(s_video_backend, &s_video_config);

    // Run CPU

    sys_run();


    free(romfile);

    sys_deinit();
    audio_deinit();
    video_deinit();
    mem_deinit();

    return result;
}
