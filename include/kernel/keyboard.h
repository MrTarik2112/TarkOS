/**
 * TarkOS - PS/2 Keyboard Driver
 * Scancode set 1 keyboard handling
 */

#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

#include <kernel/types.h>

/* Keyboard I/O ports */
#define KB_DATA_PORT        0x60
#define KB_STATUS_PORT      0x64
#define KB_COMMAND_PORT     0x64

/* Keyboard status flags */
#define KB_STATUS_OUTPUT    0x01    /* Output buffer full */
#define KB_STATUS_INPUT     0x02    /* Input buffer full */

/* Special keys */
#define KEY_ESCAPE          0x01
#define KEY_BACKSPACE       0x0E
#define KEY_TAB             0x0F
#define KEY_ENTER           0x1C
#define KEY_CTRL            0x1D
#define KEY_LSHIFT          0x2A
#define KEY_RSHIFT          0x36
#define KEY_ALT             0x38
#define KEY_CAPSLOCK        0x3A
#define KEY_F1              0x3B
#define KEY_F2              0x3C
#define KEY_F3              0x3D
#define KEY_F4              0x3E
#define KEY_F5              0x3F
#define KEY_F6              0x40
#define KEY_F7              0x41
#define KEY_F8              0x42
#define KEY_F9              0x43
#define KEY_F10             0x44
#define KEY_F11             0x57
#define KEY_F12             0x58
#define KEY_NUMLOCK         0x45
#define KEY_SCROLLLOCK      0x46

/* Key release flag */
#define KEY_RELEASE         0x80

/* Keyboard event structure */
typedef struct {
    uint8_t scancode;       /* Raw scancode */
    char    ascii;          /* ASCII character (0 if special key) */
    bool    pressed;        /* true = pressed, false = released */
    bool    shift;          /* Shift key state */
    bool    ctrl;           /* Control key state */
    bool    alt;            /* Alt key state */
} key_event_t;

/* Key buffer size */
#define KEY_BUFFER_SIZE     256

/**
 * Initialize the keyboard driver
 */
void keyboard_init(void);

/**
 * Check if a key is available in the buffer
 */
bool keyboard_has_key(void);

/**
 * Get the next key event from the buffer
 * Returns false if buffer is empty
 */
bool keyboard_get_event(key_event_t* event);

/**
 * Get the next ASCII character from the buffer
 * Returns 0 if no character available
 */
char keyboard_getchar(void);

/**
 * Get current modifier key states
 */
bool keyboard_is_shift_pressed(void);
bool keyboard_is_ctrl_pressed(void);
bool keyboard_is_alt_pressed(void);
bool keyboard_is_capslock_on(void);

#endif /* _KERNEL_KEYBOARD_H */
