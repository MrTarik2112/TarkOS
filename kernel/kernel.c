/**
 * TarkOS v1.2 - Mouse Support Update
 * Features: Mouse Driver, Software Cursor, 50+ Commands, 5 Games
 */

/* ============= HEADERS ============= */
// Minimal definitions
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef int bool;
// For memset/strlen
typedef unsigned long size_t;

#define true 1
#define false 0
#define NULL ((void *)0)

// Multiboot header
typedef struct multiboot_info {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t syms[4];
  uint32_t mmap_length;
  uint32_t mmap_addr;
} multiboot_info_t;

/* ============= VGA DRIVER ============= */
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

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

static uint16_t *vga = (uint16_t *)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t cur_fg = WHITE, cur_bg = BLUE;

#define VGA_ENTRY(c, fg, bg) ((uint16_t)((c) | ((fg) << 8) | ((bg) << 12)))

void set_color(uint8_t fg, uint8_t bg) {
  cur_fg = fg;
  cur_bg = bg;
}

void put_entry_at(char c, uint8_t color, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    vga[y * VGA_WIDTH + x] = (uint16_t)c | ((uint16_t)color << 8);
}

void put_char_at(int x, int y, char c, uint8_t fg, uint8_t bg) {
  put_entry_at(c, (bg << 4) | (fg & 0x0F), x, y);
}

// Get the character and color at specific position (for mouse cursor restore)
uint16_t get_entry_at(int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    return vga[y * VGA_WIDTH + x];
  return 0;
}

void clear_screen(void) {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = VGA_ENTRY(' ', cur_fg, cur_bg);
  cursor_x = 0;
  cursor_y = 0;
}

void scroll(void) {
  for (int y = 1; y < VGA_HEIGHT - 1; y++)
    for (int x = 0; x < VGA_WIDTH; x++)
      vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
  for (int x = 0; x < VGA_WIDTH; x++)
    vga[(VGA_HEIGHT - 2) * VGA_WIDTH + x] = VGA_ENTRY(' ', cur_fg, cur_bg);
}

void put_char(char c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
  } else if (c == '\b') {
    if (cursor_x > 0) {
      cursor_x--;
      put_char_at(cursor_x, cursor_y, ' ', cur_fg, cur_bg);
    }
  } else if (c == '\t') {
    cursor_x = (cursor_x + 4) & ~3;
  } else {
    put_char_at(cursor_x, cursor_y, c, cur_fg, cur_bg);
    cursor_x++;
  }
  if (cursor_x >= VGA_WIDTH) {
    cursor_x = 0;
    cursor_y++;
  }
  if (cursor_y >= VGA_HEIGHT - 1) {
    scroll();
    cursor_y = VGA_HEIGHT - 2;
  }
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}
void print_color(const char *s, uint8_t fg) {
  uint8_t old = cur_fg;
  cur_fg = fg;
  print(s);
  cur_fg = old;
}
void print_at(int x, int y, const char *s, uint8_t fg, uint8_t bg) {
  while (*s)
    put_char_at(x++, y, *s++, fg, bg);
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

// Standard memset signature to avoid conflicts
void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  while (n--)
    *p++ = (unsigned char)c;
  return s;
}

int atoi(const char *s) {
  int n = 0, neg = 0;
  while (*s == ' ')
    s++;
  if (*s == '-') {
    neg = 1;
    s++;
  }
  while (*s >= '0' && *s <= '9')
    n = n * 10 + (*s++ - '0');
  return neg ? -n : n;
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

void print_int(int n) {
  char buf[16];
  itoa(n, buf);
  print(buf);
}

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

/* ============= MOUSE DRIVER ============= */
// Mouse state
static int mouse_x = 40, mouse_y = 12;
static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];
static bool mouse_lbtn = false, mouse_rbtn = false;
static uint16_t old_entry = 0; // Character under cursor
static int old_mouse_x = 40, old_mouse_y = 12;

void mouse_wait(uint8_t type) {
  uint32_t timeout = 100000;
  if (type == 0) { // Data
    while (timeout--)
      if ((inb(0x64) & 1))
        return;
  } else { // Signal
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

  // Enable Aux Device
  mouse_wait(1);
  outb(0x64, 0xA8);

  // Enable Interrupts
  mouse_wait(1);
  outb(0x64, 0x20);
  mouse_wait(0);
  status = (inb(0x60) | 2); // Enable IRQ12
  mouse_wait(1);
  outb(0x64, 0x60);
  mouse_wait(1);
  outb(0x60, status);

  // Use default settings
  mouse_write(0xF6);
  mouse_read(); // ACK

  // Enable Scanning
  mouse_write(0xF4);
  mouse_read(); // ACK

  // Initial save
  old_entry = get_entry_at(mouse_x, mouse_y);
}

void update_mouse() {
  // Check if data available
  if (inb(0x64) & 1) {
    uint8_t data = inb(0x60);

    // This handles packet sync
    switch (mouse_cycle) {
    case 0:
      if (data & 0x08) { // Valid packet?
        mouse_packet[0] = data;
        mouse_cycle++;
      }
      break;
    case 1:
      mouse_packet[1] = data;
      mouse_cycle++;
      break;
    case 2:
      mouse_packet[2] = data;
      mouse_cycle = 0;

      // Process packet
      int dx = mouse_packet[1];
      int dy = mouse_packet[2];
      if (mouse_packet[0] & 0x10)
        dx |= 0xFFFFFF00;
      if (mouse_packet[0] & 0x20)
        dy |= 0xFFFFFF00;

      dy = -dy; // Flip Y

      // Update buttons
      mouse_lbtn = (mouse_packet[0] & 1);
      mouse_rbtn = (mouse_packet[0] & 2);

      // Move (Scale down for text mode sensitivity)
      mouse_x += dx / 4;
      mouse_y += dy / 8;

      // Clamp
      if (mouse_x < 0)
        mouse_x = 0;
      if (mouse_x >= VGA_WIDTH)
        mouse_x = VGA_WIDTH - 1;
      if (mouse_y < 0)
        mouse_y = 0;
      if (mouse_y >= VGA_HEIGHT)
        mouse_y = VGA_HEIGHT - 1;

      // Redraw if moved
      if (mouse_x != old_mouse_x || mouse_y != old_mouse_y) {
        // Restore old
        vga[old_mouse_y * VGA_WIDTH + old_mouse_x] = old_entry;

        // Save new
        old_entry = get_entry_at(mouse_x, mouse_y);
        old_mouse_x = mouse_x;
        old_mouse_y = mouse_y;

        // Draw new (Invert colors - XOR with 0x7700)
        uint16_t current = old_entry;
        uint16_t inverted = (current & 0x00FF) | 0x7000; // Grey background
        vga[mouse_y * VGA_WIDTH + mouse_x] = inverted;
      }
      break;
    }
  }
}

/* ============= TIMER & RTC ============= */
volatile uint32_t ticks = 0;
// Busy-wait sleep with mouse update support
void sleep(uint32_t ms) {
  uint32_t loops = ms * 4000; // Calibrated
  for (volatile uint32_t i = 0; i < loops; i++) {
    update_mouse(); // Keep mouse alive during sleep
    __asm__ volatile("nop");
  }
}

// RTC
#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

uint8_t get_rtc(int reg) {
  outb(CMOS_ADDR, reg);
  return inb(CMOS_DATA);
}

void get_time(int *h, int *m, int *s, int *d, int *mo, int *y) {
  uint8_t statusB = get_rtc(0x0B);
  bool bcd = !(statusB & 0x04);
  *s = get_rtc(0x00);
  *m = get_rtc(0x02);
  *h = get_rtc(0x04);
  *d = get_rtc(0x07);
  *mo = get_rtc(0x08);
  *y = get_rtc(0x09);
  if (bcd) {
    *s = ((*s & 0xF0) >> 1) + ((*s & 0xF0) >> 3) + (*s & 0x0F);
    *m = ((*m & 0xF0) >> 1) + ((*m & 0xF0) >> 3) + (*m & 0x0F);
    *h = ((*h & 0xF0) >> 1) + ((*h & 0xF0) >> 3) + (*h & 0x0F);
    *d = ((*d & 0xF0) >> 1) + ((*d & 0xF0) >> 3) + (*d & 0x0F);
    *mo = ((*mo & 0xF0) >> 1) + ((*mo & 0xF0) >> 3) + (*mo & 0x0F);
    *y = ((*y & 0xF0) >> 1) + ((*y & 0xF0) >> 3) + (*y & 0x0F);
  }
  *y += 2000;
}

/* ============= HARDWARE INFO ============= */
void get_cpu_brand(char *buffer) {
  uint32_t ebx, ecx, edx;
  __asm__ volatile("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
  *((uint32_t *)buffer) = ebx;
  *((uint32_t *)(buffer + 4)) = edx;
  *((uint32_t *)(buffer + 8)) = ecx;
  buffer[12] = 0;
}

uint32_t rand_seed = 12345;
uint32_t rand(void) {
  return (rand_seed = rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
}

/* ============= KEYBOARD ============= */
char get_key(void) {
  static const char lower[] = {
      0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
      '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
      'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
      'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
      'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};
  static const char upper[] = {
      0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
      '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
      'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
      'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' '};
  static bool shift = false;

  while (1) {
    update_mouse(); // Important: Keep mouse moving

    // RTC Clock
    if ((ticks % 10000) == 0) {
      int h, m, s, d, mo, y;
      get_time(&h, &m, &s, &d, &mo, &y);
      h = (h + 3) % 24;
      char timebuf[10];
      timebuf[0] = h / 10 + '0';
      timebuf[1] = h % 10 + '0';
      timebuf[2] = ':';
      timebuf[3] = m / 10 + '0';
      timebuf[4] = m % 10 + '0';
      timebuf[5] = 0;
      // Don't draw over mouse
      put_char_at(70, 24, timebuf[0], WHITE, BLACK);
      put_char_at(71, 24, timebuf[1], WHITE, BLACK);
      put_char_at(72, 24, timebuf[2], WHITE, BLACK);
      put_char_at(73, 24, timebuf[3], WHITE, BLACK);
      put_char_at(74, 24, timebuf[4], WHITE, BLACK);
    }
    ticks++;

    if (inb(0x64) & 1) {
      uint8_t sc = inb(0x60);
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;
      else if (!(sc & 0x80) && sc < 58)
        return shift ? upper[sc] : lower[sc];
    }
    // Minimal delay
    for (volatile int i = 0; i < 1000; i++)
      ;
    // ticks++; // Removed duplicate ticks increment
  }
}

// Non-blocking key check for games
char check_key(void) {
  update_mouse(); // Maintain movement during games
  if (inb(0x64) & 1) {
    uint8_t sc = inb(0x60);
    if (!(sc & 0x80)) {
      const char map[] = {0,    27,  '1', '2',  '3',  '4',  '5', '6', '7',  '8',
                          '9',  '0', '-', '=',  '\b', '\t', 'q', 'w', 'e',  'r',
                          't',  'y', 'u', 'i',  'o',  'p',  '[', ']', '\n', 0,
                          'a',  's', 'd', 'f',  'g',  'h',  'j', 'k', 'l',  ';',
                          '\'', '`', 0,   '\\', 'z',  'x',  'c', 'v', 'b',  'n',
                          'm',  ',', '.', '/',  0,    '*',  0,   ' '};
      return (sc < 58) ? map[sc] : 0;
    }
  }
  return 0;
}

/* ============= FILE SYSTEM ============= */
#define MAX_FILES 32
#define MAX_NAME 32
#define MAX_CONTENT 2048

typedef struct {
  char name[MAX_NAME];
  char data[MAX_CONTENT];
  int size;
  int parent_id;
  bool is_dir;
  bool used;
} inode_t;
inode_t fs[MAX_FILES];
int current_dir = 0;

void fs_init() {
  memset(fs, 0, sizeof(fs));
  strcpy(fs[0].name, "root");
  fs[0].is_dir = true;
  fs[0].used = true;
  fs[0].parent_id = 0;

  int id = 1;
  strcpy(fs[1].name, "readme.txt");
  strcpy(fs[1].data, "TarkOS v1.2 with Mouse Support!\nMove the mouse to see "
                     "the software cursor.");
  fs[1].size = strlen(fs[1].data);
  fs[1].used = true;
  fs[1].parent_id = 0;
}

int fs_find(const char *name) {
  for (int i = 0; i < MAX_FILES; i++)
    if (fs[i].used && strcmp(fs[i].name, name) == 0 &&
        fs[i].parent_id == current_dir)
      return i;
  return -1;
}

int fs_create(const char *name, bool is_dir) {
  if (fs_find(name) != -1)
    return -1;
  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs[i].used) {
      strcpy(fs[i].name, name);
      fs[i].size = 0;
      fs[i].data[0] = 0;
      fs[i].used = true;
      fs[i].is_dir = is_dir;
      fs[i].parent_id = current_dir;
      return i;
    }
  }
  return -2;
}

/* ============= SHELL COMMANDS ============= */
void cmd_ls() {
  print("\nListing directory:\n");
  int cnt = 0;
  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].used && fs[i].parent_id == current_dir && i != 0) {
      print(fs[i].is_dir ? "[" : "");
      print_color(fs[i].name, fs[i].is_dir ? LIGHT_BLUE : WHITE);
      print(fs[i].is_dir ? "]" : "");
      print("\n");
      cnt++;
    }
  }
  print("Total: ");
  print_int(cnt);
  print(" files\n");
}

void cmd_cat(char *arg) {
  int id = fs_find(arg);
  if (id == -1 || fs[id].is_dir)
    print("\nError: File not found.\n");
  else {
    print("\n");
    print(fs[id].data);
    print("\n");
  }
}

void cmd_info(multiboot_info_t *mboot) {
  set_color(LIGHT_CYAN, BLUE);
  print("\n=== System Information ===\n");
  set_color(WHITE, BLUE);
  print("  OS:       TarkOS v1.2 (Mouse Enabled)\n");

  char cpu[13];
  get_cpu_brand(cpu);
  print("  CPU:      ");
  print(cpu);
  print("\n");

  int mem = 0;
  if (mboot->flags & 1)
    mem = (mboot->mem_lower + mboot->mem_upper) / 1024;
  print("  RAM:      ");
  print_int(mem);
  print(" MB\n");
}

void cmd_date() {
  int h, m, s, d, mo, y;
  get_time(&h, &m, &s, &d, &mo, &y);
  h = (h + 3) % 24;
  print("\nDate: ");
  print_int(d);
  print("/");
  print_int(mo);
  print("/");
  print_int(y);
  print("\nTime: ");
  print_int(h);
  print(":");
  print_int(m);
  print(":");
  print_int(s);
  print("\n");
}

void game_snake() {
  struct {
    int x, y;
  } snake[100];
  int len = 3, dx = 1, dy = 0, score = 0, fx = 20, fy = 10;
  for (int i = 0; i < len; i++) {
    snake[i].x = 10 - i;
    snake[i].y = 10;
  }

  clear_screen();
  while (1) {
    // Draw Border
    for (int x = 0; x < 40; x++) {
      put_char_at(x, 1, '#', WHITE, BLACK);
      put_char_at(x, 20, '#', WHITE, BLACK);
    }
    for (int y = 1; y < 21; y++) {
      put_char_at(0, y, '#', WHITE, BLACK);
      put_char_at(39, y, '#', WHITE, BLACK);
    }

    print_at(2, 0, "Score: ", YELLOW, BLACK);
    print_int(score);
    put_char_at(fx, fy, '*', RED, BLACK);

    for (int i = 0; i < len; i++)
      put_char_at(snake[i].x, snake[i].y, 'O', GREEN, BLACK);

    char c = check_key();
    if (c == 'w' && dy == 0) {
      dx = 0;
      dy = -1;
    }
    if (c == 's' && dy == 0) {
      dx = 0;
      dy = 1;
    }
    if (c == 'a' && dx == 0) {
      dx = -1;
      dy = 0;
    }
    if (c == 'd' && dx == 0) {
      dx = 1;
      dy = 0;
    }
    if (c == 'q')
      break;

    // Move
    put_char_at(snake[len - 1].x, snake[len - 1].y, ' ', BLACK, BLACK);
    for (int i = len - 1; i > 0; i--)
      snake[i] = snake[i - 1];
    snake[0].x += dx;
    snake[0].y += dy;

    // Collision
    if (snake[0].x <= 0 || snake[0].x >= 39 || snake[0].y <= 1 ||
        snake[0].y >= 20)
      break;
    for (int i = 1; i < len; i++)
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        goto end;

    if (snake[0].x == fx && snake[0].y == fy) {
      score += 10;
      if (len < 99)
        len++;
      fx = (rand() % 38) + 1;
      fy = (rand() % 18) + 2;
    }
    sleep(50);
  }
end:
  set_color(WHITE, BLUE);
  clear_screen();
  print("Game Over! Score: ");
  print_int(score);
  print("\n");
}

/* ============= KERNEL MAIN ============= */
void kernel_main(uint32_t magic, multiboot_info_t *mboot) {
  // Disable interrupts for polling safety
  __asm__ volatile("cli");

  fs_init();
  mouse_init(); // Initialize PS/2 Mouse

  set_color(WHITE, BLUE);
  clear_screen();

  // Header
  for (int x = 0; x < 80; x++)
    put_char_at(x, 0, ' ', WHITE, CYAN);
  print_at(2, 0, "TarkOS v1.2 - Mouse Enabled", WHITE, CYAN);

  cursor_y = 2;
  print_color("Welcome to TarkOS v1.2!\n", LIGHT_GREEN);

  cmd_info(mboot);
  print("\nMove the mouse or type 'help'.\n");

  char cmd[64];
  while (1) {
    print_color("\nroot@tarkos", LIGHT_GREEN);
    print(":$ ");

    int pos = 0;
    memset(cmd, 0, 64);
    while (1) {
      char c = get_key(); // This also updates mouse
      if (c == '\n') {
        put_char('\n');
        break;
      } else if (c == '\b') {
        if (pos > 0) {
          pos--;
          put_char('\b');
        }
      } else if (c) {
        if (pos < 63) {
          cmd[pos++] = c;
          put_char(c);
        }
      }
    }

    if (strlen(cmd) == 0)
      continue;
    else if (strcmp(cmd, "help") == 0)
      print("\nCommands: ls, cat, info, date, snake, clear, reboot\n");
    else if (strcmp(cmd, "ls") == 0)
      cmd_ls();
    else if (strcmp(cmd, "info") == 0)
      cmd_info(mboot);
    else if (strcmp(cmd, "date") == 0)
      cmd_date();
    else if (strcmp(cmd, "snake") == 0)
      game_snake();
    else if (strcmp(cmd, "clear") == 0) {
      clear_screen();
      cursor_y = 2;
    } else if (strcmp(cmd, "reboot") == 0)
      outb(0x64, 0xFE);
    else if (strncmp(cmd, "cat ", 4) == 0)
      cmd_cat(cmd + 4);
    else
      print("\nUnknown command.\n");
  }
}
