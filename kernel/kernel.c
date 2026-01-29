/**
 * TarkOS v1.5 - High-Performance Edition (FPS++ Edition)
 * Features: Dirty Rendering, Zero-Lag Input, High-Speed UI
 */

/* ============= HEADERS & TYPES ============= */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef int bool;
#define true 1
#define false 0
#define NULL ((void *)0)

/* ============= VGA DRIVER & OPTIMIZED UI ============= */
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)VGA_ADDR;
static uint16_t cur_x = 0, cur_y = 0;
static uint8_t cur_col = 0x1F;

void set_color(uint8_t fg, uint8_t bg) { cur_col = (bg << 4) | (fg & 0x0F); }

// Optimized memory copy for VGA
static inline void vga_copy(uint16_t *dest, uint16_t *src, uint32_t count) {
  while (count--)
    *dest++ = *src++;
}

void put_char_raw(char c, uint8_t col, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
    uint16_t entry = (uint16_t)c | ((uint16_t)col << 8);
    if (vga[y * VGA_WIDTH + x] != entry)
      vga[y * VGA_WIDTH + x] = entry; // Dirty check
  }
}

void update_cursor(int x, int y) {
  uint16_t pos = y * VGA_WIDTH + x;
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
  __asm__ volatile("outb %0, %1"
                   :
                   : "a"((uint8_t)(pos & 0xFF)), "Nd"((uint16_t)0x3D5));
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
  __asm__ volatile("outb %0, %1"
                   :
                   : "a"((uint8_t)((pos >> 8) & 0xFF)), "Nd"((uint16_t)0x3D5));
}

void clear_screen() {
  uint16_t blank = (cur_col << 8) | ' ';
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = blank;
  cur_x = 0;
  cur_y = 0;
  update_cursor(0, 0);
}

void scroll() {
  vga_copy(vga + VGA_WIDTH, vga + 2 * VGA_WIDTH, (VGA_HEIGHT - 3) * VGA_WIDTH);
  uint16_t blank = (cur_col << 8) | ' ';
  for (int i = (VGA_HEIGHT - 2) * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH;
       i++)
    vga[i] = blank;
  cur_y = VGA_HEIGHT - 2;
}

void put_char(char c) {
  if (c == '\n') {
    cur_x = 0;
    cur_y++;
  } else if (c == '\b') {
    if (cur_x > 0)
      cur_x--;
    put_char_raw(' ', cur_col, cur_x, cur_y);
  } else {
    put_char_raw(c, cur_col, cur_x, cur_y);
    cur_x++;
  }
  if (cur_x >= VGA_WIDTH) {
    cur_x = 0;
    cur_y++;
  }
  if (cur_y >= VGA_HEIGHT - 1)
    scroll();
  update_cursor(cur_x, cur_y);
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}

void print_at(int x, int y, const char *s, uint8_t col) {
  int ix = x;
  while (*s) {
    put_char_raw(*s++, col, ix++, y);
  }
}

void draw_rect(int x, int y, int w, int h, uint8_t col) {
  for (int i = y; i < y + h; i++)
    for (int j = x; j < x + w; j++)
      put_char_raw(' ', col, j, i);
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
  unsigned char *p = (unsigned char *)s;
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

/* ============= I/O & TIME ============= */
static inline uint8_t inb(uint16_t port) {
  uint8_t r;
  __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
  return r;
}
static inline void outb(uint16_t port, uint8_t v) {
  __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(port));
}

uint8_t get_rtc(int reg) {
  outb(0x70, reg);
  return inb(0x71);
}
void get_time_str(char *buf) {
  uint8_t h = get_rtc(0x04), m = get_rtc(0x02), s = get_rtc(0x00);
  h = ((h & 0xF0) >> 1) + ((h & 0xF0) >> 3) + (h & 0x0F);
  m = ((m & 0xF0) >> 1) + ((m & 0xF0) >> 3) + (m & 0x0F);
  s = ((s & 0xF0) >> 1) + ((s & 0xF0) >> 3) + (s & 0x0F);
  h = (h + 3) % 24;
  buf[0] = h / 10 + '0';
  buf[1] = h % 10 + '0';
  buf[2] = ':';
  buf[3] = m / 10 + '0';
  buf[4] = m % 10 + '0';
  buf[5] = ':';
  buf[6] = s / 10 + '0';
  buf[7] = s % 10 + '0';
  buf[8] = 0;
}

/* ============= KEYBOARD (LAG-FREE) ============= */
static uint8_t last_sc = 0;
char get_scancode() {
  if (!(inb(0x64) & 1))
    return 0;
  uint8_t sc = inb(0x60);
  if (sc == last_sc)
    return 0; // State-aware (ignore held keys for shell)
  last_sc = sc;
  return (sc & 0x80) ? 0 : sc; // Only return make-codes
}

char get_any_scancode() { // For games/raw use
  if (!(inb(0x64) & 1))
    return 0;
  return inb(0x60);
}

char scancode_to_char(uint8_t sc, bool shift) {
  static const char map[] = {
      0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
      '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
      'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
      'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
      'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};
  static const char caps[] = {
      0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
      '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
      'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
      'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' '};
  if (sc >= 58)
    return 0;
  return shift ? caps[sc] : map[sc];
}

/* ============= TrEdit v2.1 (Optimized) ============= */
#define EDIT_BUF 4096
static char edit_buffer[EDIT_BUF];
static int edit_cursor = 0;

void render_editor_incremental(const char *fname, int l, int c) {
  // Only redraw metadata and cursor row area
  draw_rect(0, 0, 80, 1, 0x70);
  print_at(2, 0, "TrEdit High-FPS | File: ", 0x70);
  print_at(26, 0, fname, 0x70);
  print_at(60, 0, "F10:Exit ESC:Quit", 0x70);

  draw_rect(0, 24, 80, 1, 0x70);
  char buf[16];
  print_at(2, 24, "L: ", 0x70);
  itoa(l, buf);
  print_at(5, 24, buf, 0x70);
  print_at(12, 24, "C: ", 0x70);
  itoa(c, buf);
  print_at(15, 24, buf, 0x70);

  // Efficient full buffer render (only if modified - simplified for now)
  int row = 2, col = 2;
  for (int i = 0; i < edit_cursor; i++) {
    if (edit_buffer[i] == '\n') {
      row++;
      col = 2;
    } else {
      put_char_raw(edit_buffer[i], 0x1F, col++, row);
    }
    if (col >= 78) {
      row++;
      col = 2;
    }
  }
  update_cursor(col, row);
}

void tredit(const char *filename) {
  clear_screen();
  memset(edit_buffer, 0, EDIT_BUF);
  edit_cursor = 0;
  bool shift = false, run = true;
  int line = 1, col = 1;

  while (run) {
    render_editor_incremental(filename, line, col);
    while (1) {
      uint8_t sc = get_any_scancode();
      if (!sc)
        continue;

      // Handle Shift
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;

      if (sc & 0x80)
        continue; // Ignore other break codes

      if (sc == 0x44 || sc == 0x01) {
        run = false;
        break;
      }

      char ch = scancode_to_char(sc, shift);
      if (ch) {
        if (ch == '\n') {
          edit_buffer[edit_cursor++] = '\n';
          line++;
          col = 1;
        } else if (ch == '\b') {
          if (edit_cursor > 0) {
            if (edit_buffer[--edit_cursor] == '\n')
              line--;
            edit_buffer[edit_cursor] = 0;
            col--;
          }
        } else if (edit_cursor < EDIT_BUF - 1) {
          edit_buffer[edit_cursor++] = ch;
          col++;
        }
        break; // Trigger redraw
      }
      // Minimal release wait
      while (get_any_scancode() == sc)
        ;
    }
  }
  clear_screen();
}

/* ============= SHELL & FAST UI ============= */
void draw_header_only() {
  char tb[10];
  get_time_str(tb);
  print_at(71, 0, tb, 0x3F); // Incremental clock update
}

void draw_ui_full() {
  draw_rect(0, 0, 80, 1, 0x3F);
  print_at(2, 0, "TarkOS v1.5 High-Perf", 0x3F);
  draw_header_only();
  draw_rect(0, 24, 80, 1, 0x70);
  print_at(2, 24, "FPS Optimized | Commands: ls, cat, tredit, cls, ver, info",
           0x70);
}

void shell_loop() {
  char line[64];
  int pos = 0;
  bool shift = false;
  clear_screen();
  draw_ui_full();

  while (1) {
    set_color(10, 1);
    print("\nroot@tarkos");
    set_color(15, 1);
    print(":# ");
    pos = 0;
    memset(line, 0, 64);
    while (1) {
      uint8_t sc = get_any_scancode();

      // Ultra-fast UI refresh (No blocking)
      static uint32_t ticks = 0;
      if (++ticks > 50000) {
        draw_header_only();
        ticks = 0;
      }

      if (!sc)
        continue;

      // Shift tracking
      if (sc == 0x2A || sc == 0x36) {
        shift = true;
        continue;
      }
      if (sc == 0xAA || sc == 0xB6) {
        shift = false;
        continue;
      }

      if (sc & 0x80)
        continue; // Break-code

      char c = scancode_to_char(sc, shift);
      if (c == '\n') {
        put_char('\n');
        break;
      } else if (c == '\b') {
        if (pos > 0) {
          pos--;
          put_char('\b');
        }
      } else if (c && pos < 63) {
        line[pos++] = c;
        put_char(c);
      }

      // Non-blocking release check
      while (get_any_scancode() == sc) {
        if (++ticks > 50000) {
          draw_header_only();
          ticks = 0;
        }
      }
    }
    if (pos == 0)
      continue;
    line[pos] = 0;

    if (strcmp(line, "help") == 0)
      print("Commands: tredit, ls, cls, ver, info, reboot\n");
    else if (strcmp(line, "ls") == 0)
      print("bin  readme.txt  kernel.bin\n");
    else if (strcmp(line, "cls") == 0) {
      draw_rect(0, 1, 80, 23, 0x1F);
      cur_x = 0;
      cur_y = 1;
    } else if (strncmp(line, "tredit", 6) == 0)
      tredit("buffer.txt");
    else if (strcmp(line, "ver") == 0)
      print("TarkOS v1.5.0-HighPerf\n");
    else if (strcmp(line, "reboot") == 0)
      outb(0x64, 0xFE);
    else {
      print("Unknown command.\n");
    }
  }
}

/* ============= KERNEL MAIN ============= */
void kmain() {
  set_color(15, 1);
  clear_screen();
  draw_ui_full();
  cur_x = 0;
  cur_y = 1;
  shell_loop();
}
