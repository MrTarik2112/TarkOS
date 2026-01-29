/**
 * TarkOS - Framebuffer Driver Implementation
 * VESA VBE linear framebuffer with double buffering
 */

#include <kernel/framebuffer.h>
#include <lib/string.h>

/* Framebuffer info */
static framebuffer_info_t fb_info;

/* Back buffer for double buffering */
static uint32_t* back_buffer = NULL;
static bool double_buffering = false;

/* Current drawing target */
static uint32_t* draw_target = NULL;

/**
 * Initialize the framebuffer
 */
bool fb_init(multiboot_info_t* mboot) {
    /* Check if framebuffer info is available */
    if (!(mboot->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
        return false;
    }
    
    /* Check framebuffer type (must be RGB) */
    if (mboot->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
        return false;
    }
    
    /* Store framebuffer info */
    fb_info.address = (uint32_t*)(uint32_t)mboot->framebuffer_addr;
    fb_info.width = mboot->framebuffer_width;
    fb_info.height = mboot->framebuffer_height;
    fb_info.pitch = mboot->framebuffer_pitch;
    fb_info.bpp = mboot->framebuffer_bpp;
    fb_info.size = fb_info.pitch * fb_info.height;
    
    /* Set initial draw target to framebuffer */
    draw_target = fb_info.address;
    
    return true;
}

/**
 * Get framebuffer info
 */
framebuffer_info_t* fb_get_info(void) {
    return &fb_info;
}

/**
 * Get framebuffer address
 */
uint32_t* fb_get_address(void) {
    return fb_info.address;
}

/**
 * Get screen width
 */
uint32_t fb_get_width(void) {
    return fb_info.width;
}

/**
 * Get screen height
 */
uint32_t fb_get_height(void) {
    return fb_info.height;
}

/**
 * Put a pixel at (x, y) with specified color
 */
void fb_put_pixel(int32_t x, int32_t y, uint32_t color) {
    /* Bounds check */
    if (x < 0 || x >= (int32_t)fb_info.width || 
        y < 0 || y >= (int32_t)fb_info.height) {
        return;
    }
    
    /* Calculate offset and write pixel */
    uint32_t offset = y * (fb_info.pitch / 4) + x;
    draw_target[offset] = color;
}

/**
 * Get pixel color at (x, y)
 */
uint32_t fb_get_pixel(int32_t x, int32_t y) {
    /* Bounds check */
    if (x < 0 || x >= (int32_t)fb_info.width || 
        y < 0 || y >= (int32_t)fb_info.height) {
        return 0;
    }
    
    uint32_t offset = y * (fb_info.pitch / 4) + x;
    return draw_target[offset];
}

/**
 * Clear screen with specified color
 */
void fb_clear(uint32_t color) {
    uint32_t pixels = (fb_info.pitch / 4) * fb_info.height;
    memsetl(draw_target, color, pixels);
}

/**
 * Copy rectangular region
 */
void fb_copy_rect(int32_t src_x, int32_t src_y, 
                  int32_t dst_x, int32_t dst_y,
                  uint32_t width, uint32_t height) {
    uint32_t pitch_pixels = fb_info.pitch / 4;
    
    /* Determine copy direction to handle overlapping regions */
    if (dst_y < src_y || (dst_y == src_y && dst_x < src_x)) {
        /* Copy forward */
        for (uint32_t y = 0; y < height; y++) {
            uint32_t* src = &draw_target[(src_y + y) * pitch_pixels + src_x];
            uint32_t* dst = &draw_target[(dst_y + y) * pitch_pixels + dst_x];
            memcpy(dst, src, width * 4);
        }
    } else {
        /* Copy backward */
        for (int32_t y = height - 1; y >= 0; y--) {
            uint32_t* src = &draw_target[(src_y + y) * pitch_pixels + src_x];
            uint32_t* dst = &draw_target[(dst_y + y) * pitch_pixels + dst_x];
            memmove(dst, src, width * 4);
        }
    }
}

/**
 * Swap buffers (for double buffering)
 */
void fb_swap_buffers(void) {
    if (double_buffering && back_buffer != NULL) {
        /* Copy back buffer to front buffer */
        memcpy(fb_info.address, back_buffer, fb_info.size);
    }
}

/**
 * Get the back buffer address
 */
uint32_t* fb_get_back_buffer(void) {
    return back_buffer;
}

/**
 * Enable/disable double buffering
 */
void fb_set_double_buffer(bool enable) {
    if (enable && back_buffer == NULL) {
        /* Allocate back buffer (use static allocation for now) */
        /* In a real OS, this would use pmm_alloc_pages() */
        static uint32_t static_back_buffer[1024 * 768] __attribute__((aligned(4096)));
        back_buffer = static_back_buffer;
        draw_target = back_buffer;
        double_buffering = true;
    } else if (!enable) {
        draw_target = fb_info.address;
        double_buffering = false;
    }
}

/**
 * Check if double buffering is enabled
 */
bool fb_is_double_buffered(void) {
    return double_buffering;
}
