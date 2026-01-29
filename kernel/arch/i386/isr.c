/**
 * TarkOS - Interrupt Service Routines Implementation
 * High-level interrupt handling
 */

#include <kernel/isr.h>
#include <kernel/pic.h>
#include <kernel/ports.h>

/* External handler getter from idt.c */
extern isr_handler_t get_interrupt_handler(uint8_t n);

/**
 * Main ISR handler - called from assembly stub
 * Dispatches to registered handlers
 */
void isr_handler(registers_t* regs) {
    /* Get the interrupt handler */
    isr_handler_t handler = get_interrupt_handler(regs->int_no);
    
    if (handler != NULL) {
        /* Call the registered handler */
        handler(regs);
    } else if (regs->int_no < 32) {
        /* Unhandled CPU exception - this is bad */
        /* In a full OS, this would trigger a kernel panic */
        /* For now, just halt */
        CLI();
        while (1) {
            HLT();
        }
    }
    
    /* Send End of Interrupt (EOI) for hardware interrupts */
    if (regs->int_no >= 32 && regs->int_no <= 47) {
        pic_send_eoi(regs->int_no - 32);
    }
}
