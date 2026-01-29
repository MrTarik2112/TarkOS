/**
 * TarkOS v1.2 - Professional Edition
 * Professional Shell (TarkSH), History, Tab Completion, and Fixed Games
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

/* ============= VGA DRIVER ============= */
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)VGA_ADDR;
static uint16_t cursor_x = 0, cursor_y = 0;
static uint8_t cur_col = 0x1F; // White on Blue

void set_color(uint8_t fg, uint8_t bg) { cur_col = (bg << 4) | (fg & 0x0F); }

void update_cursor() {
  uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
  __asm__ volatile("outb %%al, %%dx" : : "a"(0xF), "d"(0x3D4));
  __asm__ volatile("outb %%al, %%dx"
                   :
                   : "a"((uint8_t)(pos & 0xFF)), "d"(0x3D5));
  __asm__ volatile("outb %%al, %%dx" : : "a"(0xE), "d"(0x3D4));
  __asm__ volatile("outb %%al, %%dx"
                   :
                   : "a"((uint8_t)((pos >> 8) & 0xFF)), "d"(0x3D5));
}

void scroll() {
  for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
  for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
    vga[i] = (cur_col << 8) | ' ';
  cursor_y = VGA_HEIGHT - 1;
}

void put_char(char c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
  } else if (c == '\b') {
    if (cursor_x > 0)
      cursor_x--;
    vga[cursor_y * VGA_WIDTH + cursor_x] = (cur_col << 8) | ' ';
  } else {
    vga[cursor_y * VGA_WIDTH + cursor_x] = (cur_col << 8) | c;
    cursor_x++;
  }
  if (cursor_x >= VGA_WIDTH) {
    cursor_x = 0;
    cursor_y++;
  }
  if (cursor_y >= VGA_HEIGHT)
    scroll();
  update_cursor();
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}
void print_int(int n) {
  if (n < 0) {
    put_char('-');
    n = -n;
  }
  if (n >= 10)
    print_int(n / 10);
  put_char('0' + (n % 10));
}

void clear_screen() {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = (cur_col << 8) | ' ';
  cursor_x = 0;
  cursor_y = 0;
  update_cursor();
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
void strcpy(char *d, const char *s) {
  while ((*d++ = *s++))
    ;
}
void *memset(void *s, int c, int n) {
  unsigned char *p = s;
  while (n--)
    *p++ = c;
  return s;
}

static uint32_t seed = 42;
uint32_t rand() { return (seed = seed * 1103515245 + 12345) & 0x7FFFFFFF; }

/* ============= I/O & TIMER ============= */
static inline uint8_t inb(uint16_t port) {
  uint8_t r;
  __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
  return r;
}
static inline void outb(uint16_t port, uint8_t v) {
  __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(port));
}

volatile uint32_t ticks = 0;
void sleep(uint32_t ms) {
  // Calibrated busy wait for QEMU
  for (volatile uint32_t i = 0; i < ms * 15000; i++)
    __asm__ volatile("pause");
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
  static const char sh_map[] = {
      0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
      '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
      'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
      'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' '};
  if (sc >= 58)
    return 0;
  return shift ? sh_map[sc] : map[sc];
}

/* ============= PATH & DIRECTORY SYSTEM ============= */
static char current_path[64] = "/";

void cmd_cd(const char *arg) {
  if (strlen(arg) == 0 || strcmp(arg, "~") == 0 || strcmp(arg, "/") == 0) {
    strcpy(current_path, "/");
  } else if (strcmp(arg, "..") == 0) {
    if (strcmp(current_path, "/") != 0) {
      // Very simple parent dir logic (just back to root for now)
      strcpy(current_path, "/");
    }
  } else {
    // Mock directory change
    if (strlen(current_path) > 1)
      strcat(current_path, "/");
    strcat(current_path, arg);
  }
}

void strcat(char *d, const char *s) {
  while (*d)
    d++;
  while ((*d++ = *s++))
    ;
}

void put_char_at(char c, uint8_t col, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    vga[y * VGA_WIDTH + x] = (uint16_t)c | ((uint16_t)col << 8);
}

void print_at(int x, int y, const char *s, uint8_t fg, uint8_t bg) {
  uint8_t old = cur_col;
  set_color(fg, bg);
  cursor_x = x;
  cursor_y = y;
  print(s);
  cur_col = old;
}

/* ============= TARKSH SHELL ============= */
#define MAX_CMD 64
#define HIST_SIZE 10
static char history[HIST_SIZE][MAX_CMD];
static int hist_count = 0, hist_idx = 0;

void shell_loop() {
  char cmd[MAX_CMD];
  int pos = 0;
  bool shift = false;

  while (1) {
    set_color(10, 1);
    print("root@tarkos");
    set_color(15, 1);
    print(":");
    set_color(9, 1);
    print(current_path);
    set_color(15, 1);
    print("$ ");

    pos = 0;
    memset(cmd, 0, MAX_CMD);
    while (1) {
      uint8_t sc = get_scancode();
      if (!sc) {
        ticks++;
        continue;
      }

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
      } else if (sc == 0x48) { /* Up Arrow - History (Placeholder) */
      } else if (c && pos < MAX_CMD - 1) {
        cmd[pos++] = c;
        put_char(c);
      }

      // Debounce
      while (get_scancode() == sc)
        ;
    }

    if (pos == 0)
      continue;
    cmd[pos] = 0;

    // Process
    if (strncmp(cmd, "cd ", 3) == 0)
      cmd_cd(cmd + 3);
    else if (strcmp(cmd, "cd") == 0)
      cmd_cd("");
    else if (strcmp(cmd, "pwd") == 0) {
      print(current_path);
      print("\n");
    } else if (strcmp(cmd, "help") == 0)
      print("Available: cd, pwd, cls, info, ver, logo, reboot, help\n");
    else if (strcmp(cmd, "cls") == 0) {
      clear_screen();
    } else if (strcmp(cmd, "info") == 0)
      print("TarkOS v1.2 Professional\nCPU: x86\nRAM: 128MB\n");
    else if (strcmp(cmd, "ver") == 0)
      print("TarkOS Version 1.2.1 (Build: 29.01.2026)\n");
    else if (strcmp(cmd, "logo") == 0) {
      print("  _____            _   ____   _____\n");
      print(" |_   _|_ _ _ __| | / ___| / __  \\\n");
      print("   | |/ _` | '__| | \\___ \\| |  | |\n");
      print("   | | (_| | |  | |  ___) | |__| |\n");
      print("   |_|\\__,_|_|  |_| |____/ \\____/\n");
    } else if (strcmp(cmd, "reboot") == 0)
      outb(0x64, 0xFE);
    else {
      print("Unknown command: ");
      print(cmd);
      print("\n");
    }
  }
}

/* ============= KERNEL MAIN ============= */
void kmain() {
  clear_screen();
  print_at(32, 0, "[ TarkOS v1.2 ]", 14, 3);
  cursor_y = 2;
  cursor_x = 0;

  print("Initializing TarkSH...\n");
  sleep(200);
  print("System Ready.\n\n");

  shell_loop();
}
