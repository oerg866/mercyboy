#include "backends.h"

#define NAME "a_sb16dj"

#ifdef AUDIO_SB16DJ

/*
 *  Audio Backend Implementation for SoundBlaster (16) cards
 *  on DJGPP
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>

#include "audio.h"
#include "console.h"
#include "compat.h"

#include "../platform/_djgpp.h"

#define SB_IOBASE 0x220

#define PORT_MIXER_CMD          0x04
#define PORT_MIXER_DATA         0x05
#define PORT_DSP_RESET          0x06
#define PORT_DSP_READ           0x0A
#define PORT_DSP_WRITE          0x0C
#define PORT_DSP_READ_STATUS    0x0E
#define PORT_DSP_INT16_ACK      0x0F

#define SB_1                    0x01
#define SB_PRO                  0x02
#define SB_2                    0x03
#define SB_PRO_2                0x04
#define SB_PRO_MCA              0x05
#define SB_16                   0x06

#define DSP_CMD_SET_TIME_CONST  0x40
#define DSP_CMD_SET_RATE        0x41

#define DSP_CMD_DMA_AUTO_16_OUT 0xB6

#define DSP_CMD_SPEAKER_ON      0xD1
#define DSP_CMD_SPEAKER_OFF     0xD3

#define DSP_CMD_PB_8BIT_STOP    0xD0
#define DSP_CMD_PB_8BIT_START   0xD4
#define DSP_CMD_PB_16BIT_STOP   0xD5
#define DSP_CMD_PB_16BIT_START  0xD6

#define DSP_AUTO_DMA_16BIT_STOP 0xD9
#define DSP_AUTO_DMA_8BIT_STOP  0xDA

#define DSP_CMD_GETVERSION      0xE1

#define MIXER_CMD_MASTER_VOL    0x22
#define MIXER_CMD_SET_IRQ       0x80

#define DSP_MODE_SIGNED_STEREO  0x30

#define DMA_BUFFER_SIZE_SAMPLES 0x1000
#define DMA_BUFFER_COUNT        2

typedef struct {
    uint16_t io;
    uint8_t irq;
    uint8_t dma_l;
    uint8_t dma_h;
    uint8_t type;
} sb_config;

static sb_config s_device = { 0x220, 0x05, 0x01, 0x05, 0x06 }; // Default settings
static audio_config *s_audio_config;

static uint8_t s_interrput_vector;

static _go32_dpmi_seginfo s_old_isr;
static _go32_dpmi_seginfo s_new_isr;
static uint32_t s_buffer_linear_addr;

static _go32_dpmi_seginfo s_dma_buffer;

static uint32_t s_buffer_size;
static uint32_t s_buffer_slice;

static volatile uint8_t s_buffer_read = 0;
static volatile uint8_t s_buffer_write = 0;
static unsigned s_buffer_write_pos = 0;

static volatile bool interrupt_running = false;

static void _irq() {
    int input = inportb(s_device.io + PORT_DSP_INT16_ACK);
    interrupt_running = true;

    s_buffer_read = (s_buffer_read + 1) % DMA_BUFFER_COUNT;

//    print_msg("interrupt %d input %d", (int) s_buffer_read, input);
    interrupt_running = false;
    __irq_ack();
}

static inline void _writedsp(uint8_t value) {
    while (inportb(s_device.io + PORT_DSP_WRITE) & 0x80);
    outb(s_device.io + PORT_DSP_WRITE, value);
}

static inline uint8_t _readdsp() {
    return inportb(s_device.io + PORT_DSP_READ);
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

static void _deinit_irq() {
    __irq_mask(s_device.irq);
    __irq_set(s_device.irq, &s_old_isr);
}

static void _init_irq() {
    s_new_isr.pm_offset = (uint32_t) _irq;
    s_new_isr.pm_selector = _go32_my_cs();

    __irq_chain(s_device.irq, &s_old_isr, &s_new_isr);
    __irq_unmask(s_device.irq);
}

static void _init_dma() {
    uint16_t length = DMA_BUFFER_SIZE_SAMPLES - 1;
    // Prepare DMA channel

    __sb_dma_configure(s_device.dma_h, s_buffer_linear_addr, s_buffer_size);

    // Write DMA command & Mode
    _writedsp(DSP_CMD_DMA_AUTO_16_OUT);
    _writedsp(DSP_MODE_SIGNED_STEREO);

//    _writedsp(0x18 | (s_device.dma_h & 0x03)); // 00 0 1 10 xx

    // write *SLICE LENGTH IN SAMPLESSSSS* - 1,a

    _writedsp(length & 0xff);
    _writedsp(length >> 8);
}

static void _hwinit() {
    // Buffer info
    _go32_dpmi_seginfo tmpbuf;
    uint32_t linear_addr;
    uint8_t *clearbuf;

    /* TODO - TIME CONSTANT CALCULATION, NEEDED FOR SB < 16 CARDS!!
    // Time constant, since we want to be compatible with SB 1.x which doesn't support command 0x41!
    uint32_t time_constant = 256UL - (1000000UL / (s_audio_config->channels * s_audio_config->sample_rate));

    printf("Time constant is %lu (0x%02x)\n", time_constant, time_constant);
    */
    _writedsp(DSP_CMD_SET_RATE);
    _writedsp(s_audio_config->sample_rate >> 8);
    _writedsp(s_audio_config->sample_rate & 0xff);

/*    _writedsp(DSP_CMD_SET_TIME_CONST);
    _writedsp((uint8_t) time_constant);*/

    // Allocate DMA buffer in DOS memory

    s_buffer_slice = DMA_BUFFER_SIZE_SAMPLES * (s_audio_config->bits_per_sample/8) * s_audio_config->channels;
    s_buffer_size = s_buffer_slice * DMA_BUFFER_COUNT;
    tmpbuf.size = (s_buffer_size + 15) / 16; // (1 paragraph = 16 bytes)
    s_dma_buffer.size = tmpbuf.size;

    _go32_dpmi_allocate_dos_memory(&tmpbuf);

    // Check if buffer crosses page boundary
    s_buffer_linear_addr = (tmpbuf.rm_segment << 4); // + s_dma_buffer.rm_offset;
    if ((s_buffer_linear_addr >> 16) != ((s_buffer_linear_addr + s_buffer_size - 1) >> 16)) {
        // Allocate a new buffer instead.
        _go32_dpmi_allocate_dos_memory(&s_dma_buffer);
        _go32_dpmi_free_dos_memory(&tmpbuf);
    } else {
        s_dma_buffer = tmpbuf;
    }

    s_buffer_linear_addr = (s_dma_buffer.rm_segment << 4); // + s_dma_buffer.rm_offset;

    clearbuf = alloc_mem(s_buffer_size);
//    memset(clearbuf, 0, s_buffer_size);
    dosmemput(clearbuf, s_buffer_size, s_buffer_linear_addr);

    free(clearbuf);

    printf("Allocated DOS memory, size %lu paragraphs, linear address 0x%p\n", s_dma_buffer.size, (void*)s_buffer_linear_addr);

    // Prepare interrupt
    _init_irq();

    // Setup DMA
    _init_dma();



    sleep_ms(2000);
 //   outb(s_device.io + PORT_MIXER_CMD, MIXER_CMD_MASTER_VOL);
   // outb(s_device.io + PORT_MIXER_DATA, 0xCC);



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

    // Find SB device settings.

    while (settings) {
        settings = get_blaster_token(settings, &token_type, &token_value);
        if (!settings) break; // Error or string ended

        switch (token_type) {
        case 'A': s_device.io = token_value; break;
        case 'I': s_device.irq = token_value; break;
        case 'D': s_device.dma_l = token_value; break;
        case 'H': s_device.dma_h = token_value; break;
        case 'T': s_device.type = token_value; break;
        default: break;
        }
    }

    printf("SoundBlaster settings: \n");
    printf("    Address = %03x\n", s_device.io);
    printf("        IRQ = %3x\n", s_device.irq);
    printf("        DMA = %3x\n", s_device.dma_l);
    printf("   High DMA = %3x\n", s_device.dma_h);
    printf("       Type = %3x\n", s_device.type);

    ret = _resetdsp(s_device.io);

    if (!ret) {
        printf("DSP reset failed!\n");
        return -1;
    }

    _writedsp(DSP_CMD_GETVERSION); // Get DSP version
    dsp_version_h = _readdsp();
    dsp_version_l = _readdsp();
    printf("DSP version: %u.%02x\n", dsp_version_h, dsp_version_l);

    s_audio_config = cfg;

    _hwinit();

    // Find the sample rate and bit depth for current SB type.

    _writedsp(DSP_CMD_SPEAKER_ON);

//    system("pause");

    return 0;
}

void a_sb16dj_deinit() {
    _writedsp(DSP_CMD_PB_16BIT_STOP);

    _deinit_irq();

    // Wait IRQ to cycle once more


    _go32_dpmi_free_dos_memory(&s_dma_buffer);

    printf("UWUSCHMUWU");
    sleep_ms(1000);
}

audio_buffer_status a_sb16dj_play_buffer(uint8_t *buffer, uint32_t length) {
    volatile uint8_t tmp;
    uint32_t dst;
    unsigned free_space;
    while (length) {

        // Wait until next buffer is free

        do {
            tmp = s_buffer_read;
            if (tmp != s_buffer_write) break;
            sleep_ms(1);
        } while (tmp == s_buffer_write);


        // Get bytes left in this buffer.

        dst = s_buffer_linear_addr + s_buffer_write * s_buffer_slice + s_buffer_write_pos;
        free_space = s_buffer_slice - s_buffer_write_pos;

        if (length < free_space) {
            print_msg("copy %d bytes  %p to %p    \n", s_buffer_slice, buffer, dst);
            dosmemput(buffer, length, dst);
            s_buffer_write_pos += length;
            break;
        } else {
            print_msg("copy %d bytes  %p to %p    \n", free_space, buffer, dst);
            dosmemput(buffer, free_space, dst);
            length -= free_space;
            buffer += free_space;
            s_buffer_write_pos = 0;
            s_buffer_write = (s_buffer_write + 1) % DMA_BUFFER_COUNT;
            if (length == 0) break;
        }

    }

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
