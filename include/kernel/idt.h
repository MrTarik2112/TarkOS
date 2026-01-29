/**
 * TarkOS - Interrupt Descriptor Table
 * x86 interrupt handling setup
 */

#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <kernel/types.h>

/* Number of IDT entries */
#define IDT_ENTRIES 256

/* IDT gate types */
#define IDT_GATE_TASK       0x05    /* Task gate */
#define IDT_GATE_INT16      0x06    /* 16-bit interrupt gate */
#define IDT_GATE_TRAP16     0x07    /* 16-bit trap gate */
#define IDT_GATE_INT32      0x0E    /* 32-bit interrupt gate */
#define IDT_GATE_TRAP32     0x0F    /* 32-bit trap gate */

/* IDT flags */
#define IDT_FLAG_PRESENT    (1 << 7)    /* Gate is present */
#define IDT_FLAG_RING0      (0 << 5)    /* Ring 0 */
#define IDT_FLAG_RING3      (3 << 5)    /* Ring 3 */

/**
 * IDT entry structure (8 bytes)
 */
typedef struct idt_entry {
    uint16_t base_low;      /* Lower 16 bits of handler address */
    uint16_t selector;      /* Kernel code segment selector */
    uint8_t  zero;          /* Must be zero */
    uint8_t  flags;         /* Type and attributes */
    uint16_t base_high;     /* Upper 16 bits of handler address */
} PACKED idt_entry_t;

/**
 * IDT pointer structure for LIDT instruction
 */
typedef struct idt_ptr {
    uint16_t limit;         /* Size of IDT - 1 */
    uint32_t base;          /* Address of IDT */
} PACKED idt_ptr_t;

/**
 * Registers pushed by ISR stub
 */
typedef struct registers {
    uint32_t ds;                                    /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha */
    uint32_t int_no, err_code;                      /* Interrupt number and error code */
    uint32_t eip, cs, eflags, useresp, ss;          /* Pushed by CPU */
} registers_t;

/* Interrupt handler function type */
typedef void (*isr_handler_t)(registers_t* regs);

/**
 * Initialize the Interrupt Descriptor Table
 */
void idt_init(void);

/**
 * Set an IDT entry
 */
void idt_set_entry(int index, uint32_t handler, uint16_t selector, uint8_t flags);

/**
 * Register an interrupt handler
 */
void register_interrupt_handler(uint8_t n, isr_handler_t handler);

/* Assembly function to load IDT */
extern void idt_load(uint32_t idt_ptr);

/* ISR stub declarations (defined in boot.asm) */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/* IRQ stubs (32-47) */
extern void isr32(void);
extern void isr33(void);
extern void isr34(void);
extern void isr35(void);
extern void isr36(void);
extern void isr37(void);
extern void isr38(void);
extern void isr39(void);
extern void isr40(void);
extern void isr41(void);
extern void isr42(void);
extern void isr43(void);
extern void isr44(void);
extern void isr45(void);
extern void isr46(void);
extern void isr47(void);

#endif /* _KERNEL_IDT_H */
