/**
 * TarkOS v1.6 - Professional Edition
 * Build: 2026.01.29
 */

/* ============= TYPES & MACROS ============= */
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long size_t;
#define NULL ((void *)0)
#define true 1
#define false 0

typedef struct {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
} multiboot_info_t;

/* ============= HARDWARE I/O ============= */
static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
static inline void io_wait(void) { outb(0x80, 0); }

/* ============= VGA TEXT DRIVER ============= */
#define VGA_MEM ((uint16_t *)0xB8000)
#define VGA_W 80
#define VGA_H 25

enum Color {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GREY = 7,
  DARK_GREY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  YELLOW = 14,
  WHITE = 15
};

static int term_x = 0;
static int term_y = 0;
static uint8_t term_color = 0x07; // Light Grey on Black

void set_color(uint8_t fg, uint8_t bg) { term_color = (bg << 4) | (fg & 0x0F); }

void put_raw(int x, int y, char c, uint8_t color) {
  if (x >= 0 && x < VGA_W && y >= 0 && y < VGA_H) {
    VGA_MEM[y * VGA_W + x] = (uint16_t)c | ((uint16_t)color << 8);
  }
}

uint16_t get_raw(int x, int y) {
  if (x >= 0 && x < VGA_W && y >= 0 && y < VGA_H) {
    return VGA_MEM[y * VGA_W + x];
  }
  return 0;
}

void scroll() {
  for (int i = 0; i < (VGA_H - 1) * VGA_W; i++) {
    VGA_MEM[i] = VGA_MEM[i + VGA_W];
  }
  for (int i = (VGA_H - 1) * VGA_W; i < VGA_H * VGA_W; i++) {
    VGA_MEM[i] = 0x0720;
  }
}

void put_char(char c) {
  if (c == '\n') {
    term_x = 0;
    term_y++;
  } else if (c == '\b') {
    if (term_x > 0) {
      term_x--;
      put_raw(term_x, term_y, ' ', term_color);
    }
  } else if (c == '\r') {
    term_x = 0;
  } else {
    put_raw(term_x, term_y, c, term_color);
    term_x++;
  }

  if (term_x >= VGA_W) {
    term_x = 0;
    term_y++;
  }

  if (term_y >= VGA_H) {
    scroll();
    term_y = VGA_H - 1;
  }

  // Move hardware cursor
  uint16_t pos = term_y * VGA_W + term_x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void print(const char *str) {
  while (*str)
    put_char(*str++);
}

void print_c(const char *str, uint8_t fg) {
  uint8_t old = term_color;
  set_color(fg, old >> 4);
  print(str);
  set_color(old & 0x0F, old >> 4);
}

void clear() {
  for (int i = 0; i < VGA_W * VGA_H; i++)
    VGA_MEM[i] = 0x0720;
  term_x = 0;
  term_y = 0;
}

/* ============= HELPERS ============= */
void *memset(void *s, int c, size_t n) {
  unsigned char *p = (unsigned char *)s;
  while (n--)
    *p++ = (unsigned char)c;
  return s;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strlen(const char *s) {
  int len = 0;
  while (*s++)
    len++;
  return len;
}

/* ============= TIMER & SLEEP (POLLING) ============= */
volatile size_t ticks = 0;

void sleep_ms(int ms) {
  // Crude calibration for QEMU
  // Reduced count to prevent freezing with "numbers" on screen
  volatile uint32_t count = ms * 1000;
  while (count--) {
    __asm__ volatile("nop");
  }
}

/* ============= MOUSE DRIVER (PS/2) ============= */
static int mx = 40, my = 12;
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
static int old_mx = 40, old_my = 12;
static uint16_t under_cursor = 0x0720;

void mouse_wait(uint8_t type) {
  uint32_t timeout = 10000;
  if (type == 0) {
    while (timeout--)
      if ((inb(0x64) & 1))
        return;
  } else {
    while (timeout--)
      if (!(inb(0x64) & 2))
        return;
  }
}

void mouse_write(uint8_t write) {
  mouse_wait(1);
  outb(0x64, 0xD4);
  mouse_wait(1);
  outb(0x60, write);
}

uint8_t mouse_read() {
  mouse_wait(0);
  return inb(0x60);
}

void mouse_init() {
  uint8_t status;

  mouse_wait(1);
  outb(0x64, 0xA8); // Enable packet streaming

  mouse_wait(1);
  outb(0x64, 0x20); // Read Config
  mouse_wait(0);
  status = (inb(0x60) | 2); // Enable IRQ12

  mouse_wait(1);
  outb(0x64, 0x60); // Write Config
  mouse_wait(1);
  outb(0x60, status);

  mouse_write(0xF6);
  mouse_read(); // Set Defaults
  mouse_write(0xF4);
  mouse_read(); // Enable Data Reporting

  under_cursor = get_raw(mx, my);
}

void update_mouse() {
  // Poll data
  if (inb(0x64) & 1) {
    uint8_t data = inb(0x60);

    switch (mouse_cycle) {
    case 0:
      if ((data & 0x08) == 0x08) {
        mouse_byte[0] = data;
        mouse_cycle++;
      }
      break;
    case 1:
      mouse_byte[1] = data;
      mouse_cycle++;
      break;
    case 2:
      mouse_byte[2] = data;
      mouse_cycle = 0;

      int dx = mouse_byte[1];
      int dy = mouse_byte[2];
      if (mouse_byte[0] & 0x10)
        dx |= 0xFFFFFF00;
      if (mouse_byte[0] & 0x20)
        dy |= 0xFFFFFF00;

      // Restore old cursor pos
      put_raw(old_mx, old_my, (char)(under_cursor & 0xFF),
              (uint8_t)(under_cursor >> 8));

      mx += dx;
      my -= dy; // PS/2 Y is inverted

      // Clamp
      if (mx < 0)
        mx = 0;
      if (mx >= VGA_W)
        mx = VGA_W - 1;
      if (my < 0)
        my = 0;
      if (my >= VGA_H)
        my = VGA_H - 1;

      // Save new under_cursor
      under_cursor = get_raw(mx, my);
      old_mx = mx;
      old_my = my;

      // Draw cursor (Invert color logic: Light Grey -> Green for visibility)
      put_raw(mx, my, (char)219, LIGHT_GREEN); // Solid block
      break;
    }
  }
}

/* ============= KEYBOARD DRIVER ============= */
char get_key_map(uint8_t sc) {
  const char map[] = {0,    27,  '1', '2',  '3',  '4',  '5', '6', '7',  '8',
                      '9',  '0', '-', '=',  '\b', '\t', 'q', 'w', 'e',  'r',
                      't',  'y', 'u', 'i',  'o',  'p',  '[', ']', '\n', 0,
                      'a',  's', 'd', 'f',  'g',  'h',  'j', 'k', 'l',  ';',
                      '\'', '`', 0,   '\\', 'z',  'x',  'c', 'v', 'b',  'n',
                      'm',  ',', '.', '/',  0,    '*',  0,   ' '};
  if (sc < sizeof(map))
    return map[sc];
  return 0;
}

char getch() {
  while (1) {
    update_mouse(); // Always update mouse while waiting

    if (inb(0x64) & 1) {
      uint8_t sc = inb(0x60);
      if (!(sc & 0x80)) { // Key Pressed
        char c = get_key_map(sc);
        if (c) {
          // Primitive debounce
          sleep_ms(20);
          return c;
        }
      }
    }
  }
}

/* ============= SYSTEM FEATURES ============= */
void cmd_matrix() {
  clear();
  print_c("Entering The Matrix... (Press any key to stop)\n", GREEN);
  while (1) {
    if (inb(0x64) & 1) {
      inb(0x60);
      break;
    }

    int x = ticks % VGA_W;
    int y = (ticks / VGA_W) % VGA_H;
    put_raw(x, y, (ticks % 2) ? '1' : '0', GREEN);

    ticks = (ticks * 1103515245 + 12345) & 0x7FFFFFFF; // RNG

    // Random columns
    for (int k = 0; k < 5; k++) {
      put_raw(ticks % VGA_W, ticks % VGA_H, (char)((ticks % 26) + 'a'),
              LIGHT_GREEN);
    }

    sleep_ms(5);
  }
  clear();
}

void print_logo() {
  print_c("\n  TarkOS v1.6\n", LIGHT_CYAN);
  print_c("  ===========\n", CYAN);
  print("  OpSys Kernel: LOADED\n");
  print("  Drivers: PS/2 Mouse, VGA, Keyboard... ");
  print_c("[OK]\n", GREEN);
  print("  Memory: 128MB Detected... ");
  print_c("[OK]\n", GREEN);
  print("  Date: 2026-01-29 (Simulated)\n\n");
}

/* ============= KERNEL MAIN ============= */
void kernel_main(uint32_t magic, multiboot_info_t *info) {
  (void)magic;
  (void)info; // Validated in ASM

  // Disable interrupts to use polling
  __asm__ volatile("cli");

  set_color(LIGHT_GREY, BLACK);
  clear();

  // Boot sequence simulation
  print("Booting TarkOS Kernel...\n");
  sleep_ms(200);
  print("> Initializing Memory... OK\n");
  sleep_ms(100);
  print("> Loading Drivers... OK\n");
  sleep_ms(100);

  mouse_init();
  print("> Initializing Mouse... OK\n");
  sleep_ms(200);

  print("> Starting Shell...\n");
  sleep_ms(300);
  clear();

  print_logo();
  print("Type 'help' for commands. Try moving the mouse!\n\n");

  char cmd[64];
  while (1) {
    print_c("root@tarkos", LIGHT_RED);
    print(":");
    print_c("~", LIGHT_BLUE);
    print("$ ");

    int pos = 0;
    for (int i = 0; i < 64; i++)
      cmd[i] = 0;

    // Input Loop
    while (1) {
      char c = getch();
      if (c == '\n') {
        put_char('\n');
        break;
      } else if (c == '\b') {
        if (pos > 0) {
          pos--;
          put_char('\b');
          cmd[pos] = 0;
        }
      } else if (pos < 63) {
        cmd[pos++] = c;
        put_char(c);
      }
    }

    // Command execution
    if (strcmp(cmd, "help") == 0) {
      print("\nCOMMAND LIST:\n");
      print("  clear    - Clear screen\n");
      print("  info     - System info\n");
      print("  matrix   - Matrix rain effect\n");
      print("  shutdown - Halt system\n");
      print("  reboot   - Reboot system\n");
      print("\nTry moving the mouse cursor!\n");
    } else if (strcmp(cmd, "clear") == 0) {
      clear();
      print_logo();
    } else if (strcmp(cmd, "info") == 0) {
      print("\nTarkOS Kernel v1.6\n");
      print("Build: GCC i686-elf\n");
      print("Author: Tarik\n");
    } else if (strcmp(cmd, "matrix") == 0) {
      cmd_matrix();
    } else if (strcmp(cmd, "reboot") == 0) {
      print("Rebooting...\n");
      outb(0x64, 0xFE);
    } else if (strcmp(cmd, "shutdown") == 0) {
      print("System Halted.\n");
      while (1)
        __asm__ volatile("hlt");
    } else if (strlen(cmd) > 0) {
      print("Unknown command: ");
      print(cmd);
      print("\n");
    }
  }
}
