/**
 * TarkOS - Window Manager Implementation
 * Desktop compositor with double buffering
 */

#include <kernel/window.h>
#include <kernel/framebuffer.h>
#include <kernel/graphics.h>
#include <kernel/font.h>
#include <kernel/mouse.h>
#include <lib/string.h>

/* Window array */
static window_t windows[WM_MAX_WINDOWS];
static bool window_used[WM_MAX_WINDOWS];
static int focused_window = -1;
static int top_z_order = 0;

/* Desktop color */
static uint32_t desktop_color = COLOR_DESKTOP_BG;

/* Previous mouse state for click detection */
static bool prev_left_btn = false;
static int32_t prev_mouse_x = 0;
static int32_t prev_mouse_y = 0;

/* Colors for window decorations */
#define COLOR_TITLEBAR_ACTIVE       RGB(0, 120, 215)
#define COLOR_TITLEBAR_INACTIVE     RGB(128, 128, 128)
#define COLOR_TITLEBAR_TEXT         RGB(255, 255, 255)
#define COLOR_WINDOW_BORDER         RGB(100, 100, 100)
#define COLOR_WINDOW_CLIENT         RGB(240, 240, 240)
#define COLOR_CLOSE_BUTTON          RGB(232, 17, 35)
#define COLOR_CLOSE_BUTTON_HOVER    RGB(241, 112, 122)

/**
 * Initialize the window manager
 */
void wm_init(void) {
    /* Initialize all window slots as unused */
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        window_used[i] = false;
        memset(&windows[i], 0, sizeof(window_t));
    }
    
    focused_window = -1;
    top_z_order = 0;
    
    /* Enable double buffering */
    fb_set_double_buffer(true);
}

/**
 * Create a new window
 */
int wm_create_window(int32_t x, int32_t y, uint32_t width, uint32_t height,
                     const char* title, uint32_t flags) {
    /* Find free slot */
    int slot = -1;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (!window_used[i]) {
            slot = i;
            break;
        }
    }
    
    if (slot < 0) {
        return -1;  /* No free slots */
    }
    
    /* Initialize window */
    window_t* win = &windows[slot];
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->flags = flags;
    win->bg_color = COLOR_WINDOW_CLIENT;
    win->z_order = ++top_z_order;
    win->is_dragging = false;
    
    /* Copy title */
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
    
    window_used[slot] = true;
    
    /* Focus new window */
    wm_focus_window(slot);
    
    return slot;
}

/**
 * Destroy a window
 */
void wm_destroy_window(int handle) {
    if (handle < 0 || handle >= WM_MAX_WINDOWS || !window_used[handle]) {
        return;
    }
    
    window_used[handle] = false;
    
    /* If this was focused window, find another to focus */
    if (focused_window == handle) {
        focused_window = -1;
        int highest_z = -1;
        for (int i = 0; i < WM_MAX_WINDOWS; i++) {
            if (window_used[i] && (windows[i].flags & WINDOW_VISIBLE)) {
                if (windows[i].z_order > highest_z) {
                    highest_z = windows[i].z_order;
                    focused_window = i;
                }
            }
        }
    }
}

/**
 * Get window by handle
 */
window_t* wm_get_window(int handle) {
    if (handle < 0 || handle >= WM_MAX_WINDOWS || !window_used[handle]) {
        return NULL;
    }
    return &windows[handle];
}

/**
 * Set window position
 */
void wm_set_position(int handle, int32_t x, int32_t y) {
    window_t* win = wm_get_window(handle);
    if (win) {
        win->x = x;
        win->y = y;
    }
}

/**
 * Set window size
 */
void wm_set_size(int handle, uint32_t width, uint32_t height) {
    window_t* win = wm_get_window(handle);
    if (win) {
        win->width = width;
        win->height = height;
    }
}

/**
 * Set window title
 */
void wm_set_title(int handle, const char* title) {
    window_t* win = wm_get_window(handle);
    if (win) {
        strncpy(win->title, title, sizeof(win->title) - 1);
        win->title[sizeof(win->title) - 1] = '\0';
    }
}

/**
 * Show/hide window
 */
void wm_set_visible(int handle, bool visible) {
    window_t* win = wm_get_window(handle);
    if (win) {
        if (visible) {
            win->flags |= WINDOW_VISIBLE;
        } else {
            win->flags &= ~WINDOW_VISIBLE;
        }
    }
}

/**
 * Focus a window (bring to front)
 */
void wm_focus_window(int handle) {
    if (handle < 0 || handle >= WM_MAX_WINDOWS || !window_used[handle]) {
        return;
    }
    
    /* Remove focus from current window */
    if (focused_window >= 0 && focused_window < WM_MAX_WINDOWS) {
        windows[focused_window].flags &= ~WINDOW_FOCUSED;
    }
    
    /* Set new focus */
    focused_window = handle;
    windows[handle].flags |= WINDOW_FOCUSED;
    windows[handle].z_order = ++top_z_order;
}

/**
 * Get currently focused window
 */
int wm_get_focused_window(void) {
    return focused_window;
}

/**
 * Check if point is inside window (including frame)
 */
static bool point_in_window(window_t* win, int32_t x, int32_t y) {
    int32_t frame_x = win->x;
    int32_t frame_y = win->y;
    int32_t frame_w = win->width + WM_BORDER_SIZE * 2;
    int32_t frame_h = win->height + WM_TITLEBAR_HEIGHT + WM_BORDER_SIZE;
    
    return (x >= frame_x && x < frame_x + frame_w &&
            y >= frame_y && y < frame_y + frame_h);
}

/**
 * Find window at screen position
 */
int wm_window_at(int32_t x, int32_t y) {
    int found = -1;
    int highest_z = -1;
    
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (window_used[i] && (windows[i].flags & WINDOW_VISIBLE)) {
            if (point_in_window(&windows[i], x, y)) {
                if (windows[i].z_order > highest_z) {
                    highest_z = windows[i].z_order;
                    found = i;
                }
            }
        }
    }
    
    return found;
}

/**
 * Check if point is in window's titlebar
 */
bool wm_point_in_titlebar(int handle, int32_t x, int32_t y) {
    window_t* win = wm_get_window(handle);
    if (!win) return false;
    
    int32_t tb_x = win->x + WM_BORDER_SIZE;
    int32_t tb_y = win->y;
    int32_t tb_w = win->width;
    int32_t tb_h = WM_TITLEBAR_HEIGHT;
    
    return (x >= tb_x && x < tb_x + tb_w &&
            y >= tb_y && y < tb_y + tb_h);
}

/**
 * Check if point is in window's close button
 */
bool wm_point_in_close_button(int handle, int32_t x, int32_t y) {
    window_t* win = wm_get_window(handle);
    if (!win) return false;
    if (!(win->flags & WINDOW_CLOSABLE)) return false;
    
    int32_t btn_x = win->x + WM_BORDER_SIZE + win->width - WM_BUTTON_SIZE - 4;
    int32_t btn_y = win->y + (WM_TITLEBAR_HEIGHT - WM_BUTTON_SIZE) / 2;
    
    return (x >= btn_x && x < btn_x + WM_BUTTON_SIZE &&
            y >= btn_y && y < btn_y + WM_BUTTON_SIZE);
}

/**
 * Process mouse input
 */
void wm_process_mouse(int32_t x, int32_t y, bool left_btn, bool right_btn) {
    (void)right_btn;
    
    bool left_pressed = left_btn && !prev_left_btn;
    
    /* Handle window dragging */
    if (focused_window >= 0 && windows[focused_window].is_dragging) {
        if (left_btn) {
            /* Continue dragging */
            windows[focused_window].x = x - windows[focused_window].drag_offset_x;
            windows[focused_window].y = y - windows[focused_window].drag_offset_y;
            
            /* Keep window on screen */
            if (windows[focused_window].y < 0) {
                windows[focused_window].y = 0;
            }
        } else {
            /* Stop dragging */
            windows[focused_window].is_dragging = false;
        }
    } else if (left_pressed) {
        /* Check for window click */
        int clicked = wm_window_at(x, y);
        
        if (clicked >= 0) {
            /* Focus the clicked window */
            wm_focus_window(clicked);
            
            /* Check for close button click */
            if (wm_point_in_close_button(clicked, x, y)) {
                wm_destroy_window(clicked);
            }
            /* Check for titlebar click (start drag) */
            else if (wm_point_in_titlebar(clicked, x, y)) {
                if (windows[clicked].flags & WINDOW_MOVABLE) {
                    windows[clicked].is_dragging = true;
                    windows[clicked].drag_offset_x = x - windows[clicked].x;
                    windows[clicked].drag_offset_y = y - windows[clicked].y;
                }
            }
        }
    }
    
    prev_left_btn = left_btn;
    prev_mouse_x = x;
    prev_mouse_y = y;
}

/**
 * Draw a single window
 */
static void draw_window(window_t* win, bool is_focused) {
    int32_t frame_x = win->x;
    int32_t frame_y = win->y;
    uint32_t frame_w = win->width + WM_BORDER_SIZE * 2;
    uint32_t frame_h = win->height + WM_TITLEBAR_HEIGHT + WM_BORDER_SIZE;
    
    /* Draw shadow (subtle) */
    gfx_fill_rect(frame_x + 3, frame_y + 3, frame_w, frame_h, RGB(0, 0, 0));
    
    /* Draw border */
    gfx_fill_rect(frame_x, frame_y, frame_w, frame_h, COLOR_WINDOW_BORDER);
    
    /* Draw titlebar */
    uint32_t titlebar_color = is_focused ? COLOR_TITLEBAR_ACTIVE : COLOR_TITLEBAR_INACTIVE;
    gfx_fill_rect(frame_x + WM_BORDER_SIZE, frame_y, 
                  win->width, WM_TITLEBAR_HEIGHT, titlebar_color);
    
    /* Draw title text (centered vertically) */
    int32_t text_y = frame_y + (WM_TITLEBAR_HEIGHT - font_get_height()) / 2;
    font_draw_string_transparent(frame_x + WM_BORDER_SIZE + 8, text_y, 
                                  win->title, COLOR_TITLEBAR_TEXT);
    
    /* Draw close button if window is closable */
    if (win->flags & WINDOW_CLOSABLE) {
        int32_t btn_x = frame_x + WM_BORDER_SIZE + win->width - WM_BUTTON_SIZE - 4;
        int32_t btn_y = frame_y + (WM_TITLEBAR_HEIGHT - WM_BUTTON_SIZE) / 2;
        
        /* Check if mouse is over close button */
        bool hover = (prev_mouse_x >= btn_x && prev_mouse_x < btn_x + WM_BUTTON_SIZE &&
                      prev_mouse_y >= btn_y && prev_mouse_y < btn_y + WM_BUTTON_SIZE);
        
        uint32_t btn_color = hover ? COLOR_CLOSE_BUTTON_HOVER : COLOR_CLOSE_BUTTON;
        gfx_fill_rect(btn_x, btn_y, WM_BUTTON_SIZE, WM_BUTTON_SIZE, btn_color);
        
        /* Draw X on close button */
        uint32_t x_color = COLOR_WHITE;
        int32_t cx = btn_x + WM_BUTTON_SIZE / 2;
        int32_t cy = btn_y + WM_BUTTON_SIZE / 2;
        gfx_draw_line(cx - 4, cy - 4, cx + 4, cy + 4, x_color);
        gfx_draw_line(cx + 4, cy - 4, cx - 4, cy + 4, x_color);
    }
    
    /* Draw client area */
    gfx_fill_rect(frame_x + WM_BORDER_SIZE, 
                  frame_y + WM_TITLEBAR_HEIGHT,
                  win->width, win->height, win->bg_color);
}

/**
 * Compare windows by z-order for sorting
 */
static void sort_windows_by_z(int* order, int count) {
    /* Simple bubble sort (good enough for small window count) */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (windows[order[j]].z_order > windows[order[j + 1]].z_order) {
                int tmp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = tmp;
            }
        }
    }
}

/**
 * Compose and render all windows
 */
void wm_compose(void) {
    /* Clear with desktop color */
    gfx_fill_rect(0, 0, fb_get_width(), fb_get_height(), desktop_color);
    
    /* Collect visible windows */
    int visible[WM_MAX_WINDOWS];
    int visible_count = 0;
    
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (window_used[i] && (windows[i].flags & WINDOW_VISIBLE)) {
            visible[visible_count++] = i;
        }
    }
    
    /* Sort by z-order (back to front) */
    sort_windows_by_z(visible, visible_count);
    
    /* Draw windows back to front */
    for (int i = 0; i < visible_count; i++) {
        int handle = visible[i];
        draw_window(&windows[handle], handle == focused_window);
    }
    
    /* Draw mouse cursor on top */
    int32_t mx = mouse_get_x();
    int32_t my = mouse_get_y();
    gfx_draw_cursor(mx, my);
    
    /* Swap buffers */
    fb_swap_buffers();
}

/**
 * Set desktop background color
 */
void wm_set_desktop_color(uint32_t color) {
    desktop_color = color;
}

/**
 * Draw text in a window's client area
 */
void wm_draw_text(int handle, int32_t x, int32_t y, const char* text, uint32_t color) {
    window_t* win = wm_get_window(handle);
    if (!win) return;
    
    /* Calculate absolute position */
    int32_t abs_x = win->x + WM_BORDER_SIZE + x;
    int32_t abs_y = win->y + WM_TITLEBAR_HEIGHT + y;
    
    font_draw_string_transparent(abs_x, abs_y, text, color);
}

/**
 * Fill rectangle in window's client area
 */
void wm_fill_rect(int handle, int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color) {
    window_t* win = wm_get_window(handle);
    if (!win) return;
    
    /* Calculate absolute position */
    int32_t abs_x = win->x + WM_BORDER_SIZE + x;
    int32_t abs_y = win->y + WM_TITLEBAR_HEIGHT + y;
    
    gfx_fill_rect(abs_x, abs_y, width, height, color);
}
