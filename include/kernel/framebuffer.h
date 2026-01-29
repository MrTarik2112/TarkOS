/**
 * TarkOS - Framebuffer Driver
 * VESA VBE linear framebuffer access
 */

#ifndef _KERNEL_FRAMEBUFFER_H
#define _KERNEL_FRAMEBUFFER_H

#include <kernel/types.h>
#include <kernel/multiboot.h>

/* Color macros (32-bit BGRA format) */
#define RGB(r, g, b)        ((uint32_t)(((uint8_t)(b)) | ((uint8_t)(g) << 8) | ((uint8_t)(r) << 16)))
#define RGBA(r, g, b, a)    ((uint32_t)(((uint8_t)(b)) | ((uint8_t)(g) << 8) | ((uint8_t)(r) << 16) | ((uint8_t)(a) << 24)))

/* Common colors */
#define COLOR_BLACK         RGB(0, 0, 0)
#define COLOR_WHITE         RGB(255, 255, 255)
#define COLOR_RED           RGB(255, 0, 0)
#define COLOR_GREEN         RGB(0, 255, 0)
#define COLOR_BLUE          RGB(0, 0, 255)
#define COLOR_YELLOW        RGB(255, 255, 0)
#define COLOR_CYAN          RGB(0, 255, 255)
#define COLOR_MAGENTA       RGB(255, 0, 255)
#define COLOR_GRAY          RGB(128, 128, 128)
#define COLOR_DARK_GRAY     RGB(64, 64, 64)
#define COLOR_LIGHT_GRAY    RGB(192, 192, 192)

/* Desktop colors */
#define COLOR_DESKTOP_BG    RGB(0, 120, 215)    /* Windows 10 blue */
#define COLOR_TITLEBAR      RGB(0, 100, 180)
#define COLOR_TITLEBAR_INACTIVE RGB(128, 128, 128)
#define COLOR_WINDOW_BG     RGB(240, 240, 240)
#define COLOR_WINDOW_BORDER RGB(100, 100, 100)

/* Framebuffer info structure */
typedef struct {
    uint32_t* address;          /* Framebuffer address */
    uint32_t  width;            /* Width in pixels */
    uint32_t  height;           /* Height in pixels */
    uint32_t  pitch;            /* Bytes per scanline */
    uint8_t   bpp;              /* Bits per pixel */
    uint32_t  size;             /* Total framebuffer size in bytes */
} framebuffer_info_t;

/**
 * Initialize the framebuffer
 * @param mboot Multiboot information (contains framebuffer info)
 * @return true if framebuffer initialized successfully
 */
bool fb_init(multiboot_info_t* mboot);

/**
 * Get framebuffer info
 */
framebuffer_info_t* fb_get_info(void);

/**
 * Get framebuffer address
 */
uint32_t* fb_get_address(void);

/**
 * Get screen width
 */
uint32_t fb_get_width(void);

/**
 * Get screen height
 */
uint32_t fb_get_height(void);

/**
 * Put a pixel at (x, y) with specified color
 */
void fb_put_pixel(int32_t x, int32_t y, uint32_t color);

/**
 * Get pixel color at (x, y)
 */
uint32_t fb_get_pixel(int32_t x, int32_t y);

/**
 * Clear screen with specified color
 */
void fb_clear(uint32_t color);

/**
 * Copy rectangular region
 */
void fb_copy_rect(int32_t src_x, int32_t src_y, 
                  int32_t dst_x, int32_t dst_y,
                  uint32_t width, uint32_t height);

/**
 * Swap buffers (for double buffering)
 * Copies back buffer to front buffer
 */
void fb_swap_buffers(void);

/**
 * Get the back buffer address
 */
uint32_t* fb_get_back_buffer(void);

/**
 * Enable/disable double buffering
 */
void fb_set_double_buffer(bool enable);

/**
 * Check if double buffering is enabled
 */
bool fb_is_double_buffered(void);

#endif /* _KERNEL_FRAMEBUFFER_H */
