/**
 * TarkOS v1.4.1 - Stability & UI Update
 * Professional UI, TrEdit, and Optimized Boot
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

/* ============= VGA DRIVER & BASIC UI ============= */
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)VGA_ADDR;
static uint16_t cur_x = 0, cur_y = 0;
static uint8_t cur_col = 0x1F;

void set_color(uint8_t fg, uint8_t bg) { cur_col = (bg << 4) | (fg & 0x0F); }

void put_char_raw(char c, uint8_t col, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    vga[y * VGA_WIDTH + x] = (uint16_t)c | ((uint16_t)col << 8);
}

void clear_screen() {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = (cur_col << 8) | ' ';
  cur_x = 0;
  cur_y = 0;
}

void update_cursor() {
  uint16_t pos = cur_y * VGA_WIDTH + cur_x;
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
  __asm__ volatile("outb %0, %1"
                   :
                   : "a"((uint8_t)(pos & 0xFF)), "Nd"((uint16_t)0x3D5));
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
  __asm__ volatile("outb %0, %1"
                   :
                   : "a"((uint8_t)((pos >> 8) & 0xFF)), "Nd"((uint16_t)0x3D5));
}

void scroll() {
  for (int i = 1 * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
  for (int i = (VGA_HEIGHT - 2) * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH;
       i++)
    vga[i] = (cur_col << 8) | ' ';
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
  update_cursor();
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}

void print_at(int x, int y, const char *s, uint8_t col) {
  int old_x = cur_x, old_y = cur_y;
  cur_x = x;
  cur_y = y;
  uint8_t old_col = cur_col;
  cur_col = col;
  while (*s) {
    put_char_raw(*s++, cur_col, cur_x++, cur_y);
  }
  cur_x = old_x;
  cur_y = old_y;
  cur_col = old_col;
}

void draw_rect(int x, int y, int w, int h, uint8_t col) {
  for (int i = y; i < y + h; i++)
    for (int j = x; j < x + w; j++)
      put_char_raw(' ', col, j, i);
}

void draw_window(int x, int y, int w, int h, const char *title, uint8_t bg) {
  draw_rect(x + 1, y + 1, w, h, 0x08); // Shadow
  draw_rect(x, y, w, h, bg);           // Body
  draw_rect(x, y, w, 1, 0x3F);         // Title
  print_at(x + 1, y, title, 0x3F);
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
  uint8_t h = get_rtc(0x04), m = get_rtc(0x02);
  h = ((h & 0xF0) >> 1) + ((h & 0xF0) >> 3) + (h & 0x0F);
  m = ((m & 0xF0) >> 1) + ((m & 0xF0) >> 3) + (m & 0x0F);
  h = (h + 3) % 24;
  buf[0] = h / 10 + '0';
  buf[1] = h % 10 + '0';
  buf[2] = ':';
  buf[3] = m / 10 + '0';
  buf[4] = m % 10 + '0';
  buf[5] = 0;
}

/* ============= KEYBOARD ============= */
char get_scancode() {
  if (inb(0x64) & 1)
    return inb(0x60);
  return 0;
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

/* ============= TrEdit EDITOR ============= */
#define EDIT_BUF 2048
static char edit_buffer[EDIT_BUF];
void tredit(const char *filename) {
  memset(edit_buffer, 0, EDIT_BUF);
  int pos = 0;
  bool shift = false, run = true;
  while (run) {
    draw_rect(0, 0, 80, 25, 0x1F);
    draw_rect(0, 0, 80, 1, 0x70);
    print_at(2, 0, "TrEdit v1.0 - File: ", 0x70);
    print_at(22, 0, filename, 0x70);
    print_at(60, 0, "F10/ESC: Exit", 0x70);
    int line = 2, col = 2;
    for (int i = 0; i < pos; i++) {
      if (edit_buffer[i] == '\n') {
        line++;
        col = 2;
      } else {
        put_char_raw(edit_buffer[i], 0x1F, col++, line);
      }
    }
    put_char_raw('_', 0x1E, col, line);
    while (1) {
      uint8_t sc = get_scancode();
      if (!sc)
        continue;
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;
      else if (sc & 0x80)
        continue;
      else if (sc == 0x44 || sc == 0x01) {
        run = false;
        break;
      } else {
        char ch = scancode_to_char(sc, shift);
        if (ch == '\n') {
          edit_buffer[pos++] = '\n';
          break;
        } else if (ch == '\b') {
          if (pos > 0)
            edit_buffer[--pos] = 0;
          break;
        } else if (ch && pos < EDIT_BUF - 1) {
          edit_buffer[pos++] = ch;
          break;
        }
      }
      for (volatile int d = 0; d < 50000; d++)
        ;
    }
  }
}

/* ============= SHELL & UI ============= */
void draw_ui() {
  draw_rect(0, 0, 80, 1, 0x3F);
  print_at(2, 0, "TarkOS v1.4.1", 0x3F);
  char tb[10];
  get_time_str(tb);
  print_at(72, 0, tb, 0x3F);
  draw_rect(0, 24, 80, 1, 0x70);
  print_at(2, 24, "tredit: edit | ls: list | cls: clear | help", 0x70);
}

void shell_loop() {
  char line[64];
  int pos = 0;
  bool shift = false;
  draw_ui();
  while (1) {
    set_color(10, 1);
    print("\nroot@tarkos");
    set_color(15, 1);
    print(":# ");
    pos = 0;
    memset(line, 0, 64);
    while (1) {
      uint8_t sc = get_scancode();
      static uint32_t t = 0;
      if (++t > 1000000) {
        draw_ui();
        t = 0;
      }
      if (!sc)
        continue;
      if (sc == 0x2A || sc == 0x36) {
        shift = true;
        continue;
      }
      if (sc == 0xAA || sc == 0xB6) {
        shift = false;
        continue;
      }
      if (sc & 0x80)
        continue;
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
      for (volatile int d = 0; d < 50000; d++)
        ;
    }
    if (pos == 0)
      continue;
    line[pos] = 0;
    if (strcmp(line, "help") == 0)
      print("Available: tredit, ls, cls, ver, info, reboot\n");
    else if (strcmp(line, "ls") == 0)
      print("bin  readme.txt  kernel.bin\n");
    else if (strcmp(line, "cls") == 0) {
      draw_rect(0, 1, 80, 23, 0x1F);
      cur_x = 0;
      cur_y = 1;
    } else if (strncmp(line, "tredit", 6) == 0)
      tredit("file.txt");
    else if (strcmp(line, "reboot") == 0)
      outb(0x64, 0xFE);
    else {
      print("Unknown command.\n");
    }
  }
}

/* ============= MAIN ============= */
void kmain() {
  set_color(15, 1);
  clear_screen();
  draw_window(20, 10, 40, 5, " System Init ", 0x7F);
  print_at(22, 12, "Starting TarkSH...", 0x7F);
  for (volatile int i = 0; i < 1000000; i++)
    ;

  clear_screen();
  update_cursor();
  cur_y = 1;
  shell_loop();
}
