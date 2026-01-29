/**
 * TarkOS - PS/2 Mouse Driver Implementation
 * 3-byte packet mouse handling
 */

#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/mouse.h>
#include <kernel/ports.h>

/* Mouse state */
static mouse_state_t mouse_state = {0, 0, 0, 0, false, false, false};

/* Screen bounds */
static uint32_t max_x = 1024;
static uint32_t max_y = 768;

/* Packet collection state */
static uint8_t mouse_cycle = 0;
static uint8_t mouse_packet[3];

/* State change flag */
static volatile bool state_changed = false;

/**
 * Wait for mouse controller to be ready for write
 */
static void mouse_wait_write(void) {
  int timeout = 100000;
  while (timeout-- > 0) {
    if ((inb(MOUSE_STATUS_PORT) & 0x02) == 0) {
      return;
    }
  }
}

/**
 * Wait for mouse data to be available
 */
static void mouse_wait_read(void) {
  int timeout = 100000;
  while (timeout-- > 0) {
    if ((inb(MOUSE_STATUS_PORT) & 0x01) != 0) {
      return;
    }
  }
}

/**
 * Write command to mouse (via 8042 controller)
 */
static void mouse_write(uint8_t data) {
  mouse_wait_write();
  outb(MOUSE_COMMAND_PORT, MOUSE_CMD_WRITE);
  mouse_wait_write();
  outb(MOUSE_DATA_PORT, data);
}

/**
 * Read data from mouse
 */
static uint8_t mouse_read(void) {
  mouse_wait_read();
  return inb(MOUSE_DATA_PORT);
}

/**
 * Mouse interrupt handler (IRQ12)
 */
static void mouse_handler(registers_t *regs) {
  (void)regs;

  /* Read data byte */
  uint8_t data = inb(MOUSE_DATA_PORT);

  /* Collect packet bytes */
  mouse_packet[mouse_cycle++] = data;

  /* Check if we have a complete packet */
  if (mouse_cycle == 3) {
    mouse_cycle = 0;

    /* Validate packet (bit 3 should always be set) */
    if (!(mouse_packet[0] & MOUSE_ALWAYS_1)) {
      /* Invalid packet, resync */
      return;
    }

    /* Skip if overflow */
    if (mouse_packet[0] & (MOUSE_X_OVERFLOW | MOUSE_Y_OVERFLOW)) {
      return;
    }

    /* Extract button states */
    mouse_state.left_button = (mouse_packet[0] & MOUSE_LEFT_BTN) != 0;
    mouse_state.right_button = (mouse_packet[0] & MOUSE_RIGHT_BTN) != 0;
    mouse_state.middle_button = (mouse_packet[0] & MOUSE_MIDDLE_BTN) != 0;

    /* Extract movement deltas */
    int32_t dx = mouse_packet[1];
    int32_t dy = mouse_packet[2];

    /* Apply sign extension */
    if (mouse_packet[0] & MOUSE_X_SIGN) {
      dx |= 0xFFFFFF00;
    }
    if (mouse_packet[0] & MOUSE_Y_SIGN) {
      dy |= 0xFFFFFF00;
    }

    /* Store deltas */
    mouse_state.dx = dx;
    mouse_state.dy = -dy; /* Invert Y for screen coordinates */

    /* Update position with bounds checking */
    mouse_state.x += dx;
    mouse_state.y -= dy; /* Invert Y for screen coordinates */

    /* Clamp to screen bounds */
    if (mouse_state.x < 0)
      mouse_state.x = 0;
    if (mouse_state.y < 0)
      mouse_state.y = 0;
    if (mouse_state.x >= (int32_t)max_x)
      mouse_state.x = max_x - 1;
    if (mouse_state.y >= (int32_t)max_y)
      mouse_state.y = max_y - 1;

    /* Mark state as changed */
    state_changed = true;
  }
}

/**
 * Initialize the mouse driver
 */
void mouse_init(uint32_t screen_width, uint32_t screen_height) {
  /* Set screen bounds */
  max_x = screen_width;
  max_y = screen_height;

  /* Center mouse initially */
  mouse_state.x = screen_width / 2;
  mouse_state.y = screen_height / 2;

  /* Enable auxiliary device (mouse) */
  mouse_wait_write();
  outb(MOUSE_COMMAND_PORT, MOUSE_CMD_ENABLE);

  /* Get current command byte */
  mouse_wait_write();
  outb(MOUSE_COMMAND_PORT, MOUSE_CMD_STATUS);
  uint8_t status = mouse_read();

  /* Enable IRQ12 and disable mouse clock inhibit */
  status |= 0x02;  /* Enable IRQ12 */
  status &= ~0x20; /* Enable mouse clock */

  /* Write new command byte */
  mouse_wait_write();
  outb(MOUSE_COMMAND_PORT, MOUSE_CMD_SET);
  mouse_wait_write();
  outb(MOUSE_DATA_PORT, status);

  /* Set default settings */
  mouse_write(MOUSE_SET_DEFAULTS);
  mouse_read(); /* ACK */

  /* Enable data reporting */
  mouse_write(MOUSE_ENABLE_DATA);
  mouse_read(); /* ACK */

  /* Register interrupt handler */
  register_interrupt_handler(IRQ12, mouse_handler);
}

/**
 * Get current mouse state
 */
mouse_state_t mouse_get_state(void) { return mouse_state; }

/**
 * Get current mouse X position
 */
int32_t mouse_get_x(void) { return mouse_state.x; }

/**
 * Get current mouse Y position
 */
int32_t mouse_get_y(void) { return mouse_state.y; }

/**
 * Check if left button is pressed
 */
bool mouse_left_button(void) { return mouse_state.left_button; }

/**
 * Check if right button is pressed
 */
bool mouse_right_button(void) { return mouse_state.right_button; }

/**
 * Check if middle button is pressed
 */
bool mouse_middle_button(void) { return mouse_state.middle_button; }

/**
 * Set screen bounds for cursor
 */
void mouse_set_bounds(uint32_t width, uint32_t height) {
  max_x = width;
  max_y = height;

  /* Clamp current position to new bounds */
  if (mouse_state.x >= (int32_t)max_x)
    mouse_state.x = max_x - 1;
  if (mouse_state.y >= (int32_t)max_y)
    mouse_state.y = max_y - 1;
}

/**
 * Set mouse position directly
 */
void mouse_set_position(int32_t x, int32_t y) {
  mouse_state.x = x;
  mouse_state.y = y;

  /* Clamp to bounds */
  if (mouse_state.x < 0)
    mouse_state.x = 0;
  if (mouse_state.y < 0)
    mouse_state.y = 0;
  if (mouse_state.x >= (int32_t)max_x)
    mouse_state.x = max_x - 1;
  if (mouse_state.y >= (int32_t)max_y)
    mouse_state.y = max_y - 1;

  state_changed = true;
}

/**
 * Check if mouse state has changed since last check
 */
bool mouse_state_changed(void) { return state_changed; }

/**
 * Clear mouse state changed flag
 */
void mouse_clear_changed(void) { state_changed = false; }
