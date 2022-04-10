#include "backends.h"

#define NAME "a_waveout"

#ifdef AUDIO_WAVEOUT

/*
 *  Audio Backend Implementation for Win16/Win32 WAVEOUT API
 */

#include <mmsystem.h>

#ifdef __WIN16__
typedef WAVEFORMAT WAVEFORMAT_T;
#else
typedef WAVEFORMATEX WAVEFORMAT_T;
#endif

//#include "trace.h"
#include "audio.h"
#include "ringbuf.h"
#include "trace.h"

static HWAVEOUT s_waveout_handle = NULL;
static WAVEHDR *s_wavehdrs = NULL;
static ringbuffer *s_ringbuf = NULL;;

#define BUFFER_COUNT 4
#define BUFFER_LENGTH_MINIMUM 1536


void a_waveout_cfg_to_waveformat(audio_config *cfg, WAVEFORMAT_T *waveformat) {
    waveformat->wFormatTag = WAVE_FORMAT_PCM;
    waveformat->nChannels = cfg->channels;
    waveformat->nSamplesPerSec = cfg->sample_rate;
    waveformat->nAvgBytesPerSec = cfg->sample_rate * (cfg->bits_per_sample / 8) * cfg->channels;
    waveformat->nBlockAlign = (cfg->bits_per_sample / 8) *cfg->channels;
#ifndef __WIN16__
    waveformat->wBitsPerSample = cfg->bits_per_sample;
    waveformat->cbSize = 0;
#endif
}

void CALLBACK a_waveout_callback (HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    WAVEHDR *header;
    unsigned bufferNum;

    if (uMsg == WOM_DONE) {
        header = (WAVEHDR*) dwParam1;
        bufferNum = header->dwUser;
        // This call is ommited because the header never change, seems to work fine:
        // waveOutUnprepareHeader(hwo, (WAVEHDR*) header, sizeof(WAVEHDR));
        ringbuffer_unblock(s_ringbuf, bufferNum);
        waveOutWrite(hwo, &s_wavehdrs[ringbuffer_increment_fetch_and_block(s_ringbuf)], sizeof(WAVEHDR));
    }
}

int a_waveout_init(audio_config *cfg) {
    WAVEFORMAT_T waveformat;
    MMRESULT ret;
    uint32_t bytes_per_sample = cfg->bits_per_sample / 8;
    uint32_t ringbuffer_slice_size;
    uint32_t i;

    if (cfg->buffer_size < BUFFER_LENGTH_MINIMUM) {
        cfg->buffer_size = BUFFER_LENGTH_MINIMUM;
        print_msg("WARNING: Buffer size extremely low, increasing\n");
    }

    ringbuffer_slice_size = cfg->buffer_size * cfg->channels * bytes_per_sample;

    // Build WAVEFORMAT(EX) structure out of the audio config.
    a_waveout_cfg_to_waveformat(cfg, &waveformat);

    // Allocate memory for ringbuffer.
    s_ringbuf = ringbuffer_create(ringbuffer_slice_size, BUFFER_COUNT);
    s_wavehdrs = alloc_mem(BUFFER_COUNT * sizeof(WAVEHDR));
    memset(s_wavehdrs, 0, BUFFER_COUNT * sizeof(WAVEHDR));

    // Open the device
    ret = waveOutOpen(&s_waveout_handle, WAVE_MAPPER, &waveformat, (DWORD) a_waveout_callback, 0, CALLBACK_FUNCTION);
    print_msg("MMRESULT = %d\n", ret);

    // Initialize and prepare headers
    for (i = 0; i < BUFFER_COUNT; ++i) {
        s_wavehdrs[i].dwBufferLength = ringbuffer_slice_size;
        s_wavehdrs[i].lpData = (char*) ringbuffer_get_data(s_ringbuf, i);
        s_wavehdrs[i].dwUser = i;
        waveOutPrepareHeader(s_waveout_handle, &s_wavehdrs[i], sizeof(WAVEHDR));
    }

    // Give it a little head start because WAVEOUT is slow as hell...
    for (i = 0; i < BUFFER_COUNT/2; ++i) {
        ringbuffer_increment_fetch_and_block(s_ringbuf);
        ret = waveOutWrite(s_waveout_handle, &s_wavehdrs[i], sizeof(WAVEHDR));
        print_msg("MMRESULT = %d\n", ret);
    }

    print_msg("WaveOut init done\n");
    return 0;
}

void a_waveout_deinit(void) {
    waveOutClose(s_waveout_handle);
    free(s_wavehdrs);
    ringbuffer_destroy(s_ringbuf);
}

audio_buffer_status a_waveout_play_buffer(uint8_t *buffer, uint32_t length) {
    ringbuffer_insert_bytes(s_ringbuf, buffer, length);
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
