#include "backends.h"

#define NAME "i_win32"

#ifdef INPUT_WIN32

#include <windows.h>
#include <winuser.h>

#include "compat.h"

// Key presets

static int32_t keys[8] = {
    0x41,       // A key, for some reason letters don't have a define for it
    0x53,       // S
    VK_SPACE,
    VK_RETURN,
    VK_RIGHT,
    VK_LEFT,
    VK_UP,
    VK_DOWN
};

uint8_t i_win32_get_buttons() {
    // Win32 GetAsyncKeyState: MSB is indicator if key is down
    return 0
            | ((~GetAsyncKeyState(keys[PAD_DOWN]) & (1 << 15))  >> (15 - 7))
            | ((~GetAsyncKeyState(keys[PAD_UP]) & (1 << 15))    >> (15 - 6))
            | ((~GetAsyncKeyState(keys[PAD_LEFT]) & (1 << 15))  >> (15 - 5))
            | ((~GetAsyncKeyState(keys[PAD_RIGHT]) & (1 << 15)) >> (15 - 4))
            | ((~GetAsyncKeyState(keys[PAD_START]) & (1 << 15)) >> (15 - 3))
            | ((~GetAsyncKeyState(keys[PAD_SEL]) & (1 << 15))   >> (15 - 2))
            | ((~GetAsyncKeyState(keys[PAD_B]) & (1 << 15))     >> (15 - 1))
            | ((~GetAsyncKeyState(keys[PAD_A]) & (1 << 15))     >> (15 - 0));
}

const input_backend_t i_win32 = {
    NAME,                   // name
    1,                      // present
    NULL,                   // init
    NULL,                   // deinit
    i_win32_get_buttons    // get_buttons
};
#else
const input_backend_t i_win32 = { NAME, 0 };
#endif
