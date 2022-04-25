#include "backends.h"

#define NAME "i_dos32"

#ifdef INPUT_DOS32

#include <dpmi.h>
#include <pc.h>

#include "compat.h"
#include "console.h"

int32_t keys[8] = {
    0x1E,   // A
    0x1F,   // S
    0x39,   // SPACE
    0x1C,   // ENTER
    0x4D,   // RIGHT ARROW
    0x4B,   // LEFT ARROW
    0x48,   // UP ARROW
    0x50,   // DOWN ARROW
};

static volatile uint8_t pressed_keys[256];
static _go32_dpmi_seginfo old_kb_ivec;
static _go32_dpmi_seginfo new_kb_ivec;

void i_dos32_kb_interrupt() {
    uint8_t key = inportb(0x60);
    static bool extended = false;

    if (key >= 0xE0) {
        extended = true;
    } else {
        print_msg("EXTENDED: %u, KEY: %02x\n", (unsigned) extended, (unsigned) key);
        if (extended) {
            pressed_keys[key | 0x80] = (~key) >> 7;
            extended = false;
        } else {
            pressed_keys[key & 0x7F] = (~key) >> 7;
        }
    }

    outportb(0x20, 0x20);   // Send End of interrupt command to the PIC
}

void i_dos32_kb_interrupt_end() {}

void i_dos32_init() {
    // Lock memory location of the interrupt handler and keypress table
    _go32_dpmi_lock_data( (char *)&pressed_keys, sizeof(pressed_keys) );
    _go32_dpmi_lock_code(i_dos32_kb_interrupt, (unsigned long) i_dos32_kb_interrupt_end - (unsigned long) i_dos32_kb_interrupt);

    _go32_dpmi_get_protected_mode_interrupt_vector(0x08 + 1, &old_kb_ivec); // Store old keyboard interrupt vector

    // Register the new keyboard interrupt vector
    new_kb_ivec.pm_offset = (unsigned long) i_dos32_kb_interrupt;

    if (_go32_dpmi_allocate_iret_wrapper(&new_kb_ivec) != 0)
        print_msg("ERROR allocating keyboard iret wrapper!");
    if (_go32_dpmi_set_protected_mode_interrupt_vector(0x08 + 1, &new_kb_ivec) != 0) {
        print_msg("ERROR setting keyboard interrupt vector!");
    }
}

void i_dos32_deinit() {
    // We must restore the old interrupt vector, this is crucial!
    _go32_dpmi_set_protected_mode_interrupt_vector(0x08 + 1, &old_kb_ivec);
    _go32_dpmi_free_iret_wrapper(&new_kb_ivec);
    printf("%s successful!\n", __func__);
}

uint8_t  i_dos32_get_buttons() {
    return  0
            | ((~pressed_keys[keys[PAD_DOWN]] & 0x01)  << 7)
            | ((~pressed_keys[keys[PAD_UP]] & 0x01)    << 6)
            | ((~pressed_keys[keys[PAD_LEFT]] & 0x01)  << 5)
            | ((~pressed_keys[keys[PAD_RIGHT]] & 0x01) << 4)
            | ((~pressed_keys[keys[PAD_START]] & 0x01) << 3)
            | ((~pressed_keys[keys[PAD_SEL]] & 0x01)   << 2)
            | ((~pressed_keys[keys[PAD_B]] & 0x01)     << 1)
            | ((~pressed_keys[keys[PAD_A]] & 0x01)     << 0);
}

const input_backend_t i_dos32 = {
    NAME,                   // name
    1,                      // present
    i_dos32_init,           // init
    i_dos32_deinit,         // deinit
    i_dos32_get_buttons     // get_buttons
};
#else
const input_backend_t i_dos32 = { NAME, 0 };
#endif
