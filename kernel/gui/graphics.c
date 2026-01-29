/**
 * TarkOS - Graphics Library Implementation
 * Drawing primitives for GUI rendering
 */

#include <kernel/graphics.h>
#include <kernel/framebuffer.h>
#include <lib/string.h>

/* Cursor dimensions */
#define CURSOR_WIDTH  12
#define CURSOR_HEIGHT 19

/* Saved cursor background */
static uint32_t cursor_bg[CURSOR_WIDTH * CURSOR_HEIGHT];
static int32_t cursor_bg_x = -1;
static int32_t cursor_bg_y = -1;

/* Arrow cursor bitmap (1 = white, 2 = black, 0 = transparent) */
static const uint8_t cursor_data[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    {2,0,0,0,0,0,0,0,0,0,0,0},
    {2,2,0,0,0,0,0,0,0,0,0,0},
    {2,1,2,0,0,0,0,0,0,0,0,0},
    {2,1,1,2,0,0,0,0,0,0,0,0},
    {2,1,1,1,2,0,0,0,0,0,0,0},
    {2,1,1,1,1,2,0,0,0,0,0,0},
    {2,1,1,1,1,1,2,0,0,0,0,0},
    {2,1,1,1,1,1,1,2,0,0,0,0},
    {2,1,1,1,1,1,1,1,2,0,0,0},
    {2,1,1,1,1,1,1,1,1,2,0,0},
    {2,1,1,1,1,1,1,1,1,1,2,0},
    {2,1,1,1,1,1,1,2,2,2,2,2},
    {2,1,1,1,2,1,1,2,0,0,0,0},
    {2,1,1,2,0,2,1,1,2,0,0,0},
    {2,1,2,0,0,2,1,1,2,0,0,0},
    {2,2,0,0,0,0,2,1,1,2,0,0},
    {2,0,0,0,0,0,2,1,1,2,0,0},
    {0,0,0,0,0,0,0,2,1,1,2,0},
    {0,0,0,0,0,0,0,2,2,2,0,0}
};

/**
 * Helper: Get absolute value
 */
static inline int32_t abs(int32_t x) {
    return (x < 0) ? -x : x;
}

/**
 * Draw a filled rectangle
 */
void gfx_fill_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color) {
    framebuffer_info_t* fb = fb_get_info();
    uint32_t pitch_pixels = fb->pitch / 4;
    
    /* Clip to screen bounds */
    int32_t x1 = (x < 0) ? 0 : x;
    int32_t y1 = (y < 0) ? 0 : y;
    int32_t x2 = x + width;
    int32_t y2 = y + height;
    
    if (x2 > (int32_t)fb->width) x2 = fb->width;
    if (y2 > (int32_t)fb->height) y2 = fb->height;
    
    if (x1 >= x2 || y1 >= y2) return;
    
    /* Get draw target */
    uint32_t* target = fb_is_double_buffered() ? fb_get_back_buffer() : fb_get_address();
    
    /* Fill rectangle */
    uint32_t rect_width = x2 - x1;
    for (int32_t py = y1; py < y2; py++) {
        uint32_t* row = &target[py * pitch_pixels + x1];
        memsetl(row, color, rect_width);
    }
}

/**
 * Draw a rectangle outline
 */
void gfx_draw_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color) {
    gfx_draw_hline(x, y, width, color);                     /* Top */
    gfx_draw_hline(x, y + height - 1, width, color);        /* Bottom */
    gfx_draw_vline(x, y, height, color);                    /* Left */
    gfx_draw_vline(x + width - 1, y, height, color);        /* Right */
}

/**
 * Draw a horizontal line
 */
void gfx_draw_hline(int32_t x, int32_t y, uint32_t length, uint32_t color) {
    framebuffer_info_t* fb = fb_get_info();
    
    if (y < 0 || y >= (int32_t)fb->height) return;
    
    int32_t x1 = (x < 0) ? 0 : x;
    int32_t x2 = x + length;
    if (x2 > (int32_t)fb->width) x2 = fb->width;
    
    if (x1 >= x2) return;
    
    uint32_t* target = fb_is_double_buffered() ? fb_get_back_buffer() : fb_get_address();
    uint32_t pitch_pixels = fb->pitch / 4;
    
    uint32_t* row = &target[y * pitch_pixels + x1];
    memsetl(row, color, x2 - x1);
}

/**
 * Draw a vertical line
 */
void gfx_draw_vline(int32_t x, int32_t y, uint32_t length, uint32_t color) {
    framebuffer_info_t* fb = fb_get_info();
    
    if (x < 0 || x >= (int32_t)fb->width) return;
    
    int32_t y1 = (y < 0) ? 0 : y;
    int32_t y2 = y + length;
    if (y2 > (int32_t)fb->height) y2 = fb->height;
    
    if (y1 >= y2) return;
    
    uint32_t* target = fb_is_double_buffered() ? fb_get_back_buffer() : fb_get_address();
    uint32_t pitch_pixels = fb->pitch / 4;
    
    for (int32_t py = y1; py < y2; py++) {
        target[py * pitch_pixels + x] = color;
    }
}

/**
 * Draw a line between two points (Bresenham's algorithm)
 */
void gfx_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
    int32_t dx = abs(x2 - x1);
    int32_t dy = abs(y2 - y1);
    int32_t sx = (x1 < x2) ? 1 : -1;
    int32_t sy = (y1 < y2) ? 1 : -1;
    int32_t err = dx - dy;
    
    while (1) {
        fb_put_pixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int32_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/**
 * Draw a circle outline (midpoint circle algorithm)
 */
void gfx_draw_circle(int32_t cx, int32_t cy, int32_t radius, uint32_t color) {
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;
    
    while (x >= y) {
        fb_put_pixel(cx + x, cy + y, color);
        fb_put_pixel(cx + y, cy + x, color);
        fb_put_pixel(cx - y, cy + x, color);
        fb_put_pixel(cx - x, cy + y, color);
        fb_put_pixel(cx - x, cy - y, color);
        fb_put_pixel(cx - y, cy - x, color);
        fb_put_pixel(cx + y, cy - x, color);
        fb_put_pixel(cx + x, cy - y, color);
        
        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

/**
 * Draw a filled circle
 */
void gfx_fill_circle(int32_t cx, int32_t cy, int32_t radius, uint32_t color) {
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;
    
    while (x >= y) {
        gfx_draw_hline(cx - x, cy + y, 2 * x + 1, color);
        gfx_draw_hline(cx - x, cy - y, 2 * x + 1, color);
        gfx_draw_hline(cx - y, cy + x, 2 * y + 1, color);
        gfx_draw_hline(cx - y, cy - x, 2 * y + 1, color);
        
        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

/**
 * Interpolate between two colors
 */
static uint32_t interpolate_color(uint32_t c1, uint32_t c2, uint32_t t, uint32_t max) {
    uint8_t r1 = (c1 >> 16) & 0xFF;
    uint8_t g1 = (c1 >> 8) & 0xFF;
    uint8_t b1 = c1 & 0xFF;
    
    uint8_t r2 = (c2 >> 16) & 0xFF;
    uint8_t g2 = (c2 >> 8) & 0xFF;
    uint8_t b2 = c2 & 0xFF;
    
    uint8_t r = r1 + ((r2 - r1) * t) / max;
    uint8_t g = g1 + ((g2 - g1) * t) / max;
    uint8_t b = b1 + ((b2 - b1) * t) / max;
    
    return RGB(r, g, b);
}

/**
 * Draw a gradient rectangle (vertical gradient)
 */
void gfx_fill_rect_gradient(int32_t x, int32_t y, uint32_t width, uint32_t height,
                            uint32_t color_top, uint32_t color_bottom) {
    for (uint32_t py = 0; py < height; py++) {
        uint32_t color = interpolate_color(color_top, color_bottom, py, height);
        gfx_draw_hline(x, y + py, width, color);
    }
}

/**
 * Draw a 3D-style raised panel
 */
void gfx_draw_panel_raised(int32_t x, int32_t y, uint32_t width, uint32_t height,
                           uint32_t bg_color) {
    /* Fill background */
    gfx_fill_rect(x, y, width, height, bg_color);
    
    /* Light edges (top-left) */
    gfx_draw_hline(x, y, width, COLOR_WHITE);
    gfx_draw_vline(x, y, height, COLOR_WHITE);
    
    /* Dark edges (bottom-right) */
    gfx_draw_hline(x, y + height - 1, width, COLOR_DARK_GRAY);
    gfx_draw_vline(x + width - 1, y, height, COLOR_DARK_GRAY);
}

/**
 * Draw a 3D-style sunken panel
 */
void gfx_draw_panel_sunken(int32_t x, int32_t y, uint32_t width, uint32_t height,
                           uint32_t bg_color) {
    /* Fill background */
    gfx_fill_rect(x, y, width, height, bg_color);
    
    /* Dark edges (top-left) */
    gfx_draw_hline(x, y, width, COLOR_DARK_GRAY);
    gfx_draw_vline(x, y, height, COLOR_DARK_GRAY);
    
    /* Light edges (bottom-right) */
    gfx_draw_hline(x, y + height - 1, width, COLOR_WHITE);
    gfx_draw_vline(x + width - 1, y, height, COLOR_WHITE);
}

/**
 * Draw a bitmap image
 */
void gfx_draw_bitmap(int32_t x, int32_t y, uint32_t width, uint32_t height, 
                     const uint32_t* data) {
    for (uint32_t py = 0; py < height; py++) {
        for (uint32_t px = 0; px < width; px++) {
            fb_put_pixel(x + px, y + py, data[py * width + px]);
        }
    }
}

/**
 * Alpha blend two colors
 */
uint32_t gfx_blend_colors(uint32_t fg, uint32_t bg, uint8_t alpha) {
    uint8_t fg_r = (fg >> 16) & 0xFF;
    uint8_t fg_g = (fg >> 8) & 0xFF;
    uint8_t fg_b = fg & 0xFF;
    
    uint8_t bg_r = (bg >> 16) & 0xFF;
    uint8_t bg_g = (bg >> 8) & 0xFF;
    uint8_t bg_b = bg & 0xFF;
    
    uint8_t r = ((fg_r * alpha) + (bg_r * (255 - alpha))) / 255;
    uint8_t g = ((fg_g * alpha) + (bg_g * (255 - alpha))) / 255;
    uint8_t b = ((fg_b * alpha) + (bg_b * (255 - alpha))) / 255;
    
    return RGB(r, g, b);
}

/**
 * Draw a bitmap with transparency (alpha blending)
 */
void gfx_draw_bitmap_alpha(int32_t x, int32_t y, uint32_t width, uint32_t height,
                           const uint32_t* data) {
    for (uint32_t py = 0; py < height; py++) {
        for (uint32_t px = 0; px < width; px++) {
            uint32_t pixel = data[py * width + px];
            uint8_t alpha = (pixel >> 24) & 0xFF;
            
            if (alpha == 255) {
                fb_put_pixel(x + px, y + py, pixel);
            } else if (alpha > 0) {
                uint32_t bg = fb_get_pixel(x + px, y + py);
                uint32_t blended = gfx_blend_colors(pixel, bg, alpha);
                fb_put_pixel(x + px, y + py, blended);
            }
        }
    }
}

/**
 * Save region under cursor
 */
void gfx_save_cursor_bg(int32_t x, int32_t y) {
    cursor_bg_x = x;
    cursor_bg_y = y;
    
    for (int32_t py = 0; py < CURSOR_HEIGHT; py++) {
        for (int32_t px = 0; px < CURSOR_WIDTH; px++) {
            cursor_bg[py * CURSOR_WIDTH + px] = fb_get_pixel(x + px, y + py);
        }
    }
}

/**
 * Restore region under cursor
 */
void gfx_restore_cursor_bg(int32_t x, int32_t y) {
    if (cursor_bg_x == x && cursor_bg_y == y) {
        for (int32_t py = 0; py < CURSOR_HEIGHT; py++) {
            for (int32_t px = 0; px < CURSOR_WIDTH; px++) {
                fb_put_pixel(x + px, y + py, cursor_bg[py * CURSOR_WIDTH + px]);
            }
        }
    }
}

/**
 * Draw mouse cursor
 */
void gfx_draw_cursor(int32_t x, int32_t y) {
    for (int32_t py = 0; py < CURSOR_HEIGHT; py++) {
        for (int32_t px = 0; px < CURSOR_WIDTH; px++) {
            uint8_t pixel = cursor_data[py][px];
            if (pixel == 1) {
                fb_put_pixel(x + px, y + py, COLOR_WHITE);
            } else if (pixel == 2) {
                fb_put_pixel(x + px, y + py, COLOR_BLACK);
            }
        }
    }
}
