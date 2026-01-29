/**
 * TarkOS - Bitmap Font Rendering
 * 8x16 pixel bitmap font for text display
 */

#ifndef _KERNEL_FONT_H
#define _KERNEL_FONT_H

#include <kernel/types.h>

/* Font dimensions */
#define FONT_WIDTH      8
#define FONT_HEIGHT     16
#define FONT_CHARS      256

/**
 * Draw a character at specified position
 * @param x X position (pixels)
 * @param y Y position (pixels)
 * @param c Character to draw
 * @param fg Foreground color
 * @param bg Background color (use 0 for transparent)
 */
void font_draw_char(int32_t x, int32_t y, char c, uint32_t fg, uint32_t bg);

/**
 * Draw a character with transparent background
 */
void font_draw_char_transparent(int32_t x, int32_t y, char c, uint32_t fg);

/**
 * Draw a string at specified position
 * @param x X position (pixels)
 * @param y Y position (pixels)
 * @param str String to draw
 * @param fg Foreground color
 * @param bg Background color
 */
void font_draw_string(int32_t x, int32_t y, const char* str, uint32_t fg, uint32_t bg);

/**
 * Draw a string with transparent background
 */
void font_draw_string_transparent(int32_t x, int32_t y, const char* str, uint32_t fg);

/**
 * Get string width in pixels
 */
uint32_t font_get_string_width(const char* str);

/**
 * Get font height
 */
uint32_t font_get_height(void);

/**
 * Get font width
 */
uint32_t font_get_width(void);

#endif /* _KERNEL_FONT_H */
