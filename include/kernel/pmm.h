/**
 * TarkOS - Physical Memory Manager
 * Bitmap-based physical page allocator
 */

#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <kernel/types.h>
#include <kernel/multiboot.h>

/* Page size: 4KB */
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12

/* Maximum supported physical memory: 1GB (for simplicity) */
#define MAX_MEMORY          (1024 * 1024 * 1024)
#define MAX_PAGES           (MAX_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE         (MAX_PAGES / 32)

/* Page frame macros */
#define PAGE_ALIGN_DOWN(addr)   ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(addr)     (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define ADDR_TO_PAGE(addr)      ((addr) >> PAGE_SHIFT)
#define PAGE_TO_ADDR(page)      ((page) << PAGE_SHIFT)

/**
 * Initialize the physical memory manager
 * @param mboot Multiboot information structure (contains memory map)
 */
void pmm_init(multiboot_info_t* mboot);

/**
 * Allocate a single physical page
 * @return Physical address of allocated page, or 0 on failure
 */
uint32_t pmm_alloc_page(void);

/**
 * Allocate multiple contiguous physical pages
 * @param count Number of pages to allocate
 * @return Physical address of first page, or 0 on failure
 */
uint32_t pmm_alloc_pages(uint32_t count);

/**
 * Free a physical page
 * @param addr Physical address of page to free
 */
void pmm_free_page(uint32_t addr);

/**
 * Free multiple contiguous physical pages
 * @param addr Physical address of first page
 * @param count Number of pages to free
 */
void pmm_free_pages(uint32_t addr, uint32_t count);

/**
 * Mark a page as used (not available for allocation)
 * @param addr Physical address to mark as used
 */
void pmm_mark_used(uint32_t addr);

/**
 * Mark a region as used
 * @param start Start address
 * @param size Size in bytes
 */
void pmm_mark_region_used(uint32_t start, uint32_t size);

/**
 * Mark a region as free
 * @param start Start address
 * @param size Size in bytes
 */
void pmm_mark_region_free(uint32_t start, uint32_t size);

/**
 * Get total physical memory in bytes
 */
uint32_t pmm_get_total_memory(void);

/**
 * Get free physical memory in bytes
 */
uint32_t pmm_get_free_memory(void);

/**
 * Get used physical memory in bytes
 */
uint32_t pmm_get_used_memory(void);

#endif /* _KERNEL_PMM_H */
