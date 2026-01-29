/**
 * TarkOS - Global Descriptor Table Implementation
 * Sets up flat memory model for 32-bit protected mode
 */

#include <kernel/gdt.h>

/* GDT entries */
#define GDT_ENTRIES 6

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdt_ptr;

/**
 * Set a GDT entry
 */
void gdt_set_entry(int index, uint32_t base, uint32_t limit, 
                   uint8_t access, uint8_t flags) {
    gdt[index].base_low = (uint16_t)(base & 0xFFFF);
    gdt[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[index].base_high = (uint8_t)((base >> 24) & 0xFF);
    
    gdt[index].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[index].granularity = (uint8_t)((limit >> 16) & 0x0F);
    gdt[index].granularity |= (flags & 0xF0);
    
    gdt[index].access = access;
}

/**
 * Initialize the Global Descriptor Table
 * Sets up flat memory model: base=0, limit=4GB
 */
void gdt_init(void) {
    /* Set up GDT pointer */
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base = (uint32_t)&gdt;
    
    /* Null segment (required) */
    gdt_set_entry(0, 0, 0, 0, 0);
    
    /* Kernel code segment
     * Base: 0x00000000
     * Limit: 0xFFFFFFFF (4GB with granularity)
     * Access: Present, Ring 0, Code/Data, Executable, Readable
     * Flags: 4KB granularity, 32-bit
     */
    gdt_set_entry(1, 0, 0xFFFFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_DATA |
        GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
        GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    
    /* Kernel data segment
     * Base: 0x00000000
     * Limit: 0xFFFFFFFF (4GB with granularity)
     * Access: Present, Ring 0, Code/Data, Writable
     * Flags: 4KB granularity, 32-bit
     */
    gdt_set_entry(2, 0, 0xFFFFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_DATA |
        GDT_ACCESS_RW,
        GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    
    /* User code segment
     * Base: 0x00000000
     * Limit: 0xFFFFFFFF (4GB with granularity)
     * Access: Present, Ring 3, Code/Data, Executable, Readable
     * Flags: 4KB granularity, 32-bit
     */
    gdt_set_entry(3, 0, 0xFFFFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_DATA |
        GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
        GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    
    /* User data segment
     * Base: 0x00000000
     * Limit: 0xFFFFFFFF (4GB with granularity)
     * Access: Present, Ring 3, Code/Data, Writable
     * Flags: 4KB granularity, 32-bit
     */
    gdt_set_entry(4, 0, 0xFFFFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_DATA |
        GDT_ACCESS_RW,
        GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    
    /* TSS segment (placeholder - can be set up later for task switching) */
    gdt_set_entry(5, 0, 0, 0, 0);
    
    /* Load the GDT */
    gdt_flush((uint32_t)&gdt_ptr);
}
