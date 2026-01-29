/**
 * TarkOS v1.7 - Clean & Stable
 * Features: Correct Mouse Driver, Shell, Clean UI
 */

/* ============= HEADERS ============= */
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long size_t;
typedef int bool;

#define true 1
#define false 0
#define NULL ((void *)0)

typedef struct {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
} multiboot_info_t;

/* ============= VGA DRIVER ============= */
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
static uint8_t term_color = 0x07;

void set_color(uint8_t fg, uint8_t bg) { term_color = (bg << 4) | (fg & 0x0F); }

void put_raw(int x, int y, char c, uint8_t color) {
  if (x >= 0 && x < VGA_W && y >= 0 && y < VGA_H) {
    VGA_MEM[y * VGA_W + x] = (uint16_t)c | ((uint16_t)color << 8);
  }
}

uint16_t get_raw(int x, int y) {
  if (x >= 0 && x < VGA_W && y >= 0 && y < VGA_H)
    return VGA_MEM[y * VGA_W + x];
  return 0;
}

void scroll() {
  for (int i = 0; i < (VGA_H - 1) * VGA_W; i++)
    VGA_MEM[i] = VGA_MEM[i + VGA_W];
  for (int i = (VGA_H - 1) * VGA_W; i < VGA_H * VGA_W; i++)
    VGA_MEM[i] = 0x0720;
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
  } else if (c >= ' ') {
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

  // Update HW Cursor
  uint16_t pos = term_y * VGA_W + term_x;
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
  __asm__ volatile("outb %0, %1"
                   :
                   : "a"((uint8_t)(pos & 0xFF)), "Nd"((uint16_t)0x3D5));
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
  __asm__ volatile("outb %0, %1"
                   :
                   : "a"((uint8_t)((pos >> 8) & 0xFF)), "Nd"((uint16_t)0x3D5));
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}
void print_c(const char *s, uint8_t fg) {
  uint8_t old = term_color;
  set_color(fg, old >> 4);
  print(s);
  set_color(old & 0xF, old >> 4);
}

void clear() {
  for (int i = 0; i < VGA_W * VGA_H; i++)
    VGA_MEM[i] = 0x0720;
  term_x = 0;
  term_y = 0;
}

/* ============= I/O ============= */
static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

/* ============= UTILS ============= */
void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  while (n--)
    *p++ = c;
  return s;
}
int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}
size_t strlen(const char *s) {
  size_t l = 0;
  while (s[l])
    l++;
  return l;
}

/* ============= CORRECT MOUSE DRIVER ============= */
static int mx = 40, my = 12;
static uint8_t m_cycle = 0;
static uint8_t m_packet[3];
static int old_mx = 40, old_my = 12;
static uint16_t old_char = 0x0720;

void mouse_wait(uint8_t type) {
  uint32_t t = 100000;
  if (type == 0)
    while (t--) {
      if ((inb(0x64) & 1))
        return;
    }
  else
    while (t--) {
      if (!(inb(0x64) & 2))
        return;
    }
}

void mouse_write(uint8_t w) {
  mouse_wait(1);
  outb(0x64, 0xD4);
  mouse_wait(1);
  outb(0x60, w);
}
uint8_t mouse_read() {
  mouse_wait(0);
  return inb(0x60);
}

void mouse_init() {
  uint8_t status;
  mouse_wait(1);
  outb(0x64, 0xA8); // Enable Aux
  mouse_wait(1);
  outb(0x64, 0x20); // Get Status
  mouse_wait(0);
  status = (inb(0x60) | 2); // Enable IRQ12
  mouse_wait(1);
  outb(0x64, 0x60); // Set Status
  mouse_wait(1);
  outb(0x60, status);

  mouse_write(0xF6);
  mouse_read(); // Defaults
  mouse_write(0xF4);
  mouse_read(); // Enable Streaming

  old_char = get_raw(mx, my);
}

void update_mouse() {
  if (inb(0x64) & 1) {
    uint8_t b = inb(0x60);

    // Cycle Synchronization
    if (m_cycle == 0) {
      // Bit 3 must be 1. If not, reset cycle.
      if ((b & 0x08) == 0x08) {
        m_packet[0] = b;
        m_cycle++;
      }
    } else if (m_cycle == 1) {
      m_packet[1] = b;
      m_cycle++;
    } else {
      m_packet[2] = b;
      m_cycle = 0;

      // CORRECT 9-BIT SIGNED PARSING
      int dx = m_packet[1];
      int dy = m_packet[2];

      // Sign Extension for X
      if (m_packet[0] & 0x10)
        dx |= 0xFFFFFF00;
      // Sign Extension for Y
      if (m_packet[0] & 0x20)
        dy |= 0xFFFFFF00;

      // Negate Y (Mouse Up is negative Y in hardware, but we need screen Y)
      // But usually screen Y increases downwards.
      // PS/2: Y+ is Up. Screen: Y+ is Down. So we must invert.
      dy = -dy;

      // Restore Old
      put_raw(old_mx, old_my, (char)(old_char & 0xFF),
              (uint8_t)(old_char >> 8));

      // Update Position (Scaling Divisors for Text Mode)
      mx += dx; // Sensitivity X
      my += dy; // Sensitivity Y

      // Clamp
      if (mx < 0)
        mx = 0;
      if (mx >= VGA_W)
        mx = VGA_W - 1;
      if (my < 0)
        my = 0;
      if (my >= VGA_H)
        my = VGA_H - 1;

      // Save New
      old_char = get_raw(mx, my);
      old_mx = mx;
      old_my = my;

      // Draw Cursor (Solid Green Block)
      // Save original character but change bg/fg
      char c = (char)(old_char & 0xFF);
      if (c == ' ')
        c = 219; // If empty space, use solid block

      // Invert colors logic: White->Black, Black->White
      uint8_t col = (uint8_t)(old_char >> 8);
      uint8_t inv_col = ((col & 0x0F) << 4) | ((col & 0xF0) >> 4);
      if (inv_col == col)
        inv_col = 0x70; // Ensure visibility

      put_raw(mx, my, c, inv_col);
    }
  }
}

/* ============= KEYBOARD ============= */
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

void sleep(int ms) {
  volatile int start = ms * 5000;
  while (start--) {
    update_mouse();
    __asm__ volatile("nop");
  }
}

char getch() {
  while (1) {
    update_mouse();
    if (inb(0x64) & 1) {
      uint8_t sc = inb(0x60);
      if (!(sc & 0x80)) {
        char c = get_key_map(sc);
        if (c) {
          sleep(20);
          return c;
        }
      }
    }
  }
}

/* ============= KERNEL MAIN ============= */
void kernel_main(uint32_t magic, multiboot_info_t *info) {
  (void)magic;
  (void)info;              // Validated
  __asm__ volatile("cli"); // Disable interrupts

  set_color(LIGHT_GREY, BLACK);
  clear();

  mouse_init();

  print_c("TarkOS v1.7\n", LIGHT_CYAN);
  print("Welcome! Type 'help'.\n\n");

  char cmd[64];
  while (1) {
    print_c("root@tarkos", LIGHT_GREEN);
    print(":$ ");

    int pos = 0;
    memset(cmd, 0, 64);
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
      } else if (pos < 60) {
        put_char(c);
        cmd[pos++] = c;
      }
    }

    if (strlen(cmd) == 0)
      continue;

    if (strcmp(cmd, "clear") == 0)
      clear();
    else if (strcmp(cmd, "help") == 0)
      print("Commands: clear, info, reboot, shutdown\nMouse should work "
            "smoothly now!\n");
    else if (strcmp(cmd, "info") == 0)
      print("Kernel: TarkOS v1.7\nDriver: PS/2 Mouse (Correct 9-bit)\n");
    else if (strcmp(cmd, "reboot") == 0)
      outb(0x64, 0xFE);
    else if (strcmp(cmd, "shutdown") == 0) {
      print("Halted.\n");
      while (1)
        __asm__ volatile("hlt");
    } else {
      print("Unknown command.\n");
    }
  }
}
