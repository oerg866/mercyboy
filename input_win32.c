#ifdef INPUT_WIN32

#include <windows.h>
#include <winuser.h>
#include "input.h"

#include <stdint.h>

uint8_t sys_buttons_all = 0;
uint8_t sys_buttons_old = 0;

// Key presets

int32_t keys[8] = {
    0x41,       // A key, for some reason letters don't have a define for it
    0x53,       // S
    VK_SPACE,
    VK_RETURN,
    VK_RIGHT,
    VK_LEFT,
    VK_UP,
    VK_DOWN
};

void backend_handle_joypad() {

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

    // Win32 GetAsyncKeyState: MSB is indicator if key is down

    sys_buttons_all = 0
                    | ((~GetAsyncKeyState(keys[PAD_DOWN]) & (1 << 15))  >> (15 - 7))
                    | ((~GetAsyncKeyState(keys[PAD_UP]) & (1 << 15))    >> (15 - 6))
                    | ((~GetAsyncKeyState(keys[PAD_LEFT]) & (1 << 15))  >> (15 - 5))
                    | ((~GetAsyncKeyState(keys[PAD_RIGHT]) & (1 << 15)) >> (15 - 4))
                    | ((~GetAsyncKeyState(keys[PAD_START]) & (1 << 15)) >> (15 - 3))
                    | ((~GetAsyncKeyState(keys[PAD_SEL]) & (1 << 15))   >> (15 - 2))
                    | ((~GetAsyncKeyState(keys[PAD_B]) & (1 << 15))     >> (15 - 1))
                    | ((~GetAsyncKeyState(keys[PAD_A]) & (1 << 15))     >> (15 - 0));

}


#endif
