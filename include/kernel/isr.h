/**
 * TarkOS - Interrupt Service Routines
 * High-level interrupt handling
 */

#ifndef _KERNEL_ISR_H
#define _KERNEL_ISR_H

#include <kernel/types.h>
#include <kernel/idt.h>

/* CPU exception names */
static const char* const exception_names[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

/* IRQ numbers (after remapping) */
#define IRQ0    32      /* Timer */
#define IRQ1    33      /* Keyboard */
#define IRQ2    34      /* Cascade (never raised) */
#define IRQ3    35      /* COM2 */
#define IRQ4    36      /* COM1 */
#define IRQ5    37      /* LPT2 */
#define IRQ6    38      /* Floppy Disk */
#define IRQ7    39      /* LPT1 / Spurious */
#define IRQ8    40      /* CMOS RTC */
#define IRQ9    41      /* Free */
#define IRQ10   42      /* Free */
#define IRQ11   43      /* Free */
#define IRQ12   44      /* PS/2 Mouse */
#define IRQ13   45      /* FPU */
#define IRQ14   46      /* Primary ATA */
#define IRQ15   47      /* Secondary ATA */

/**
 * Main interrupt handler called from assembly stub
 */
void isr_handler(registers_t* regs);

#endif /* _KERNEL_ISR_H */
