/**
 * TarkOS - Physical Memory Manager Implementation
 * Bitmap-based physical page allocator
 */

#include <kernel/pmm.h>
#include <lib/string.h>

/* Bitmap to track page usage (1 bit per 4KB page) */
static uint32_t pmm_bitmap[BITMAP_SIZE];

/* Memory statistics */
static uint32_t total_memory = 0;
static uint32_t used_pages = 0;
static uint32_t total_pages = 0;

/* External symbols from linker script */
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

/**
 * Set a bit in the bitmap (mark page as used)
 */
static inline void bitmap_set(uint32_t page) {
    pmm_bitmap[page / 32] |= (1 << (page % 32));
}

/**
 * Clear a bit in the bitmap (mark page as free)
 */
static inline void bitmap_clear(uint32_t page) {
    pmm_bitmap[page / 32] &= ~(1 << (page % 32));
}

/**
 * Test if a bit is set in the bitmap
 */
static inline bool bitmap_test(uint32_t page) {
    return (pmm_bitmap[page / 32] & (1 << (page % 32))) != 0;
}

/**
 * Find first free page in bitmap
 */
static uint32_t bitmap_find_free(void) {
    for (uint32_t i = 0; i < total_pages / 32; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            /* Found a block with at least one free page */
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t page = i * 32 + j;
                if (page < total_pages && !bitmap_test(page)) {
                    return page;
                }
            }
        }
    }
    return (uint32_t)-1;  /* No free pages */
}

/**
 * Find contiguous free pages
 */
static uint32_t bitmap_find_free_contiguous(uint32_t count) {
    uint32_t start = 0;
    uint32_t found = 0;
    
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            if (found == 0) {
                start = i;
            }
            found++;
            if (found == count) {
                return start;
            }
        } else {
            found = 0;
        }
    }
    return (uint32_t)-1;  /* Not enough contiguous pages */
}

/**
 * Initialize the physical memory manager
 */
void pmm_init(multiboot_info_t* mboot) {
    /* Start by marking all memory as used */
    memset(pmm_bitmap, 0xFF, sizeof(pmm_bitmap));
    
    /* Check if memory map is available */
    if (!(mboot->flags & MULTIBOOT_INFO_MEM_MAP)) {
        /* No memory map - use basic memory info */
        total_memory = (mboot->mem_upper + 1024) * 1024;  /* mem_upper is in KB, starting at 1MB */
        total_pages = total_memory / PAGE_SIZE;
        used_pages = total_pages;
        return;
    }
    
    /* Parse memory map and mark available regions as free */
    multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mboot->mmap_addr;
    multiboot_mmap_entry_t* mmap_end = (multiboot_mmap_entry_t*)(mboot->mmap_addr + mboot->mmap_length);
    
    while (mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t start = mmap->addr;
            uint64_t end = start + mmap->len;
            
            /* Skip memory below 1MB (BIOS, VGA, etc.) */
            if (start < 0x100000) {
                start = 0x100000;
            }
            
            /* Limit to MAX_MEMORY */
            if (end > MAX_MEMORY) {
                end = MAX_MEMORY;
            }
            
            if (start < end) {
                /* Update total memory */
                if (end > total_memory) {
                    total_memory = (uint32_t)end;
                }
                
                /* Mark region as free */
                pmm_mark_region_free((uint32_t)start, (uint32_t)(end - start));
            }
        }
        
        /* Move to next entry */
        mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
    
    /* Calculate total pages */
    total_pages = total_memory / PAGE_SIZE;
    
    /* Mark kernel region as used */
    uint32_t kernel_start = (uint32_t)&_kernel_start;
    uint32_t kernel_end = (uint32_t)&_kernel_end;
    pmm_mark_region_used(kernel_start, kernel_end - kernel_start);
    
    /* Mark first 1MB as used (BIOS, VGA, etc.) */
    pmm_mark_region_used(0, 0x100000);
    
    /* Count used pages */
    used_pages = 0;
    for (uint32_t i = 0; i < total_pages; i++) {
        if (bitmap_test(i)) {
            used_pages++;
        }
    }
}

/**
 * Allocate a single physical page
 */
uint32_t pmm_alloc_page(void) {
    uint32_t page = bitmap_find_free();
    if (page == (uint32_t)-1) {
        return 0;  /* Out of memory */
    }
    
    bitmap_set(page);
    used_pages++;
    
    return PAGE_TO_ADDR(page);
}

/**
 * Allocate multiple contiguous physical pages
 */
uint32_t pmm_alloc_pages(uint32_t count) {
    if (count == 0) {
        return 0;
    }
    if (count == 1) {
        return pmm_alloc_page();
    }
    
    uint32_t start = bitmap_find_free_contiguous(count);
    if (start == (uint32_t)-1) {
        return 0;  /* Not enough contiguous memory */
    }
    
    /* Mark all pages as used */
    for (uint32_t i = 0; i < count; i++) {
        bitmap_set(start + i);
        used_pages++;
    }
    
    return PAGE_TO_ADDR(start);
}

/**
 * Free a physical page
 */
void pmm_free_page(uint32_t addr) {
    uint32_t page = ADDR_TO_PAGE(addr);
    if (page >= total_pages) {
        return;
    }
    
    if (bitmap_test(page)) {
        bitmap_clear(page);
        used_pages--;
    }
}

/**
 * Free multiple contiguous physical pages
 */
void pmm_free_pages(uint32_t addr, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        pmm_free_page(addr + i * PAGE_SIZE);
    }
}

/**
 * Mark a page as used
 */
void pmm_mark_used(uint32_t addr) {
    uint32_t page = ADDR_TO_PAGE(addr);
    if (page < total_pages && !bitmap_test(page)) {
        bitmap_set(page);
        used_pages++;
    }
}

/**
 * Mark a region as used
 */
void pmm_mark_region_used(uint32_t start, uint32_t size) {
    uint32_t addr = PAGE_ALIGN_DOWN(start);
    uint32_t end = PAGE_ALIGN_UP(start + size);
    
    while (addr < end) {
        pmm_mark_used(addr);
        addr += PAGE_SIZE;
    }
}

/**
 * Mark a region as free
 */
void pmm_mark_region_free(uint32_t start, uint32_t size) {
    uint32_t addr = PAGE_ALIGN_UP(start);
    uint32_t end = PAGE_ALIGN_DOWN(start + size);
    
    while (addr < end) {
        uint32_t page = ADDR_TO_PAGE(addr);
        if (page < MAX_PAGES) {
            bitmap_clear(page);
        }
        addr += PAGE_SIZE;
    }
}

/**
 * Get total physical memory in bytes
 */
uint32_t pmm_get_total_memory(void) {
    return total_memory;
}

/**
 * Get free physical memory in bytes
 */
uint32_t pmm_get_free_memory(void) {
    return (total_pages - used_pages) * PAGE_SIZE;
}

/**
 * Get used physical memory in bytes
 */
uint32_t pmm_get_used_memory(void) {
    return used_pages * PAGE_SIZE;
}
