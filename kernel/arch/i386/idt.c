/**
 * TarkOS - Interrupt Descriptor Table Implementation
 * Sets up CPU exception and IRQ handlers
 */

#include <kernel/idt.h>
#include <kernel/gdt.h>

/* IDT entries */
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

/* Interrupt handlers array */
static isr_handler_t interrupt_handlers[IDT_ENTRIES];

/**
 * Set an IDT entry
 */
void idt_set_entry(int index, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[index].base_low = (uint16_t)(handler & 0xFFFF);
    idt[index].base_high = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[index].selector = selector;
    idt[index].zero = 0;
    idt[index].flags = flags;
}

/**
 * Register an interrupt handler
 */
void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

/**
 * Get interrupt handler
 */
isr_handler_t get_interrupt_handler(uint8_t n) {
    return interrupt_handlers[n];
}

/**
 * Initialize the Interrupt Descriptor Table
 */
void idt_init(void) {
    /* Set up IDT pointer */
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base = (uint32_t)&idt;
    
    /* Clear all entries */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_entry(i, 0, 0, 0);
        interrupt_handlers[i] = NULL;
    }
    
    /* CPU exception handlers (0-31) */
    idt_set_entry(0, (uint32_t)isr0, GDT_KERNEL_CODE_SEGMENT, 
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(1, (uint32_t)isr1, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(2, (uint32_t)isr2, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(3, (uint32_t)isr3, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(4, (uint32_t)isr4, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(5, (uint32_t)isr5, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(6, (uint32_t)isr6, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(7, (uint32_t)isr7, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(8, (uint32_t)isr8, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(9, (uint32_t)isr9, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(10, (uint32_t)isr10, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(11, (uint32_t)isr11, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(12, (uint32_t)isr12, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(13, (uint32_t)isr13, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(14, (uint32_t)isr14, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(15, (uint32_t)isr15, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(16, (uint32_t)isr16, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(17, (uint32_t)isr17, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(18, (uint32_t)isr18, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(19, (uint32_t)isr19, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(20, (uint32_t)isr20, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(21, (uint32_t)isr21, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(22, (uint32_t)isr22, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(23, (uint32_t)isr23, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(24, (uint32_t)isr24, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(25, (uint32_t)isr25, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(26, (uint32_t)isr26, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(27, (uint32_t)isr27, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(28, (uint32_t)isr28, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(29, (uint32_t)isr29, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(30, (uint32_t)isr30, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(31, (uint32_t)isr31, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    
    /* IRQ handlers (32-47) - after PIC remapping */
    idt_set_entry(32, (uint32_t)isr32, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(33, (uint32_t)isr33, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(34, (uint32_t)isr34, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(35, (uint32_t)isr35, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(36, (uint32_t)isr36, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(37, (uint32_t)isr37, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(38, (uint32_t)isr38, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(39, (uint32_t)isr39, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(40, (uint32_t)isr40, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(41, (uint32_t)isr41, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(42, (uint32_t)isr42, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(43, (uint32_t)isr43, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(44, (uint32_t)isr44, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(45, (uint32_t)isr45, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(46, (uint32_t)isr46, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    idt_set_entry(47, (uint32_t)isr47, GDT_KERNEL_CODE_SEGMENT,
                  IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_GATE_INT32);
    
    /* Load the IDT */
    idt_load((uint32_t)&idt_ptr);
}
