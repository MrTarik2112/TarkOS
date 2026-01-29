/**
 * TarkOS - PS/2 Keyboard Driver Implementation
 * Scancode set 1 keyboard handling
 */

#include <kernel/keyboard.h>
#include <kernel/ports.h>
#include <kernel/idt.h>
#include <kernel/isr.h>

/* Modifier key states */
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool capslock_on = false;

/* Key event buffer (ring buffer) */
static key_event_t key_buffer[KEY_BUFFER_SIZE];
static volatile uint32_t buffer_head = 0;
static volatile uint32_t buffer_tail = 0;

/* US QWERTY keyboard layout - lowercase */
static const char scancode_to_ascii_lower[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  /* 0x00-0x0E */
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',       /* 0x0F-0x1C */
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',            /* 0x1D-0x29 */
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,              /* 0x2A-0x36 */
    '*',  0,   ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        /* 0x37-0x46 */
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',              /* 0x47-0x53 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                               /* 0x54-0x63 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                               /* 0x64-0x73 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                            /* 0x74-0x7F */
};

/* US QWERTY keyboard layout - uppercase/shifted */
static const char scancode_to_ascii_upper[128] = {
    0,    27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  /* 0x00-0x0E */
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',       /* 0x0F-0x1C */
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',             /* 0x1D-0x29 */
    0,    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,               /* 0x2A-0x36 */
    '*',  0,   ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        /* 0x37-0x46 */
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',              /* 0x47-0x53 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                               /* 0x54-0x63 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                               /* 0x64-0x73 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                            /* 0x74-0x7F */
};

/**
 * Add key event to buffer
 */
static void buffer_push(key_event_t* event) {
    uint32_t next = (buffer_head + 1) % KEY_BUFFER_SIZE;
    
    /* Don't overwrite if buffer is full */
    if (next != buffer_tail) {
        key_buffer[buffer_head] = *event;
        buffer_head = next;
    }
}

/**
 * Pop key event from buffer
 */
static bool buffer_pop(key_event_t* event) {
    if (buffer_tail == buffer_head) {
        return false;  /* Buffer empty */
    }
    
    *event = key_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEY_BUFFER_SIZE;
    return true;
}

/**
 * Keyboard interrupt handler (IRQ1)
 */
static void keyboard_handler(registers_t* regs) {
    (void)regs;
    
    /* Read scancode from data port */
    uint8_t scancode = inb(KB_DATA_PORT);
    
    /* Check if key release */
    bool released = (scancode & KEY_RELEASE) != 0;
    uint8_t key = scancode & ~KEY_RELEASE;
    
    /* Handle modifier keys */
    switch (key) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            shift_pressed = !released;
            return;
        
        case KEY_CTRL:
            ctrl_pressed = !released;
            return;
        
        case KEY_ALT:
            alt_pressed = !released;
            return;
        
        case KEY_CAPSLOCK:
            if (!released) {
                capslock_on = !capslock_on;
            }
            return;
    }
    
    /* Create key event */
    key_event_t event;
    event.scancode = key;
    event.pressed = !released;
    event.shift = shift_pressed;
    event.ctrl = ctrl_pressed;
    event.alt = alt_pressed;
    
    /* Determine ASCII character */
    bool uppercase = shift_pressed ^ capslock_on;
    if (uppercase) {
        event.ascii = scancode_to_ascii_upper[key];
    } else {
        event.ascii = scancode_to_ascii_lower[key];
    }
    
    /* For non-letter keys, shift determines the character directly */
    if (event.ascii >= 'A' && event.ascii <= 'Z' && !shift_pressed && capslock_on) {
        /* Capslock only affects letters, not symbols */
    } else if (event.ascii >= 'a' && event.ascii <= 'z' && shift_pressed && capslock_on) {
        /* Shift + capslock = lowercase */
        event.ascii = scancode_to_ascii_lower[key];
    }
    
    /* Add to buffer */
    buffer_push(&event);
}

/**
 * Initialize the keyboard driver
 */
void keyboard_init(void) {
    /* Register interrupt handler */
    register_interrupt_handler(IRQ1, keyboard_handler);
    
    /* Clear any pending data */
    while (inb(KB_STATUS_PORT) & KB_STATUS_OUTPUT) {
        inb(KB_DATA_PORT);
    }
}

/**
 * Check if a key is available in the buffer
 */
bool keyboard_has_key(void) {
    return buffer_tail != buffer_head;
}

/**
 * Get the next key event from the buffer
 */
bool keyboard_get_event(key_event_t* event) {
    return buffer_pop(event);
}

/**
 * Get the next ASCII character from the buffer
 */
char keyboard_getchar(void) {
    key_event_t event;
    
    while (true) {
        if (buffer_pop(&event)) {
            /* Only return on key press with valid ASCII */
            if (event.pressed && event.ascii != 0) {
                return event.ascii;
            }
        } else {
            /* No key available - could wait with HLT or return 0 */
            return 0;
        }
    }
}

/**
 * Get current modifier key states
 */
bool keyboard_is_shift_pressed(void) {
    return shift_pressed;
}

bool keyboard_is_ctrl_pressed(void) {
    return ctrl_pressed;
}

bool keyboard_is_alt_pressed(void) {
    return alt_pressed;
}

bool keyboard_is_capslock_on(void) {
    return capslock_on;
}
