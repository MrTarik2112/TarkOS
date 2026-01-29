/**
 * TarkOS - Paging
 * x86 virtual memory management
 */

#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <kernel/types.h>

/* Page directory/table entry flags */
#define PAGE_PRESENT        (1 << 0)    /* Page is present in memory */
#define PAGE_WRITE          (1 << 1)    /* Page is writable */
#define PAGE_USER           (1 << 2)    /* Page is user-accessible */
#define PAGE_WRITETHROUGH   (1 << 3)    /* Write-through caching */
#define PAGE_NOCACHE        (1 << 4)    /* Disable caching */
#define PAGE_ACCESSED       (1 << 5)    /* Page has been accessed */
#define PAGE_DIRTY          (1 << 6)    /* Page has been written to */
#define PAGE_SIZE_4MB       (1 << 7)    /* 4MB page (in page directory) */
#define PAGE_GLOBAL         (1 << 8)    /* Global page */

/* Page directory and table sizes */
#define PAGE_DIR_ENTRIES    1024
#define PAGE_TABLE_ENTRIES  1024

/* Address masks */
#define PAGE_FRAME_MASK     0xFFFFF000
#define PAGE_FLAGS_MASK     0x00000FFF

/* Get page directory/table index from virtual address */
#define PAGE_DIR_INDEX(addr)    (((uint32_t)(addr) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(addr)  (((uint32_t)(addr) >> 12) & 0x3FF)
#define PAGE_OFFSET(addr)       ((uint32_t)(addr) & 0xFFF)

/**
 * Page directory entry
 */
typedef uint32_t page_dir_entry_t;

/**
 * Page table entry
 */
typedef uint32_t page_table_entry_t;

/**
 * Page directory (contains 1024 entries)
 */
typedef struct page_directory {
    page_dir_entry_t entries[PAGE_DIR_ENTRIES];
} page_directory_t;

/**
 * Page table (contains 1024 entries)
 */
typedef struct page_table {
    page_table_entry_t entries[PAGE_TABLE_ENTRIES];
} page_table_t;

/**
 * Initialize paging
 * Sets up identity mapping for kernel and framebuffer
 */
void paging_init(void);

/**
 * Map a virtual address to a physical address
 * @param virt Virtual address
 * @param phys Physical address
 * @param flags Page flags (PAGE_PRESENT, PAGE_WRITE, etc.)
 */
void paging_map(uint32_t virt, uint32_t phys, uint32_t flags);

/**
 * Unmap a virtual address
 * @param virt Virtual address to unmap
 */
void paging_unmap(uint32_t virt);

/**
 * Get physical address from virtual address
 * @param virt Virtual address
 * @return Physical address, or 0 if not mapped
 */
uint32_t paging_get_physical(uint32_t virt);

/**
 * Enable paging
 */
void paging_enable(void);

/**
 * Disable paging
 */
void paging_disable(void);

/**
 * Flush a single TLB entry
 * @param virt Virtual address to flush
 */
void paging_flush_tlb(uint32_t virt);

/**
 * Flush the entire TLB
 */
void paging_flush_tlb_all(void);

/**
 * Identity map a range of addresses
 * @param start Start address (page-aligned)
 * @param size Size in bytes
 * @param flags Page flags
 */
void paging_identity_map(uint32_t start, uint32_t size, uint32_t flags);

#endif /* _KERNEL_PAGING_H */
