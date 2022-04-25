#include "backends.h"

#define NAME "a_sb16dj"

#if (1)

/*
 *  Audio Backend Implementation for SoundBlaster (16) cards
 *  on DJGPP
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pc.h>

#include "audio.h"
#include "console.h"
#include "compat.h"

#define SB_IOBASE 0x220

#define PORT_MIXER_CMD          0x04
#define PORT_MIXER_DATA         0x05
#define PORT_DSP_RESET          0x06
#define PORT_DSP_READ           0x0A
#define PORT_DSP_WRITE          0x0C
#define PORT_DSP_READ_STATUS    0x0E
#define PORT_DSP_INT16_ACK      0x0F

typedef struct {
    uint16_t io;
    uint8_t irq;
    uint8_t dma_l;
    uint8_t dma_h;
    uint8_t type;
} sb_config;

static sb_config a_sb16dj_device = { 0x220, 0x05, 0x00, 0x01, 0x01 }; // Default settings

static inline void _writedsp(uint8_t value) {
    outportb(a_sb16dj_device.io + PORT_DSP_WRITE, value);
}

static inline uint8_t _readdsp() {
    return inportb(a_sb16dj_device.io + PORT_DSP_READ);
}

static bool _resetdsp(uint16_t base) {
    outportb(base + PORT_DSP_RESET, 1);
    sleep_ms(10);
    outportb(base + PORT_DSP_RESET, 0);
    sleep_ms(10);

    if ((inportb(base + PORT_DSP_READ_STATUS) & 0x80) && (inportb(base + PORT_DSP_READ) == 0xAA)) {
        return true;
    }

    return false;
}

static char *get_blaster_token(char *str, char *type, unsigned *value) {
    char *output;
    char c; // Temporary character to parse token type

    // Find first character
    while (*str && (*str <= ' ')) { // Skip whitespace
        str++;
    }

    if (!*str)   // String ended before token type!
        return NULL;

    c = toupper(*str);

    // If token type is not supported, error out!
    if (c != 'A' && c != 'I' && c != 'D' && c != 'H' && c != 'T')
        return NULL;


    *type = c;

    // We found the type, now parse the number.
    *value = strtoul(str+1, &output, 16);
    return output;
}

int a_sb16dj_init(audio_config *cfg) {
    char *settings = getenv("BLASTER");
    char token_type;
    unsigned token_value;
    unsigned dsp_version_h;
    unsigned dsp_version_l;
    bool ret;

    while (settings) {
        settings = get_blaster_token(settings, &token_type, &token_value);
        if (!settings) break; // Error or string ended

        switch (token_type) {
        case 'A': a_sb16dj_device.io = token_value; break;
        case 'I': a_sb16dj_device.irq = token_value; break;
        case 'D': a_sb16dj_device.dma_l = token_value; break;
        case 'H': a_sb16dj_device.dma_h = token_value; break;
        case 'T': a_sb16dj_device.type = token_value; break;
        default: break;
        }
    }

    printf("SoundBlaster settings: \n");
    printf("    Address = %03x\n", a_sb16dj_device.io);
    printf("        IRQ = %3x\n", a_sb16dj_device.irq);
    printf("        DMA = %3x\n", a_sb16dj_device.dma_l);
    printf("   High DMA = %3x\n", a_sb16dj_device.dma_h);
    printf("       Type = %3x\n", a_sb16dj_device.type);

    ret = _resetdsp(a_sb16dj_device.io);

    if (!ret) {
        printf("DSP reset failed!\n");
        return 1;
    }

    _writedsp(0xe1); // Get DSP version
    dsp_version_h = _readdsp();
    dsp_version_l = _readdsp();
    printf("DSP version: %u.%02x\n", dsp_version_h, dsp_version_l);

    system("pause");

    return 0;
}

void a_sb16dj_deinit() {
}

audio_buffer_status a_sb16dj_play_buffer(uint8_t *buffer, uint32_t length) {

    return AUDIO_BUFFER_COPIED;
}

const audio_backend_t a_sb16dj = {
    NAME,               // name
    1,                  // present
    a_sb16dj_init,        // init
    a_sb16dj_deinit,      // deinit
    a_sb16dj_play_buffer, // play_buffer
};
#else
const audio_backend_t a_sb16dj = { NAME, 0 };
#endif
