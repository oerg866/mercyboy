#include "backends.h"

#define NAME "a_waveout"

#ifdef AUDIO_WAVEOUT

/*
 *  Audio Backend Implementation for Win32 WAVEOUT API
 */

#include <mmsystem.h>

#ifndef _WAVEFORMATEX_
typedef PCMWAVEFORMAT WAVEFORMAT_T;
#else
typedef WAVEFORMATEX WAVEFORMAT_T;
#endif

#include "audio.h"
#include "ringbuf.h"
#include "console.h"


#define BUFFER_COUNT 4
#define BUFFER_LENGTH_MINIMUM 1536


static HWAVEOUT s_waveout_handle = NULL;
static WAVEHDR *s_wavehdrs = NULL;

static uint8_t *s_buffer_pool = NULL;

static int s_write_index = 0;
static int s_write_pos = 0;

static uint32_t s_buffer_size;

static CRITICAL_SECTION s_lock;

void a_waveout_cfg_to_waveformat(audio_config *cfg, WAVEFORMAT_T *out) {
#ifdef _WAVEFORMATEX_
    WAVEFORMAT_T *waveformat = out;
    waveformat->wBitsPerSample = cfg->bits_per_sample;
    waveformat->cbSize = 0;
#else
    // Win16 & Early Win32, use PCMWAVEFORMAT
    WAVEFORMAT *waveformat = &out->wf;
    out->wBitsPerSample = cfg->bits_per_sample;
#endif
    waveformat->wFormatTag = WAVE_FORMAT_PCM;
    waveformat->nChannels = cfg->channels;
    waveformat->nSamplesPerSec = cfg->sample_rate;
    waveformat->nAvgBytesPerSec = cfg->sample_rate * (cfg->bits_per_sample / 8) * cfg->channels;
    waveformat->nBlockAlign = (cfg->bits_per_sample / 8) *cfg->channels;
}

void CALLBACK a_waveout_callback (HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    WAVEHDR *header;

    if (uMsg == WOM_DONE) {

        EnterCriticalSection(&s_lock);
        header = (WAVEHDR*) dwParam1;
        header->dwFlags = WHDR_PREPARED;
        LeaveCriticalSection(&s_lock);

    }
}

int a_waveout_init(audio_config *cfg) {
    WAVEFORMAT_T waveformat;
    MMRESULT ret;
    uint32_t bytes_per_sample = cfg->bits_per_sample / 8;
    uint32_t i;

    InitializeCriticalSection(&s_lock);

    memset(&waveformat, 0, sizeof(WAVEFORMAT_T));

    // We don't support this. Timing is dictated by the ringbuffer locking.
    cfg->audio_timing_override = 1;

    if (cfg->buffer_size < BUFFER_LENGTH_MINIMUM) {
        cfg->buffer_size = BUFFER_LENGTH_MINIMUM;
        print_msg("WARNING: Buffer size extremely low, increasing\n");
    }

    // Build WAVEFORMAT(EX) structure out of the audio config.
    a_waveout_cfg_to_waveformat(cfg, &waveformat);

    s_buffer_size = cfg->buffer_size;

    s_buffer_pool = alloc_mem(s_buffer_size * BUFFER_COUNT);

    // Allocate memory for ringbuffer.

    s_wavehdrs = alloc_mem(BUFFER_COUNT * sizeof(WAVEHDR));
    memset(s_wavehdrs, 0, BUFFER_COUNT * sizeof(WAVEHDR));

    // Open the device
    ret = waveOutOpen(&s_waveout_handle, WAVE_MAPPER, &waveformat, (DWORD_PTR) a_waveout_callback, 0, CALLBACK_FUNCTION);
    print_msg("MMRESULT = %d\n", ret);

    s_write_index = 0;

    // Initialize and prepare headers
    // Give it a little head start because WAVEOUT is slow as hell...

    for (i = 0; i < BUFFER_COUNT; ++i) {
        s_wavehdrs[i].dwBufferLength = s_buffer_size;
        s_wavehdrs[i].lpData = (char*) &s_buffer_pool[s_buffer_size * i];
        s_wavehdrs[i].dwUser = i;
        waveOutPrepareHeader(s_waveout_handle, &s_wavehdrs[i], sizeof(WAVEHDR));
    }

    for (i = 0; i < BUFFER_COUNT; ++i) {

        ret = waveOutWrite(s_waveout_handle, &s_wavehdrs[i], sizeof(WAVEHDR));
        print_msg("MMRESULT = %d\n", ret);

    }

    print_msg("WaveOut init done\n");
    return 0;
}

void a_waveout_deinit(void) {
    unsigned i;

    waveOutClose(s_waveout_handle);
    for (i = 0; i < BUFFER_COUNT; ++i) {
        waveOutUnprepareHeader(s_waveout_handle, &s_wavehdrs[i], sizeof(WAVEHDR));
    }
    free(s_wavehdrs);
    free(s_buffer_pool);
    DeleteCriticalSection(&s_lock);
}

audio_buffer_status a_waveout_play_buffer(uint8_t *buffer, uint32_t length) {
    WAVEHDR *header = NULL;
    uint8_t *dst = NULL;
    uint8_t *src = buffer;
    uint32_t free_space;
    DWORD tmp = 0;

    while (length) {

        header = &s_wavehdrs[s_write_index];

        // Wait until next buffer is free
#if !defined(BENCHMARK)
        do {
            EnterCriticalSection(&s_lock);
            tmp = header->dwFlags;
            LeaveCriticalSection(&s_lock);
            yield();
        } while (tmp != WHDR_PREPARED);
#endif

        // Get bytes left in this buffer.

        dst = &s_buffer_pool[s_write_index * s_buffer_size + s_write_pos];
        free_space = s_buffer_size-s_write_pos;

        if (length < free_space) {
            memcpy (dst, src, length);
            s_write_pos += length;
            break;
        } else {
            memcpy (dst, src, free_space);
            waveOutWrite(s_waveout_handle, header, sizeof(WAVEHDR));
            length -= free_space;
            src += free_space;
            s_write_pos = 0;
            s_write_index = (s_write_index + 1) % BUFFER_COUNT;
            if (length == 0) break;
        }

    }
    return AUDIO_BUFFER_COPIED;
}

const audio_backend_t a_waveout = {
    NAME,                   // name
    1,                      // present
    a_waveout_init,         // init
    a_waveout_deinit,       // deinit
    a_waveout_play_buffer   // play_buffer
};
#else
const audio_backend_t a_waveout = { NAME, 0 };
#endif
