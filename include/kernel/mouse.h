/**
 * TarkOS - PS/2 Mouse Driver
 * Mouse packet handling and cursor tracking
 */

#ifndef _KERNEL_MOUSE_H
#define _KERNEL_MOUSE_H

#include <kernel/types.h>

/* Mouse I/O ports (same as keyboard - 8042 controller) */
#define MOUSE_DATA_PORT     0x60
#define MOUSE_STATUS_PORT   0x64
#define MOUSE_COMMAND_PORT  0x64

/* 8042 controller commands */
#define MOUSE_CMD_WRITE     0xD4    /* Write to mouse */
#define MOUSE_CMD_ENABLE    0xA8    /* Enable auxiliary device */
#define MOUSE_CMD_DISABLE   0xA7    /* Disable auxiliary device */
#define MOUSE_CMD_STATUS    0x20    /* Read command byte */
#define MOUSE_CMD_SET       0x60    /* Write command byte */

/* Mouse commands */
#define MOUSE_SET_DEFAULTS  0xF6    /* Set default settings */
#define MOUSE_ENABLE_DATA   0xF4    /* Enable data reporting */
#define MOUSE_DISABLE_DATA  0xF5    /* Disable data reporting */
#define MOUSE_SET_SAMPLE    0xF3    /* Set sample rate */
#define MOUSE_GET_DEVICE_ID 0xF2    /* Get device ID */
#define MOUSE_RESET         0xFF    /* Reset mouse */

/* Mouse packet flags (byte 0) */
#define MOUSE_LEFT_BTN      0x01    /* Left button pressed */
#define MOUSE_RIGHT_BTN     0x02    /* Right button pressed */
#define MOUSE_MIDDLE_BTN    0x04    /* Middle button pressed */
#define MOUSE_ALWAYS_1      0x08    /* Always set to 1 */
#define MOUSE_X_SIGN        0x10    /* X movement is negative */
#define MOUSE_Y_SIGN        0x20    /* Y movement is negative */
#define MOUSE_X_OVERFLOW    0x40    /* X overflow */
#define MOUSE_Y_OVERFLOW    0x80    /* Y overflow */

/* Mouse state structure */
typedef struct {
    int32_t x;              /* Current X position */
    int32_t y;              /* Current Y position */
    int32_t dx;             /* X movement delta */
    int32_t dy;             /* Y movement delta */
    bool left_button;       /* Left button state */
    bool right_button;      /* Right button state */
    bool middle_button;     /* Middle button state */
} mouse_state_t;

/**
 * Initialize the mouse driver
 * @param screen_width Screen width for cursor bounds
 * @param screen_height Screen height for cursor bounds
 */
void mouse_init(uint32_t screen_width, uint32_t screen_height);

/**
 * Get current mouse state
 */
mouse_state_t mouse_get_state(void);

/**
 * Get current mouse X position
 */
int32_t mouse_get_x(void);

/**
 * Get current mouse Y position
 */
int32_t mouse_get_y(void);

/**
 * Check if left button is pressed
 */
bool mouse_left_button(void);

/**
 * Check if right button is pressed
 */
bool mouse_right_button(void);

/**
 * Check if middle button is pressed
 */
bool mouse_middle_button(void);

/**
 * Set screen bounds for cursor
 */
void mouse_set_bounds(uint32_t width, uint32_t height);

/**
 * Set mouse position directly
 */
void mouse_set_position(int32_t x, int32_t y);

/**
 * Check if mouse state has changed since last check
 */
bool mouse_state_changed(void);

/**
 * Clear mouse state changed flag
 */
void mouse_clear_changed(void);

#endif /* _KERNEL_MOUSE_H */
