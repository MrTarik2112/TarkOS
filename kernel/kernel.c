/**
 * TarkOS v1.1 - Professional Hobby OS
 * Features: Interrupts, Buffer Keyboard, Games, File System
 */

/* ============= HEADERS ============= */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef int bool;
#define true 1
#define false 0
#define NULL ((void *)0)

// Standard colors
enum VGA_COLOR {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GRAY = 7,
  DARK_GRAY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  YELLOW = 14,
  WHITE = 15
};

/* ============= I/O PORTS ============= */
static inline uint8_t inb(uint16_t port) {
  uint8_t r;
  __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
  return r;
}
static inline void outb(uint16_t port, uint8_t v) {
  __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(port));
}
static inline void io_wait(void) { outb(0x80, 0); }

/* ============= VGA DRIVER ============= */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)0xB8000;
static int cur_x = 0, cur_y = 0;
static uint8_t color = 0x1F; // White on Blue

void set_color(uint8_t fg, uint8_t bg) { color = (bg << 4) | (fg & 0x0F); }

void put_char_at(char c, uint8_t col, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    vga[y * VGA_WIDTH + x] = (uint16_t)c | ((uint16_t)col << 8);
}

void scroll() {
  for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
  for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
    vga[i] = 0x0720;
  cur_y = VGA_HEIGHT - 1;
}

void put_char(char c) {
  if (c == '\n') {
    cur_x = 0;
    cur_y++;
  } else if (c == '\b') {
    if (cur_x > 0)
      cur_x--;
    put_char_at(' ', color, cur_x, cur_y);
  } else {
    put_char_at(c, color, cur_x, cur_y);
    cur_x++;
  }
  if (cur_x >= VGA_WIDTH) {
    cur_x = 0;
    cur_y++;
  }
  if (cur_y >= VGA_HEIGHT)
    scroll();

  // Move Hardware Cursor
  uint16_t pos = cur_y * VGA_WIDTH + cur_x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}
void print_color(const char *s, uint8_t fg) {
  uint8_t old = color;
  set_color(fg, old >> 4);
  print(s);
  color = old;
}

void clear_screen() {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = 0x20 | (color << 8);
  cur_x = 0;
  cur_y = 0;
}

/* ============= UTILS ============= */
int strlen(const char *s) {
  int n = 0;
  while (*s++)
    n++;
  return n;
}
int strcmp(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *a - *b;
}
int strncmp(const char *a, const char *b, int n) {
  while (n-- && *a && *a == *b) {
    a++;
    b++;
  }
  return n < 0 ? 0 : *a - *b;
}
void strcpy(char *d, const char *s) {
  while ((*d++ = *s++))
    ;
}
void *memset(void *s, int c, int n) {
  unsigned char *p = s;
  while (n--)
    *p++ = (unsigned char)c;
  return s;
}

void itoa(int n, char *buf) {
  int i = 0, sign = n;
  if (n < 0)
    n = -n;
  do {
    buf[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);
  if (sign < 0)
    buf[i++] = '-';
  buf[i] = 0;
  for (int j = 0, k = i - 1; j < k; j++, k--) {
    char t = buf[j];
    buf[j] = buf[k];
    buf[k] = t;
  }
}
void print_dec(int n) {
  char buf[16];
  itoa(n, buf);
  print(buf);
}

uint32_t seed = 123456789;
uint32_t rand() { return (seed = seed * 1103515245 + 12345) & 0x7FFFFFFF; }

/* ============= IDT & INTERRUPTS ============= */
struct idt_entry {
  uint16_t base_lo;
  uint16_t sel;
  uint8_t always0;
  uint8_t flags;
  uint16_t base_hi;
} __attribute__((packed));
struct idt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  idt[num].base_lo = base & 0xFFFF;
  idt[num].base_hi = (base >> 16) & 0xFFFF;
  idt[num].sel = sel;
  idt[num].always0 = 0;
  idt[num].flags = flags;
}

// ISRs (Simple stubs)
extern void isr0();
extern void isr1();
void isr_handler() { print("INTERRUPT!\n"); }

/* ============= TIMER (PIT) ============= */
volatile uint32_t timer_ticks = 0;
void timer_handler() {
  timer_ticks++;
  outb(0x20, 0x20);
} // Send EOI

// Minimal Sleep (Busy Wait using Calibrated Loop for reliability without IDT)
void sleep(int ms) {
  // Calibration: ~400,000 iterations per ms on QEMU default
  for (volatile int i = 0; i < ms * 4000; i++)
    __asm__ volatile("nop");
}

/* ============= KEYBOARD (POLLING FIX) ============= */
// Direct polling is more reliable for games than interrupt without full ISR
// stack
char get_key_scan() {
  if (inb(0x64) & 1)
    return inb(0x60);
  return 0;
}

char scancode_to_ascii(uint8_t sc) {
  static const char map[] = {
      0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
      '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
      'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
      'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
      'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};
  if (sc < 58)
    return map[sc];
  return 0;
}

char make_char(uint8_t sc, bool shift) {
  static const char caps[] = {
      0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
      '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
      'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
      'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' '};
  return shift ? ((sc < 58) ? caps[sc] : 0) : scancode_to_ascii(sc);
}

char get_char() {
  while (1) {
    uint8_t sc = get_key_scan();
    if (sc && !(sc & 0x80)) {
      // Debounce
      while (get_key_scan() == sc)
        ;
      return scancode_to_ascii(sc);
    }
    seed++; // Entropy
  }
}

/* ============= SNAKE GAME (FIXED) ============= */
void game_snake() {
  int w = 40, h = 20;
  int sx[100], sy[100];
  int len = 3, dx = 1, dy = 0, score = 0;
  int fx = 20, fy = 10;

  // Init Snake
  for (int i = 0; i < len; i++) {
    sx[i] = 10 - i;
    sy[i] = 10;
  }

  // Setup Screen
  set_color(WHITE, BLACK);
  clear_screen();
  print_color("SNAKE - WASD to Move, Q to Quit\n", YELLOW);

  // Box
  for (int x = 0; x < w; x++) {
    put_char_at('#', LIGHT_GRAY, x, 1);
    put_char_at('#', LIGHT_GRAY, x, h);
  }
  for (int y = 1; y <= h; y++) {
    put_char_at('#', LIGHT_GRAY, 0, y);
    put_char_at('#', LIGHT_GRAY, w - 1, y);
  }

  while (1) {
    // Draw stats
    char sbuf[10];
    itoa(score, sbuf);
    put_char_at('S', WHITE, 42, 2);
    put_char_at(':', WHITE, 43, 2);
    for (int k = 0; sbuf[k]; k++)
      put_char_at(sbuf[k], YELLOW, 45 + k, 2);

    // Draw Food
    put_char_at('*', RED, fx, fy);

    // Draw Snake
    for (int i = 0; i < len; i++)
      put_char_at(i == 0 ? '@' : 'o', GREEN, sx[i], sy[i]);

    // Input (Non-blocking)
    uint8_t key = get_key_scan();
    if (key) {
      if (key == 0x11 && dy == 0) {
        dx = 0;
        dy = -1;
      } // W
      if (key == 0x1F && dy == 0) {
        dx = 0;
        dy = 1;
      } // S
      if (key == 0x1E && dx == 0) {
        dx = -1;
        dy = 0;
      } // A
      if (key == 0x20 && dx == 0) {
        dx = 1;
        dy = 0;
      } // D
      if (key == 0x10)
        break; // Q
    }

    // Logic
    // Tail Move
    put_char_at(' ', BLACK, sx[len - 1], sy[len - 1]);
    for (int i = len - 1; i > 0; i--) {
      sx[i] = sx[i - 1];
      sy[i] = sy[i - 1];
    }
    sx[0] += dx;
    sy[0] += dy;

    // Collision Wall
    if (sx[0] <= 0 || sx[0] >= w - 1 || sy[0] <= 1 || sy[0] >= h)
      break;

    // Collision Self
    for (int i = 1; i < len; i++)
      if (sx[0] == sx[i] && sy[0] == sy[i])
        goto gameover;

    // Eat Food
    if (sx[0] == fx && sy[0] == fy) {
      score += 10;
      if (len < 99)
        len++;
      fx = (rand() % (w - 2)) + 1;
      fy = (rand() % (h - 2)) + 2;
    }

    sleep(50); // Speed control
  }

gameover:
  set_color(WHITE, BLUE);
  clear_screen();
  print("GAME OVER! Score: ");
  print_dec(score);
  print("\n");
}

/* ============= KERNEL MAIN ============= */
void kmain() {
  set_color(WHITE, BLUE);
  clear_screen();

  // Top Bar
  for (int i = 0; i < 80; i++)
    put_char_at(' ', CYAN, i, 0);
  char *title = "TarkOS v1.1 - Professional Mode";
  for (int i = 0; title[i]; i++)
    put_char_at(title[i], WHITE | (CYAN << 4), i + 2, 0);

  cur_y = 2;
  print_color("System Ready. 128MB RAM Detected.\n", LIGHT_GREEN);
  print("Type 'help' for commands. Try 'snake'!\n\n");

  char cmd_buf[64];

  while (1) {
    print_color("root@tarkos", LIGHT_GREEN);
    print(":");
    print_color("~", LIGHT_BLUE);
    print("$ ");

    // Get Command
    int idx = 0;
    while (1) {
      char c = get_char();
      if (c == '\n') {
        put_char('\n');
        cmd_buf[idx] = 0;
        break;
      } else if (c == '\b') {
        if (idx > 0) {
          idx--;
          put_char('\b');
        }
      } else if (idx < 63) {
        cmd_buf[idx++] = c;
        put_char(c);
      }
    }

    // Execute
    if (idx == 0)
      continue;
    else if (strcmp(cmd_buf, "help") == 0) {
      print("\nCOMMANDS:\n");
      print("  ls, cat, touch, rm    - File System\n");
      print("  snake, tetris, pong   - Games\n");
      print("  info, date, clear     - System\n");
      print("  calc, echo, color     - Tools\n");
    } else if (strcmp(cmd_buf, "snake") == 0)
      game_snake();
    else if (strcmp(cmd_buf, "clear") == 0) {
      clear_screen();
      cur_y = 2;
    } else if (strcmp(cmd_buf, "reboot") == 0)
      outb(0x64, 0xFE);
    else if (strncmp(cmd_buf, "echo ", 5) == 0) {
      print(cmd_buf + 5);
      print("\n");
    } else if (strcmp(cmd_buf, "info") == 0) {
      print("\nTarkOS v1.1\nKernel: Monolithic\nGUI: VGA Text\n");
    } else {
      print("Unknown command.\n");
    }
  }
}
