#ifdef INPUT_SDL2

#include "input.h"
#include <SDL2/SDL.h>

uint8_t sys_buttons_all = 0;
uint8_t sys_buttons_old = 0;

// Key presets

int32_t keys[8] = {
    SDL_SCANCODE_A,       // A key, for some reason letters don't have a define for it
    SDL_SCANCODE_S,       // S
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_DOWN
};

void backend_handle_joypad() {

    const uint8_t *state = SDL_GetKeyboardState(NULL);

    // 0xFF00
    /*
    Bit 7 - Not used
    Bit 6 - Not used
    Bit 5 - P15 Select Button Keys      (0=Select)
    Bit 4 - P14 Select Direction Keys   (0=Select)
    Bit 3 - P13 Input Down  or Start    (0=Pressed) (Read Only)
    Bit 2 - P12 Input Up    or Select   (0=Pressed) (Read Only)
    Bit 1 - P11 Input Left  or Button B (0=Pressed) (Read Only)
    Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
    */

   sys_buttons_old = sys_buttons_all; // old joypad state for later

    sys_buttons_all = 0
            | ((~state[SDL_SCANCODE_DOWN] & 0x01)    << 7)
            | ((~state[SDL_SCANCODE_UP] & 0x01)      << 6)
            | ((~state[SDL_SCANCODE_LEFT] & 0x01)    << 5)
            | ((~state[SDL_SCANCODE_RIGHT] & 0x01)   << 4)
            | ((~state[SDL_SCANCODE_RETURN] & 0x01)  << 3)
            | ((~state[SDL_SCANCODE_SPACE] & 0x01)   << 2)
            | ((~state[SDL_SCANCODE_S] & 0x01)       << 1)
            | ((~state[SDL_SCANCODE_A] & 0x01)       << 0)
            ;

}

#endif
