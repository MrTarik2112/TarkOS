/**
 * TarkOS - Graphics Library
 * Drawing primitives for GUI rendering
 */

#ifndef _KERNEL_GRAPHICS_H
#define _KERNEL_GRAPHICS_H

#include <kernel/types.h>

/**
 * Draw a filled rectangle
 */
void gfx_fill_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);

/**
 * Draw a rectangle outline
 */
void gfx_draw_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);

/**
 * Draw a horizontal line
 */
void gfx_draw_hline(int32_t x, int32_t y, uint32_t length, uint32_t color);

/**
 * Draw a vertical line
 */
void gfx_draw_vline(int32_t x, int32_t y, uint32_t length, uint32_t color);

/**
 * Draw a line between two points (Bresenham's algorithm)
 */
void gfx_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);

/**
 * Draw a circle outline
 */
void gfx_draw_circle(int32_t cx, int32_t cy, int32_t radius, uint32_t color);

/**
 * Draw a filled circle
 */
void gfx_fill_circle(int32_t cx, int32_t cy, int32_t radius, uint32_t color);

/**
 * Draw a gradient rectangle (vertical gradient)
 */
void gfx_fill_rect_gradient(int32_t x, int32_t y, uint32_t width, uint32_t height,
                            uint32_t color_top, uint32_t color_bottom);

/**
 * Draw a 3D-style raised panel
 */
void gfx_draw_panel_raised(int32_t x, int32_t y, uint32_t width, uint32_t height,
                           uint32_t bg_color);

/**
 * Draw a 3D-style sunken panel
 */
void gfx_draw_panel_sunken(int32_t x, int32_t y, uint32_t width, uint32_t height,
                           uint32_t bg_color);

/**
 * Draw a bitmap image
 * @param x X position
 * @param y Y position
 * @param width Image width
 * @param height Image height
 * @param data Pixel data (32-bit BGRA)
 */
void gfx_draw_bitmap(int32_t x, int32_t y, uint32_t width, uint32_t height, 
                     const uint32_t* data);

/**
 * Draw a bitmap with transparency (alpha blending)
 */
void gfx_draw_bitmap_alpha(int32_t x, int32_t y, uint32_t width, uint32_t height,
                           const uint32_t* data);

/**
 * Alpha blend two colors
 */
uint32_t gfx_blend_colors(uint32_t fg, uint32_t bg, uint8_t alpha);

/**
 * Draw mouse cursor
 */
void gfx_draw_cursor(int32_t x, int32_t y);

/**
 * Save region under cursor
 */
void gfx_save_cursor_bg(int32_t x, int32_t y);

/**
 * Restore region under cursor
 */
void gfx_restore_cursor_bg(int32_t x, int32_t y);

#endif /* _KERNEL_GRAPHICS_H */
