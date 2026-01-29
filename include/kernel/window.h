/**
 * TarkOS - Window Manager
 * Desktop compositor with double buffering
 */

#ifndef _KERNEL_WINDOW_H
#define _KERNEL_WINDOW_H

#include <kernel/types.h>

/* Window manager constants */
#define WM_MAX_WINDOWS      16
#define WM_TITLEBAR_HEIGHT  24
#define WM_BORDER_SIZE      2
#define WM_BUTTON_SIZE      18

/* Window flags */
#define WINDOW_VISIBLE      (1 << 0)
#define WINDOW_FOCUSED      (1 << 1)
#define WINDOW_MOVABLE      (1 << 2)
#define WINDOW_RESIZABLE    (1 << 3)
#define WINDOW_CLOSABLE     (1 << 4)
#define WINDOW_MINIMIZABLE  (1 << 5)
#define WINDOW_MAXIMIZABLE  (1 << 6)

/* Default window flags */
#define WINDOW_DEFAULT      (WINDOW_VISIBLE | WINDOW_MOVABLE | WINDOW_CLOSABLE)

/* Window structure */
typedef struct window {
    int32_t x, y;               /* Position (top-left of frame) */
    uint32_t width, height;     /* Content area size */
    char title[64];             /* Window title */
    uint32_t flags;             /* Window flags */
    uint32_t bg_color;          /* Background color */
    int32_t z_order;            /* Z-order (higher = on top) */
    
    /* Drag state */
    bool is_dragging;
    int32_t drag_offset_x;
    int32_t drag_offset_y;
} window_t;

/**
 * Initialize the window manager
 */
void wm_init(void);

/**
 * Create a new window
 * @param x X position
 * @param y Y position  
 * @param width Content width
 * @param height Content height
 * @param title Window title
 * @param flags Window flags
 * @return Window handle (index), or -1 on failure
 */
int wm_create_window(int32_t x, int32_t y, uint32_t width, uint32_t height,
                     const char* title, uint32_t flags);

/**
 * Destroy a window
 */
void wm_destroy_window(int handle);

/**
 * Get window by handle
 */
window_t* wm_get_window(int handle);

/**
 * Set window position
 */
void wm_set_position(int handle, int32_t x, int32_t y);

/**
 * Set window size
 */
void wm_set_size(int handle, uint32_t width, uint32_t height);

/**
 * Set window title
 */
void wm_set_title(int handle, const char* title);

/**
 * Show/hide window
 */
void wm_set_visible(int handle, bool visible);

/**
 * Focus a window (bring to front)
 */
void wm_focus_window(int handle);

/**
 * Get currently focused window
 */
int wm_get_focused_window(void);

/**
 * Find window at screen position
 * @return Window handle or -1 if none
 */
int wm_window_at(int32_t x, int32_t y);

/**
 * Check if point is in window's titlebar
 */
bool wm_point_in_titlebar(int handle, int32_t x, int32_t y);

/**
 * Check if point is in window's close button
 */
bool wm_point_in_close_button(int handle, int32_t x, int32_t y);

/**
 * Process mouse input (called by main loop)
 */
void wm_process_mouse(int32_t x, int32_t y, bool left_btn, bool right_btn);

/**
 * Compose and render all windows
 * Call this each frame to update the display
 */
void wm_compose(void);

/**
 * Set desktop background color
 */
void wm_set_desktop_color(uint32_t color);

/**
 * Draw text in a window's client area
 * @param handle Window handle
 * @param x X position relative to client area
 * @param y Y position relative to client area
 * @param text Text to draw
 * @param color Text color
 */
void wm_draw_text(int handle, int32_t x, int32_t y, const char* text, uint32_t color);

/**
 * Fill rectangle in window's client area
 */
void wm_fill_rect(int handle, int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);

#endif /* _KERNEL_WINDOW_H */
