#include "backends.h"

#define NAME "i_sdl2"

#ifdef INPUT_SDL2

#include <SDL2/SDL.h>

// Key presets

int32_t keys[8] = {
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_DOWN
};

uint8_t i_sdl2_get_buttons() {
    const uint8_t *state = SDL_GetKeyboardState(NULL);

    return  0
            | ((~state[keys[PAD_DOWN]] & 0x01)  << 7)
            | ((~state[keys[PAD_UP]] & 0x01)    << 6)
            | ((~state[keys[PAD_LEFT]] & 0x01)  << 5)
            | ((~state[keys[PAD_RIGHT]] & 0x01) << 4)
            | ((~state[keys[PAD_START]] & 0x01) << 3)
            | ((~state[keys[PAD_SEL]] & 0x01)   << 2)
            | ((~state[keys[PAD_B]] & 0x01)     << 1)
            | ((~state[keys[PAD_A]] & 0x01)     << 0);
}

void i_sdl2_init() {}
void i_sdl2_deinit() {}

const input_backend_t i_sdl2 = {
    NAME,                   // name
    1,                      // present
    i_sdl2_init,            // init
    i_sdl2_deinit,          // deinit
    &i_sdl2_get_buttons     // get_buttons
};
#else
const input_backend_t i_sdl2 = { NAME, 0 };
#endif
