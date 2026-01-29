/**
 * TarkOS - Multiboot1 Specification Structures
 * Based on Multiboot Specification version 0.6.96
 */

#ifndef _KERNEL_MULTIBOOT_H
#define _KERNEL_MULTIBOOT_H

#include <kernel/types.h>

/* Multiboot magic values */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002

/* Multiboot info flags */
#define MULTIBOOT_INFO_MEMORY           0x00000001
#define MULTIBOOT_INFO_BOOTDEV          0x00000002
#define MULTIBOOT_INFO_CMDLINE          0x00000004
#define MULTIBOOT_INFO_MODS             0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS        0x00000010
#define MULTIBOOT_INFO_ELF_SHDR         0x00000020
#define MULTIBOOT_INFO_MEM_MAP          0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO       0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE     0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME 0x00000200
#define MULTIBOOT_INFO_APM_TABLE        0x00000400
#define MULTIBOOT_INFO_VBE_INFO         0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO 0x00001000

/* Framebuffer types */
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED  0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB      1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2

/* Memory map entry types */
#define MULTIBOOT_MEMORY_AVAILABLE          1
#define MULTIBOOT_MEMORY_RESERVED           2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE   3
#define MULTIBOOT_MEMORY_NVS                4
#define MULTIBOOT_MEMORY_BADRAM             5

/**
 * Multiboot memory map entry
 */
typedef struct multiboot_mmap_entry {
    uint32_t size;          /* Size of this entry (not including size field) */
    uint64_t addr;          /* Base address */
    uint64_t len;           /* Length in bytes */
    uint32_t type;          /* Type (1 = available, other = reserved) */
} PACKED multiboot_mmap_entry_t;

/**
 * Multiboot module structure
 */
typedef struct multiboot_mod {
    uint32_t mod_start;     /* Module start address */
    uint32_t mod_end;       /* Module end address */
    uint32_t cmdline;       /* Module command line */
    uint32_t reserved;      /* Must be 0 */
} PACKED multiboot_mod_t;

/**
 * Multiboot information structure
 * Passed to kernel by bootloader
 */
typedef struct multiboot_info {
    uint32_t flags;         /* Feature flags */
    
    /* Memory info (flags bit 0) */
    uint32_t mem_lower;     /* Lower memory in KB (starts at 0) */
    uint32_t mem_upper;     /* Upper memory in KB (starts at 1MB) */
    
    /* Boot device (flags bit 1) */
    uint32_t boot_device;
    
    /* Kernel command line (flags bit 2) */
    uint32_t cmdline;
    
    /* Module info (flags bit 3) */
    uint32_t mods_count;
    uint32_t mods_addr;
    
    /* Symbol table info (flags bit 4 or 5) */
    union {
        struct {
            uint32_t tabsize;
            uint32_t strsize;
            uint32_t addr;
            uint32_t reserved;
        } aout_sym;
        struct {
            uint32_t num;
            uint32_t size;
            uint32_t addr;
            uint32_t shndx;
        } elf_sec;
    };
    
    /* Memory map (flags bit 6) */
    uint32_t mmap_length;
    uint32_t mmap_addr;
    
    /* Drives info (flags bit 7) */
    uint32_t drives_length;
    uint32_t drives_addr;
    
    /* ROM configuration table (flags bit 8) */
    uint32_t config_table;
    
    /* Boot loader name (flags bit 9) */
    uint32_t boot_loader_name;
    
    /* APM table (flags bit 10) */
    uint32_t apm_table;
    
    /* VBE info (flags bit 11) */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    
    /* Framebuffer info (flags bit 12) */
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    
    /* Color info (varies by framebuffer type) */
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
} PACKED multiboot_info_t;

#endif /* _KERNEL_MULTIBOOT_H */
