#include "backends.h"

#define NAME "a_dummy"

/*
 *  Dummy Audio Backend
 */

#include "compat.h"

int a_dummy_init(audio_config *cfg) {
    return 1;
}

void a_dummy_deinit() {}

audio_buffer_status a_dummy_play_buffer(uint8_t *buffer, uint32_t length) {
    return AUDIO_BUFFER_COPIED;
}

const audio_backend_t a_dummy = {
    NAME,                   // name
    1,                      // present
    a_dummy_init,           // init
    a_dummy_deinit,         // deinit
    a_dummy_play_buffer,  // play_buffer
};
