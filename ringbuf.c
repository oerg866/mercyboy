#include "ringbuf.h"

#include "console.h"

#include <string.h>

ringbuffer *ringbuffer_create(uint32_t buf_size, uint32_t num_buf) {
    ringbuffer *ret = NULL;

    if (!buf_size || (num_buf < 3)) {
        print_msg("Ringbuf parameter error\n");
        return NULL;
    }

    ret = alloc_mem(sizeof(ringbuffer));

    ret->bufs = num_buf;
    ret->buf_size = buf_size;
    ret->total_size = buf_size * num_buf;
    ret->buf_start = alloc_mem(ret->total_size);
    ret->is_blocked = alloc_mem(num_buf);
    ret->r_offset = 0;
    ret->w_offset = 0;
    ret->r_buf = 0;
    ret->w_buf = 0;

    ringbuffer_clear(ret);

    print_msg("ringbuffer total alloc: %lu\n", (unsigned long) ret->total_size);

    return ret;
}


void ringbuffer_clear(ringbuffer *rb) {
    memset(rb->buf_start, 0, rb->total_size);
    memset(rb->is_blocked, 0, rb->bufs);
}

void ringbuffer_destroy(ringbuffer *rb) {
    free(rb->buf_start);
    free(rb);
}

void ringbuffer_print_info(ringbuffer *rb, char *tag) {
    print_msg("ringbuffer %p %s bufs %lu, buf_size %lu, total_size %lu, r_offset %lu, r_buf %lu, w_offset %lu, w_buf %lu\n",
    rb->buf_start,
    tag,
    (unsigned long) rb->bufs,
    (unsigned long) rb->buf_size,
    (unsigned long) rb->total_size,
    (unsigned long) rb->r_offset,
    (unsigned long) rb->r_buf,
    (unsigned long) rb->w_offset,
    (unsigned long) rb->w_buf);
}

uint32_t ringbuffer_increment_fetch_and_block(ringbuffer *rb)
// Fetches the next read buffer slice's ID and marks this buffer slice as blocked.
{
    rb->r_buf = (rb->r_buf + 1) % rb->bufs;
    rb->is_blocked[rb->r_buf] = 1;
    return rb->r_buf;
}

void ringbuffer_unblock(ringbuffer *rb, uint32_t buffer_id) {
    rb->is_blocked[buffer_id] = 0;
}

void ringbuffer_unblock_current(ringbuffer *rb) {
    rb->is_blocked[rb->r_buf] = 0;
}

void ringbuffer_insert_bytes(ringbuffer *rb, uint8_t* src, uint32_t len) {
    uint32_t bytes_to_write = 0;
    volatile uint8_t is_blocked = 1;

    // Primitive locking mechanism for writes to unreturned buffers.
    // Win16 and early Win32 doesn't seem to have any concept of concurrency so
    // this is the best I could come up with

#if !defined(BENCHMARK)
    while (is_blocked) {
        is_blocked = rb->is_blocked[rb->w_buf];
        yield();
    }
#endif

    while (len) {

        bytes_to_write = MIN(len, rb->buf_size - rb->w_offset);

        memcpy (&rb->buf_start[rb->w_buf * rb->buf_size + rb->w_offset], src, bytes_to_write);
        src += bytes_to_write;
        rb->w_offset += bytes_to_write;

        if (rb->w_offset >= rb->buf_size) {
            rb->w_buf = (rb->w_buf + 1) % rb->bufs;
            rb->w_offset %= rb->buf_size;


            // Primitive locking mechanism for writes to unreturned buffers.
            is_blocked = 1;

#if !defined(BENCHMARK)
            while (is_blocked) {
                is_blocked = rb->is_blocked[rb->w_buf];
                yield();
            }
#endif
        }

        len -= bytes_to_write;
    }
}

uint8_t *ringbuffer_get_data(ringbuffer *rb, uint32_t buffer_id) {
    return &rb->buf_start[rb->buf_size * buffer_id];
}
