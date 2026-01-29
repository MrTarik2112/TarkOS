/**
 * TarkOS - Paging Implementation
 * x86 virtual memory management with identity mapping
 */

#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <lib/string.h>

/* Page directory - must be page-aligned */
static page_directory_t kernel_page_directory __attribute__((aligned(PAGE_SIZE)));

/* Page tables for identity mapping first 16MB (4 tables * 4MB each) */
static page_table_t kernel_page_tables[4] __attribute__((aligned(PAGE_SIZE)));

/* Current page directory */
static page_directory_t* current_page_directory = NULL;

/* External kernel symbols */
extern uint32_t _kernel_end;

/**
 * Load page directory into CR3
 */
static inline void load_page_directory(page_directory_t* dir) {
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)dir));
}

/**
 * Get current page directory from CR3
 */
static inline page_directory_t* get_page_directory(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return (page_directory_t*)(cr3 & PAGE_FRAME_MASK);
}

/**
 * Enable paging by setting bit 31 of CR0
 */
void paging_enable(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  /* Set PG bit */
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

/**
 * Disable paging by clearing bit 31 of CR0
 */
void paging_disable(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000;  /* Clear PG bit */
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

/**
 * Flush a single TLB entry
 */
void paging_flush_tlb(uint32_t virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

/**
 * Flush the entire TLB by reloading CR3
 */
void paging_flush_tlb_all(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));
}

/**
 * Get or create page table for a virtual address
 */
static page_table_t* get_page_table(uint32_t virt, bool create) {
    uint32_t dir_index = PAGE_DIR_INDEX(virt);
    page_dir_entry_t* entry = &current_page_directory->entries[dir_index];
    
    if (!(*entry & PAGE_PRESENT)) {
        if (!create) {
            return NULL;
        }
        
        /* Allocate a new page table */
        uint32_t table_phys = pmm_alloc_page();
        if (table_phys == 0) {
            return NULL;  /* Out of memory */
        }
        
        /* Clear the new page table */
        memset((void*)table_phys, 0, PAGE_SIZE);
        
        /* Set up page directory entry */
        *entry = table_phys | PAGE_PRESENT | PAGE_WRITE;
    }
    
    return (page_table_t*)(*entry & PAGE_FRAME_MASK);
}

/**
 * Initialize paging with identity mapping
 */
void paging_init(void) {
    /* Clear page directory */
    memset(&kernel_page_directory, 0, sizeof(page_directory_t));
    
    /* Set up identity mapping for first 16MB using pre-allocated tables */
    for (int i = 0; i < 4; i++) {
        /* Clear page table */
        memset(&kernel_page_tables[i], 0, sizeof(page_table_t));
        
        /* Fill page table with identity mappings */
        for (int j = 0; j < PAGE_TABLE_ENTRIES; j++) {
            uint32_t phys_addr = (i * PAGE_TABLE_ENTRIES + j) * PAGE_SIZE;
            kernel_page_tables[i].entries[j] = phys_addr | PAGE_PRESENT | PAGE_WRITE;
        }
        
        /* Add page table to page directory */
        kernel_page_directory.entries[i] = 
            (uint32_t)&kernel_page_tables[i] | PAGE_PRESENT | PAGE_WRITE;
    }
    
    /* Set current page directory */
    current_page_directory = &kernel_page_directory;
    
    /* Load page directory and enable paging */
    load_page_directory(&kernel_page_directory);
    paging_enable();
}

/**
 * Map a virtual address to a physical address
 */
void paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    page_table_t* table = get_page_table(virt, true);
    if (table == NULL) {
        return;  /* Failed to get/create page table */
    }
    
    uint32_t table_index = PAGE_TABLE_INDEX(virt);
    table->entries[table_index] = (phys & PAGE_FRAME_MASK) | (flags & PAGE_FLAGS_MASK);
    
    paging_flush_tlb(virt);
}

/**
 * Unmap a virtual address
 */
void paging_unmap(uint32_t virt) {
    page_table_t* table = get_page_table(virt, false);
    if (table == NULL) {
        return;  /* Page table doesn't exist */
    }
    
    uint32_t table_index = PAGE_TABLE_INDEX(virt);
    table->entries[table_index] = 0;
    
    paging_flush_tlb(virt);
}

/**
 * Get physical address from virtual address
 */
uint32_t paging_get_physical(uint32_t virt) {
    page_table_t* table = get_page_table(virt, false);
    if (table == NULL) {
        return 0;
    }
    
    uint32_t table_index = PAGE_TABLE_INDEX(virt);
    page_table_entry_t entry = table->entries[table_index];
    
    if (!(entry & PAGE_PRESENT)) {
        return 0;
    }
    
    return (entry & PAGE_FRAME_MASK) | PAGE_OFFSET(virt);
}

/**
 * Identity map a range of addresses
 */
void paging_identity_map(uint32_t start, uint32_t size, uint32_t flags) {
    uint32_t addr = PAGE_ALIGN_DOWN(start);
    uint32_t end = PAGE_ALIGN_UP(start + size);
    
    while (addr < end) {
        paging_map(addr, addr, flags);
        addr += PAGE_SIZE;
    }
}
