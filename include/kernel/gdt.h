/**
 * TarkOS - Global Descriptor Table
 * x86 memory segmentation setup
 */

#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <kernel/types.h>

/* GDT segment selectors */
#define GDT_NULL_SEGMENT        0x00
#define GDT_KERNEL_CODE_SEGMENT 0x08
#define GDT_KERNEL_DATA_SEGMENT 0x10
#define GDT_USER_CODE_SEGMENT   0x18
#define GDT_USER_DATA_SEGMENT   0x20
#define GDT_TSS_SEGMENT         0x28

/* Access byte flags */
#define GDT_ACCESS_PRESENT      (1 << 7)    /* Segment is present */
#define GDT_ACCESS_RING0        (0 << 5)    /* Ring 0 (kernel) */
#define GDT_ACCESS_RING3        (3 << 5)    /* Ring 3 (user) */
#define GDT_ACCESS_SYSTEM       (0 << 4)    /* System segment */
#define GDT_ACCESS_CODE_DATA    (1 << 4)    /* Code/data segment */
#define GDT_ACCESS_EXECUTABLE   (1 << 3)    /* Executable (code segment) */
#define GDT_ACCESS_DC           (1 << 2)    /* Direction/Conforming */
#define GDT_ACCESS_RW           (1 << 1)    /* Readable/Writable */
#define GDT_ACCESS_ACCESSED     (1 << 0)    /* Accessed */

/* Flags nibble */
#define GDT_FLAG_GRANULARITY    (1 << 3)    /* 4KB granularity (vs 1 byte) */
#define GDT_FLAG_32BIT          (1 << 2)    /* 32-bit segment */
#define GDT_FLAG_64BIT          (1 << 1)    /* 64-bit segment (long mode) */

/**
 * GDT entry structure (8 bytes)
 */
typedef struct gdt_entry {
    uint16_t limit_low;     /* Lower 16 bits of limit */
    uint16_t base_low;      /* Lower 16 bits of base */
    uint8_t  base_middle;   /* Middle 8 bits of base */
    uint8_t  access;        /* Access flags */
    uint8_t  granularity;   /* Flags and upper 4 bits of limit */
    uint8_t  base_high;     /* Upper 8 bits of base */
} PACKED gdt_entry_t;

/**
 * GDT pointer structure for LGDT instruction
 */
typedef struct gdt_ptr {
    uint16_t limit;         /* Size of GDT - 1 */
    uint32_t base;          /* Address of GDT */
} PACKED gdt_ptr_t;

/**
 * Initialize the Global Descriptor Table
 */
void gdt_init(void);

/**
 * Set a GDT entry
 */
void gdt_set_entry(int index, uint32_t base, uint32_t limit, 
                   uint8_t access, uint8_t flags);

/* Assembly function to load GDT */
extern void gdt_flush(uint32_t gdt_ptr);

#endif /* _KERNEL_GDT_H */
