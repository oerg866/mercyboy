#include <stdio.h>
#include <stdlib.h>
#include "sys.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include "audio.h"
#include "trace.h"

#include "string.h"

#ifdef __WIN32__
#include <windows.h>
#endif


int main(int argc, char* argv[])
//int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{

    trace_init(0, 0, NULL);

#ifdef DEBUG
    FILE *dbg = NULL;
    /*  Uncomment for trace file
     *  dbg = fopen("dbg.txt", "rw");
     */
    trace_init(0, 1, dbg);
#endif


    printf("MercyBoy TEST VERSION\n");

    FILE *infile = fopen(argv[1], "rb");

    int fsize;
    fseek(infile, 0, SEEK_END);
    fsize = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    uint8_t *romfile = malloc(fsize);

    printf ("File read: %lu\n", fread(romfile, 1, fsize, infile));

    fclose(infile);

    int width = 160;
    int height = 144;

    printf ("File size: %u bytes.\n", fsize);
    cpu_init();
    sys_init();
    int result = mem_init(romfile, fsize);

    audio_init();
    video_backend_init(width, height);
    video_init();

    // Run CPU

    run();


    free(romfile);

    SDL_Quit();


    return result;
}
