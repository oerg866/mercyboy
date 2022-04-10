#ifndef RINGBUFFER_H
#define RINGBUFFER_H

/*
 *  Simple ringbuffer implementation (intended for audio code)
 */

#include "compat.h"

typedef struct {
    uint8_t *buf_start;
    uint32_t bufs;
    uint32_t buf_size;
    uint32_t total_size;
    uint8_t *is_blocked;
    uint32_t r_offset;
    uint32_t w_offset;
    uint32_t r_buf;
    uint32_t w_buf;
} ringbuffer;

ringbuffer *ringbuffer_create(uint32_t buf_size, uint32_t num_buf);
void ringbuffer_clear(ringbuffer *rb);
void ringbuffer_destroy(ringbuffer *rb);
void ringbuffer_print_info(ringbuffer *rb, char *tag);
uint32_t ringbuffer_increment_fetch_and_block(ringbuffer *rb);
void ringbuffer_unblock(ringbuffer *rb, uint32_t buffer_id);
void ringbuffer_unblock_current(ringbuffer *rb);
void ringbuffer_insert_bytes(ringbuffer *rb, uint8_t* src, uint32_t len);
uint8_t *ringbuffer_get_data(ringbuffer *rb, uint32_t buffer_id);
#endif
