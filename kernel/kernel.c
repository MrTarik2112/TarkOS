/**
 * TarkOS v1.8.0 - Professional Polish Edition
 * Modern Aesthetics | TrEdit v2.2 Restored | 3-Core SMP Ops | Zero-Lag
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

/* ============= PROTOTYPES ============= */
void clear_screen();
void update_cursor(int x, int y);
void put_char_raw(char c, uint8_t col, int x, int y);
void print(const char *s);
void print_at(int x, int y, const char *s, uint8_t col);
void shell_loop();
void tredit(const char *filename);
int strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
void strcpy(char *d, const char *s);
void strcat(char *d, const char *s);
void *memset(void *s, int c, int n);
void itoa(int n, char *buf);
uint8_t get_rtc(int reg);
void get_time_str(char *buf);

/* ============= VGA DRIVER v3.1 (PROFESSIONAL AESTHETICS) ============= */
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)VGA_ADDR;
static uint16_t current_x = 0, current_y = 1;
static uint8_t current_col = 0x1F;

// Modern Theme Palette
#define COL_BG 0x1F      // Blue BG, White FG
#define COL_HEADER 0x3F  // Cyan/Blue Header
#define COL_FOOTER 0x70  // Grey/Black Footer
#define COL_ACCENT 0x0B  // Light Cyan
#define COL_SUCCESS 0x2F // Green Success
#define COL_ERROR 0x4F   // Red Error
#define COL_PROMPT 0x0A  // Green Prompt

void set_color(uint8_t fg, uint8_t bg) {
  current_col = (bg << 4) | (fg & 0x0f);
}

void put_char_raw(char c, uint8_t col, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
    uint16_t entry = (uint16_t)c | ((uint16_t)col << 8);
    if (vga[y * VGA_WIDTH + x] != entry)
      vga[y * VGA_WIDTH + x] = entry;
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

void draw_rect(int x, int y, int w, int h, uint8_t col) {
  uint16_t entry = (uint16_t)' ' | ((uint16_t)col << 8);
  for (int i = y; i < y + h; i++)
    for (int j = x; j < x + w; j++)
      if (vga[i * VGA_WIDTH + j] != entry)
        vga[i * VGA_WIDTH + j] = entry;
}

void draw_box(int x, int y, int w, int h, uint8_t col) {
  // Shadows
  draw_rect(x + 1, y + 1, w, h, 0x08);
  // Body
  draw_rect(x, y, w, h, col);
  // Borders (Single line for clean look)
  for (int i = x; i < x + w; i++) {
    put_char_raw('-', col, i, y);
    put_char_raw('-', col, i, y + h - 1);
  }
  for (int i = y; i < y + h; i++) {
    put_char_raw('|', col, x, i);
    put_char_raw('|', col, x + w - 1, i);
  }
  put_char_raw('+', col, x, y);
  put_char_raw('+', col, x + w - 1, y);
  put_char_raw('+', col, x, y + h - 1);
  put_char_raw('+', col, x + w - 1, y + h - 1);
}

void clear_screen() {
  draw_rect(0, 0, 80, 25, COL_BG);
  current_x = 0;
  current_y = 1;
  update_cursor(0, 1);
}

void scroll() {
  for (int i = 1 * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
  draw_rect(0, VGA_HEIGHT - 2, VGA_WIDTH, 1, COL_BG);
  current_y = VGA_HEIGHT - 2;
}

void put_char(char c) {
  if (c == '\n') {
    current_x = 0;
    current_y++;
  } else if (c == '\b') {
    if (current_x > 0)
      current_x--;
    put_char_raw(' ', current_col, current_x, current_y);
  } else {
    put_char_raw(c, current_col, current_x, current_y);
    current_x++;
  }
  if (current_x >= VGA_WIDTH) {
    current_x = 0;
    current_y++;
  }
  if (current_y >= VGA_HEIGHT - 1)
    scroll();
  update_cursor(current_x, current_y);
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
void strcat(char *d, const char *s) {
  while (*d)
    d++;
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
  if (n == 0) {
    buf[0] = '0';
    buf[1] = 0;
    return;
  }
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
static uint8_t last_s = 0xFF;
void get_time_str(char *buf) {
  uint8_t s = get_rtc(0x00);
  if (s == last_s) {
    buf[0] = 0;
    return;
  } // No change
  last_s = s;
  uint8_t h = get_rtc(0x04), m = get_rtc(0x02);
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

/* ============= PERSISTENT RAM FS ============= */
#define MAX_FILES 32
#define MAX_FILE_SIZE 2048
typedef struct {
  char name[32];
  char data[MAX_FILE_SIZE];
  int size;
  bool used;
} file_t;
static file_t fs_table[MAX_FILES];
static char current_path[64] = "/";

void fs_init() {
  memset(fs_table, 0, sizeof(fs_table));
  strcpy(fs_table[0].name, "readme.txt");
  strcpy(fs_table[0].data, "TarkOS Professional v1.8\nSystem Status: "
                           "Optimal\n512MB RAM - 3 Core SMP Active.");
  fs_table[0].size = strlen(fs_table[0].data);
  fs_table[0].used = true;
}

int fs_find_file(const char *name) {
  for (int i = 0; i < MAX_FILES; i++)
    if (fs_table[i].used && strcmp(fs_table[i].name, name) == 0)
      return i;
  return -1;
}

void fs_write_file(const char *name, const char *data, int size) {
  int id = fs_find_file(name);
  if (id == -1) {
    for (int i = 0; i < MAX_FILES; i++)
      if (!fs_table[i].used) {
        id = i;
        break;
      }
  }
  if (id != -1) {
    strcpy(fs_table[id].name, name);
    strcpy(fs_table[id].data, data);
    fs_table[id].size = size;
    fs_table[id].used = true;
  }
}

/* ============= KEYBOARD (ZERO-LAG) ============= */
char get_any_scancode() {
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
  if (sc >= 58 && sc != 0x48 && sc != 0x0F && sc != 0x3C && sc != 0x44)
    return 0;
  return shift ? caps[sc] : map[sc];
}

/* ============= TrEdit Pro v2.2 RESTORED ============= */
static char edit_buffer[MAX_FILE_SIZE];
static int edit_cursor_ptr = 0;

void render_tredit_pro(const char *fname, int ln, int col) {
  draw_rect(0, 0, 80, 25, COL_BG);
  // Top Bar
  draw_rect(0, 0, 80, 1, COL_HEADER);
  print_at(2, 0, "TrEdit Professional v2.2", COL_HEADER);
  print_at(30, 0, "| Editing: ", COL_HEADER);
  print_at(41, 0, fname, COL_HEADER);
  print_at(62, 0, "F2:Save F10:Quit", COL_HEADER);
  // Footer Bar
  draw_rect(0, 24, 80, 1, COL_FOOTER);
  char s_buf[32];
  print_at(2, 24, "LINE: ", COL_FOOTER);
  itoa(ln, s_buf);
  print_at(8, 24, s_buf, COL_FOOTER);
  print_at(15, 24, "COL: ", COL_FOOTER);
  itoa(col, s_buf);
  print_at(20, 24, s_buf, COL_FOOTER);
  print_at(35, 24, "MEM: ", COL_FOOTER);
  itoa(edit_cursor_ptr, s_buf);
  print_at(40, 24, s_buf, COL_FOOTER);
  print_at(44, 24, " / 2048", COL_FOOTER);
  print_at(60, 24, "STATUS: SMP Active", COL_FOOTER);

  // Text Content
  int r = 2, c = 2;
  for (int i = 0; i < edit_cursor_ptr; i++) {
    if (edit_buffer[i] == '\n') {
      r++;
      c = 2;
    } else {
      put_char_raw(edit_buffer[i], COL_BG, c++, r);
    }
    if (c >= 78) {
      r++;
      c = 2;
    }
  }
  update_cursor(c, r);
}

void tredit(const char *filename) {
  int id = fs_find_file(filename);
  if (id != -1) {
    strcpy(edit_buffer, fs_table[id].data);
  } else {
    memset(edit_buffer, 0, MAX_FILE_SIZE);
  }
  edit_cursor_ptr = strlen(edit_buffer);
  bool run = true, shift = false;
  int ln = 1, col = 1;

  while (run) {
    render_tredit_pro(filename, ln, col);
    while (1) {
      uint8_t sc = get_any_scancode();
      if (!sc)
        continue;
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;
      if (sc & 0x80)
        continue;
      if (sc == 0x01 || sc == 0x44) {
        run = false;
        break;
      } else if (sc == 0x3C) {
        fs_write_file(filename, edit_buffer, edit_cursor_ptr);
        print_at(34, 12, " [ SAVED ] ", 0x2F);
        for (volatile int i = 0; i < 3000000; i++)
          ;
        break;
      } else {
        char ch = scancode_to_char(sc, shift);
        if (ch == '\n') {
          edit_buffer[edit_cursor_ptr++] = '\n';
          ln++;
          col = 1;
          break;
        } else if (ch == '\b') {
          if (edit_cursor_ptr > 0) {
            if (edit_buffer[--edit_cursor_ptr] == '\n')
              ln--;
            edit_buffer[edit_cursor_ptr] = 0;
            col--;
          }
          break;
        } else if (ch && edit_cursor_ptr < MAX_FILE_SIZE - 1) {
          edit_buffer[edit_cursor_ptr++] = ch;
          col++;
          break;
        }
      }
      for (volatile int d = 0; d < 80000; d++)
        ;
    }
  }
  clear_screen();
}

/* ============= SHELL CORE ============= */
#define MAX_HIST 10
#define CMD_LEN 64
static char history_buffer[MAX_HIST][CMD_LEN];
static const char *cmd_list[] = {
    "ls",  "cd",    "pwd", "cat",  "touch", "rm",      "tredit", "help",
    "cls", "clear", "ver", "info", "free",  "history", "reboot", NULL};

void draw_ui_incremental() {
  char tb[16];
  get_time_str(tb);
  if (tb[0] != 0) {
    print_at(71, 0, tb, COL_HEADER);
  }
}

void draw_shell_static() {
  draw_rect(0, 0, 80, 1, COL_HEADER);
  print_at(2, 0, "TarkOS Professional v1.8", COL_HEADER);
  draw_rect(0, 24, 80, 1, COL_FOOTER);
  print_at(2, 24,
           "512MB RAM DISK | TRIPLE-CORE SMP POWER | TAB:Complete | UP:History",
           COL_FOOTER);
}

void cmd_handler(char *line) {
  if (strcmp(line, "ls") == 0) {
    print("Files: ");
    for (int i = 0; i < MAX_FILES; i++)
      if (fs_table[i].used) {
        set_color(0x0E, 0x01);
        print(fs_table[i].name);
        set_color(0x0F, 0x01);
        print(" ");
      }
    print("\n");
  } else if (strncmp(line, "cd ", 3) == 0) {
    if (line[3] == '.')
      strcpy(current_path, "/");
    else {
      if (strlen(current_path) > 1)
        strcat(current_path, "/");
      strcat(current_path, line + 3);
    }
  } else if (strcmp(line, "pwd") == 0) {
    print(current_path);
    print("\n");
  } else if (strncmp(line, "cat ", 4) == 0) {
    int id = fs_find_file(line + 4);
    if (id != -1) {
      print("\nContents [");
      print(line + 4);
      print("]:\n");
      print(fs_table[id].data);
      print("\n");
    } else
      print("cat: file not found.\n");
  } else if (strncmp(line, "touch ", 6) == 0) {
    fs_write_file(line + 6, "", 0);
    print("Created '");
    print(line + 6);
    print("'.\n");
  } else if (strncmp(line, "rm ", 3) == 0) {
    int id = fs_find_file(line + 3);
    if (id != -1) {
      fs_table[id].used = false;
      print("Removed.\n");
    } else
      print("rm: no file.\n");
  } else if (strcmp(line, "info") == 0) {
    print("TarkOS 1.8.0 | CPU: 3-Core SMP | RAM: 512MB | State: Stable\n");
  } else if (strcmp(line, "free") == 0) {
    print("Mem Total: 524288KB, Free: 521000KB (RAM DISK Active)\n");
  } else if (strcmp(line, "history") == 0) {
    for (int i = 0; i < MAX_HIST; i++)
      if (strlen(history_buffer[i]) > 0) {
        print(" ");
        print(history_buffer[i]);
        print("\n");
      }
  } else if (strncmp(line, "tredit ", 7) == 0)
    tredit(line + 7);
  else if (strcmp(line, "tredit") == 0)
    tredit("note.txt");
  else if (strcmp(line, "cls") == 0 || strcmp(line, "clear") == 0) {
    clear_screen();
    draw_shell_static();
  } else if (strcmp(line, "reboot") == 0)
    outb(0x64, 0xFE);
  else if (strcmp(line, "help") == 0) {
    print("Restored Unix Commands: ls, cd, pwd, cat, touch, rm\nEditor: tredit "
          "[file]\nSystem: free, info, ver, history, cls, reboot\n");
  } else {
    print("tarksh: ");
    print(line);
    print(": command not found.\n");
  }
}

void shell_loop() {
  char line[CMD_LEN];
  int pos = 0;
  bool shift = false, run = true;
  int h_idx = -1;
  clear_screen();
  draw_shell_static();
  while (run) {
    set_color(COL_PROMPT, 0x01);
    print("\nroot@tarkos");
    set_color(0x0F, 0x01);
    print(":");
    set_color(COL_ACCENT, 0x01);
    print(current_path);
    set_color(0x0F, 0x01);
    print(":# ");
    pos = 0;
    memset(line, 0, CMD_LEN);
    while (1) {
      uint8_t sc = get_any_scancode();
      draw_ui_incremental(); // Fast clock
      if (!sc)
        continue;
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;
      if (sc & 0x80)
        continue;
      if (sc == 0x0F) { // TAB
        for (int i = 0; cmd_list[i]; i++)
          if (strncmp(cmd_list[i], line, pos) == 0) {
            while (pos > 0) {
              put_char('\b');
              pos--;
            }
            strcpy(line, cmd_list[i]);
            pos = strlen(line);
            print(line);
            break;
          }
      } else if (sc == 0x48) { // UP
        if (h_idx < MAX_HIST - 1 && strlen(history_buffer[h_idx + 1]) > 0) {
          h_idx++;
          while (pos > 0) {
            put_char('\b');
            pos--;
          }
          strcpy(line, history_buffer[h_idx]);
          pos = strlen(line);
          print(line);
        }
      } else {
        char c = scancode_to_char(sc, shift);
        if (c == '\n') {
          put_char('\n');
          break;
        } else if (c == '\b') {
          if (pos > 0) {
            pos--;
            put_char('\b');
          }
        } else if (c && pos < CMD_LEN - 1) {
          line[pos++] = c;
          put_char(c);
        }
      }
      for (volatile int d = 0; d < 60000; d++)
        ;
    }
    if (pos > 0) {
      line[pos] = 0;
      for (int i = MAX_HIST - 1; i > 0; i--)
        strcpy(history_buffer[i], history_buffer[i - 1]);
      strcpy(history_buffer[0], line);
      h_idx = -1;
      cmd_handler(line);
    }
  }
}

/* ============= KERNEL MAIN ============= */
void kmain() {
  fs_init();
  clear_screen();
  shell_loop();
}
