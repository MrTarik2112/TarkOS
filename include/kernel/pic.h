/**
 * TarkOS - Programmable Interrupt Controller (8259A)
 * PIC remapping and control
 */

#ifndef _KERNEL_PIC_H
#define _KERNEL_PIC_H

#include <kernel/types.h>

/* PIC I/O ports */
#define PIC1_COMMAND    0x20    /* Master PIC command port */
#define PIC1_DATA       0x21    /* Master PIC data port */
#define PIC2_COMMAND    0xA0    /* Slave PIC command port */
#define PIC2_DATA       0xA1    /* Slave PIC data port */

/* PIC commands */
#define PIC_EOI         0x20    /* End of interrupt command */

/* Initialization Command Words (ICW) */
#define ICW1_ICW4       0x01    /* ICW4 needed */
#define ICW1_SINGLE     0x02    /* Single (vs cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (vs 8) */
#define ICW1_LEVEL      0x08    /* Level triggered (vs edge) mode */
#define ICW1_INIT       0x10    /* Initialization */

#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02    /* Auto (vs normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested (not) */

/* IRQ offset after remapping */
#define PIC1_OFFSET     0x20    /* Master PIC: IRQs 0-7  -> interrupts 32-39 */
#define PIC2_OFFSET     0x28    /* Slave PIC:  IRQs 8-15 -> interrupts 40-47 */

/**
 * Initialize and remap the PICs
 * Remaps IRQ 0-7 to interrupts 32-39 (PIC1)
 * Remaps IRQ 8-15 to interrupts 40-47 (PIC2)
 */
void pic_init(void);

/**
 * Send End of Interrupt signal to PIC
 * Must be called at end of every IRQ handler
 */
void pic_send_eoi(uint8_t irq);

/**
 * Set the IRQ mask (disable specific IRQ)
 */
void pic_set_mask(uint8_t irq);

/**
 * Clear the IRQ mask (enable specific IRQ)
 */
void pic_clear_mask(uint8_t irq);

/**
 * Disable all IRQs
 */
void pic_disable(void);

#endif /* _KERNEL_PIC_H */
