#pragma once

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

extern uint8_t sys_buttons_all;
extern uint8_t sys_buttons_old;

#define PAD_A       0
#define PAD_B       1
#define PAD_SEL     2
#define PAD_START   3
#define PAD_RIGHT   4
#define PAD_LEFT    5
#define PAD_UP      6
#define PAD_DOWN    7

void backend_handle_joypad();

#endif // INPUT_H
