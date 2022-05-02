/* Helper code & definitions for DJGPP targets */

#ifndef _DJGPP_H
#define _DJGPP_H

#include <pc.h>
#include <dpmi.h>

#define PORT_PIC_1_DATA             0x21
#define PORT_PIC_2_DATA             0xA1


static inline void outb(uint16_t p, uint8_t v) {
    printf("WRITE to Port 0x%03x Value 0x%02x\n", p, v);
    outportb(p,v);
}

static inline void __irq_ack() {
    outportb(0x20, 0x20);
}

static inline void __irq_unmask_pri(uint8_t x) {
    outb(PORT_PIC_1_DATA, inportb(PORT_PIC_1_DATA) & ~(1 << x));
}

static inline void __irq_unmask_sec(uint8_t x) {
    outb(PORT_PIC_2_DATA, inportb(PORT_PIC_2_DATA) & ~(1 << x));
}

static inline void __irq_mask_pri(uint8_t x) {
    outb(PORT_PIC_1_DATA, inportb(PORT_PIC_1_DATA) | (1 << x));
}

static inline void __irq_mask_sec(uint8_t x) {
    outb(PORT_PIC_2_DATA, inportb(PORT_PIC_2_DATA) | (1 << x));
}

static inline uint8_t __irq_get_vector(uint8_t irq) {
    switch(irq) {
    case 2:     return 0x71;
    case 10:    return 0x72;
    case 11:    return 0x73;
    default:    return irq + 8;
    }
}

static inline void __irq_mask(uint8_t irq)
// Masks (disables) an interrupt, IRQs 2, 10 and 11 are chained and delegated to second PIC.
{
    uint8_t vector = __irq_get_vector(irq);
    if (irq > 0x70) {
        __irq_mask_pri(1 + (vector & 0x0F));
        __irq_mask_sec(2);
    } else {
        __irq_mask_pri(irq);
    }
}
static inline void __irq_unmask(uint8_t irq)
// Unmasks (Enables) an interrupt, IRQs 2, 10 and 11 are chained and delegated to second PIC.
{
    uint8_t vector = __irq_get_vector(irq);
    printf("%s(%d)\n", __func__, (int)irq);
    if (irq > 0x70) {
        __irq_unmask_pri(1 + (vector & 0x0F));
        __irq_unmask_sec(2);
    } else {
        __irq_unmask_pri(irq);
    }
}

static inline void __irq_chain(uint8_t irq, _go32_dpmi_seginfo *old_isr, _go32_dpmi_seginfo *new_isr) {
    uint8_t vector = __irq_get_vector(irq);
    _go32_dpmi_get_protected_mode_interrupt_vector(vector, old_isr);
    _go32_dpmi_chain_protected_mode_interrupt_vector(vector, new_isr);
}

static inline void __irq_set(uint8_t irq, _go32_dpmi_seginfo *isr) {
    //Set interrupt vector to the BIOS handler
    _go32_dpmi_set_protected_mode_interrupt_vector (__irq_get_vector(irq), isr);
}


/*********************
 * DMA helpers
 *********************/

#define PORT_DMA_LOW_CHANNEL_MASK   0x0A
#define PORT_DMA_LOW_MODE_SET       0x0B
#define PORT_DMA_LOW_FLIPFLOP_RESET 0x0C

static inline uint8_t DMA_PAGE_REGS[8] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };

#define __DMA_ADJUST(channel, port) { ((channel) > 3) ? (((port) << 1 ) + 0xC0) : (port) }

static inline void __sb_dma_configure(uint8_t channel, uint32_t addr, uint16_t block_length) {
    uint8_t page = addr >> 16;
    uint16_t offset;

    uint8_t mask_port = __DMA_ADJUST(channel, PORT_DMA_LOW_CHANNEL_MASK);
    uint8_t ff_reset_port = __DMA_ADJUST(channel, PORT_DMA_LOW_FLIPFLOP_RESET);
    uint8_t mode_set_port = __DMA_ADJUST(channel, PORT_DMA_LOW_MODE_SET);
    uint8_t channel_adjusted = channel % 4;
    uint8_t channel_addr_port = __DMA_ADJUST(channel, channel_adjusted << 1);
    uint8_t channel_length_port = __DMA_ADJUST(channel, (channel_adjusted << 1) + 1);

    printf("%s addr = %p\n", __func__, addr);

    if (channel > 7) abort();

    if (channel > 3) {
        addr >>= 1;                      // For 16 Bit DMAs we count words!!
        block_length >>= 1;
    }

    offset = addr & 0xffff;

    block_length -= 1;

    outb(mask_port, 4 | channel_adjusted);              // Disable this channel
    outb(ff_reset_port, 0);

    outb(mode_set_port, 0x58 | channel_adjusted);       // Mode set register


//    block_length -= 1;

    outb(channel_addr_port, offset & 0xff);             // Offset
    outb(channel_addr_port, offset >> 8);

    outb(DMA_PAGE_REGS[channel], page);                 // Page

    outb(ff_reset_port, 0);

    outb(channel_length_port, block_length & 0xff);     // Block length
    outb(channel_length_port, block_length >> 8);

    outb(mask_port, channel_adjusted);                  // Enable this channel
}

#endif
