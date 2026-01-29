/**
 * TarkOS - Programmable Interrupt Controller Implementation
 * 8259A PIC remapping and control
 */

#include <kernel/pic.h>
#include <kernel/ports.h>

/**
 * Initialize and remap the PICs
 * By default, IRQ 0-7 map to interrupts 0x08-0x0F (conflict with CPU exceptions)
 * By default, IRQ 8-15 map to interrupts 0x70-0x77
 * We remap to:
 *   IRQ 0-7  -> interrupts 32-39 (0x20-0x27)
 *   IRQ 8-15 -> interrupts 40-47 (0x28-0x2F)
 */
void pic_init(void) {
    uint8_t mask1, mask2;
    
    /* Save current masks */
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);
    
    /* Start initialization sequence (cascade mode) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    /* ICW2: Set vector offsets */
    outb(PIC1_DATA, PIC1_OFFSET);    /* Master PIC: IRQ 0-7 -> INT 32-39 */
    io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);    /* Slave PIC: IRQ 8-15 -> INT 40-47 */
    io_wait();
    
    /* ICW3: Configure cascade */
    outb(PIC1_DATA, 0x04);           /* Tell Master PIC there's a slave at IRQ2 */
    io_wait();
    outb(PIC2_DATA, 0x02);           /* Tell Slave PIC its cascade identity */
    io_wait();
    
    /* ICW4: Set mode */
    outb(PIC1_DATA, ICW4_8086);      /* 8086 mode */
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    /* Restore saved masks (or set all to disabled initially) */
    /* Enable only keyboard (IRQ1) and cascade (IRQ2) for now */
    outb(PIC1_DATA, 0xF8);           /* 11111000: Enable IRQ0, IRQ1, IRQ2 */
    outb(PIC2_DATA, 0xEF);           /* 11101111: Enable IRQ12 (mouse) */
    
    (void)mask1;
    (void)mask2;
}

/**
 * Send End of Interrupt signal to PIC
 * Must be called at end of every IRQ handler
 */
void pic_send_eoi(uint8_t irq) {
    /* If IRQ came from slave PIC, send EOI to both */
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    /* Always send EOI to master PIC */
    outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Set the IRQ mask (disable specific IRQ)
 */
void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) | (1 << irq);
    outb(port, value);
}

/**
 * Clear the IRQ mask (enable specific IRQ)
 */
void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

/**
 * Disable all IRQs
 */
void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}
