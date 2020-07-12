#include <stdio.h>
#include <stdlib.h>
#include "sys.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include "string.h"

#ifdef __WIN32__
#include <windows.h>
#endif

#include <SDL2/SDL.h>


int main(int argc, char* argv[])
//int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
/*    char**argv = lpCmdLine;
    int argc = nCmdShow;*/

    printf("MercyBoy TEST VERSION\n");

    FILE *infile = fopen(argv[1], "rb");

    int fsize;
    fseek(infile, 0, SEEK_END);
    fsize = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    uint8_t *romfile = malloc(fsize);

    printf ("File read: %lu\n", fread(romfile, 1, fsize, infile));

    fclose(infile);


    printf ("File size: %u bytes.\n", fsize);
    cpu_init();
    sys_init();
    int result = mem_init(romfile, fsize);


    // Init SDL

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)  {
        printf("Error intiializing SDL\n");
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("MercyBoy",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          160,
                                          144,
                                          0);

    if (!window) {
        printf("Error opening window\n");
        return -1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    printf("Window size = %i * %i\n", window_surface->h, window_surface->pitch);



    if (!window_surface) {
        printf("Failed to get surface from the window\n");
        return -1;
    }

    video_init(window_surface, window);


    SDL_UpdateWindowSurface(window);

    //SDL_Delay(5000);
    // Run CPU

    run();


    free(romfile);

    SDL_Quit();


    return result;
}
