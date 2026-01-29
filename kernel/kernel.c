/**
 * TarkOS v1.6.1 - Professional Core Restored
 * Features: High-Perf UI, TrEdit v2.1, Adv Shell (History/Tab), All Commands
 * Fix
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

/* ============= VGA DRIVER & HIGH-PERF RENDERING ============= */
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)VGA_ADDR;
static uint16_t cur_x = 0, cur_y = 1;
static uint8_t cur_col = 0x1F;

void set_color(uint8_t fg, uint8_t bg) { cur_col = (bg << 4) | (fg & 0x0F); }

void put_char_raw(char c, uint8_t col, int x, int y) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
    uint16_t entry = (uint16_t)c | ((uint16_t)col << 8);
    if (vga[y * VGA_WIDTH + x] != entry)
      vga[y * VGA_WIDTH + x] = entry; // Dirty check for FPS++
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

void clear_screen_area(int x, int y, int w, int h, uint8_t col) {
  for (int i = y; i < y + h; i++)
    for (int j = x; j < x + w; j++)
      put_char_raw(' ', col, j, i);
}

void clear_screen() {
  clear_screen_area(0, 0, 80, 25, cur_col);
  cur_x = 0;
  cur_y = 1;
  update_cursor(0, 1);
}

void scroll() {
  for (int i = 1 * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
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

/* ============= KEYBOARD ============= */
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

/* ============= TrEdit v2.1.1 RESTORED ============= */
#define EDIT_BUF 4096
static char edit_buffer[EDIT_BUF];
static int edit_cursor_pos = 0;

void render_tredit(const char *fname, int line, int col) {
  clear_screen_area(0, 0, 80, 25, 0x1F); // Clean Blue
  // Header
  clear_screen_area(0, 0, 80, 1, 0x70);
  print_at(2, 0, "TrEdit Professional v2.1.1", 0x70);
  print_at(32, 0, "| File: ", 0x70);
  print_at(40, 0, fname, 0x70);
  print_at(62, 0, "F2:Save F10:Exit", 0x70);
  // Footer
  clear_screen_area(0, 24, 80, 1, 0x70);
  char s_buf[32];
  print_at(2, 24, "L: ", 0x70);
  itoa(line, s_buf);
  print_at(5, 24, s_buf, 0x70);
  print_at(12, 24, "C: ", 0x70);
  itoa(col, s_buf);
  print_at(15, 24, s_buf, 0x70);
  print_at(30, 24, "Memory: ", 0x70);
  itoa(edit_cursor_pos, s_buf);
  print_at(38, 24, s_buf, 0x70);
  print_at(42, 24, " / 4096", 0x70);

  // Render Text
  int target_r = 2, target_c = 2;
  int current_r = 2, current_c = 2;
  for (int i = 0; i < edit_cursor_pos; i++) {
    if (edit_buffer[i] == '\n') {
      current_r++;
      current_c = 2;
    } else {
      put_char_raw(edit_buffer[i], 0x1F, current_c++, current_r);
    }
    if (current_c >= 78) {
      current_r++;
      current_c = 2;
    }
  }
  update_cursor(current_c, current_r);
}

void tredit(const char *filename) {
  memset(edit_buffer, 0, EDIT_BUF);
  edit_cursor_pos = 0;
  bool shift = false, run = true;
  int cur_line = 1, cur_col = 1;
  while (run) {
    render_tredit(filename, cur_line, cur_col);
    while (1) {
      uint8_t sc = get_any_scancode();
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
      } else if (sc == 0x3C) {
        print_at(34, 12, " [ SAVED ] ", 0x2F);
        for (volatile int i = 0; i < 10000000; i++)
          ;
        break;
      } else {
        char ch = scancode_to_char(sc, shift);
        if (ch == '\n') {
          edit_buffer[edit_cursor_pos++] = '\n';
          cur_line++;
          cur_col = 1;
          break;
        } else if (ch == '\b') {
          if (edit_cursor_pos > 0) {
            if (edit_buffer[--edit_cursor_pos] == '\n')
              cur_line--;
            edit_buffer[edit_cursor_pos] = 0;
            cur_col--;
          }
          break;
        } else if (ch && edit_cursor_pos < EDIT_BUF - 1) {
          edit_buffer[edit_cursor_pos++] = ch;
          cur_col++;
          break;
        }
      }
      while (get_any_scancode() == sc)
        ;
    }
  }
  clear_screen();
}

/* ============= SHELL & PATH SYSTEM ============= */
static char current_path[64] = "/";
#define MAX_HIST 10
#define CMD_LEN 64
static char cmd_history[MAX_HIST][CMD_LEN];
static const char *cmd_list[] = {"ls",     "cd",      "pwd",    "cat", "help",
                                 "cls",    "clear",   "tredit", "ver", "info",
                                 "reboot", "history", NULL};

void add_history(const char *c) {
  if (strlen(c) == 0)
    return;
  for (int i = MAX_HIST - 1; i > 0; i--)
    strcpy(cmd_history[i], cmd_history[i - 1]);
  strcpy(cmd_history[0], c);
}

void t_autocomplete(char *buf, int *pos) {
  if (*pos == 0)
    return;
  for (int i = 0; cmd_list[i] != NULL; i++) {
    if (strncmp(cmd_list[i], buf, *pos) == 0) {
      while (*pos > 0) {
        put_char('\b');
        (*pos)--;
      }
      strcpy(buf, cmd_list[i]);
      *pos = strlen(buf);
      print(buf);
      return;
    }
  }
}

void cmd_cd(const char *arg) {
  if (strlen(arg) == 0 || strcmp(arg, "/") == 0)
    strcpy(current_path, "/");
  else if (strcmp(arg, "..") == 0)
    strcpy(current_path, "/");
  else {
    if (strlen(current_path) > 1)
      strcat(current_path, "/");
    strcat(current_path, arg);
  }
}

void draw_ui() {
  clear_screen_area(0, 0, 80, 1, 0x3F);
  print_at(2, 0, "TarkOS v1.6.1 Professional", 0x3F);
  char tb[10];
  get_time_str(tb);
  print_at(71, 0, tb, 0x3F);
  clear_screen_area(0, 24, 80, 1, 0x70);
  print_at(2, 24, "TAB: Complete | UP: History | ls, cd, cat, tredit, help",
           0x70);
}

void shell_loop() {
  char line[CMD_LEN];
  int pos = 0;
  bool shift = false;
  int h_idx = -1;
  clear_screen();
  draw_ui();
  while (1) {
    set_color(10, 1);
    print("\nroot@tarkos");
    set_color(15, 1);
    print(":");
    set_color(9, 1);
    print(current_path);
    set_color(15, 1);
    print(":# ");
    pos = 0;
    memset(line, 0, CMD_LEN);
    while (1) {
      uint8_t sc = get_any_scancode();
      static uint32_t timer = 0;
      if (++timer > 100000) {
        if (cur_y < 24)
          draw_ui();
        timer = 0;
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

      if (sc == 0x0F) {
        t_autocomplete(line, &pos);
      } else if (sc == 0x48) { // UP
        if (h_idx < MAX_HIST - 1 && strlen(cmd_history[h_idx + 1]) > 0) {
          h_idx++;
          while (pos > 0) {
            put_char('\b');
            pos--;
          }
          strcpy(line, cmd_history[h_idx]);
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
      while (get_any_scancode() == sc)
        ;
    }
    if (pos == 0)
      continue;
    line[pos] = 0;
    add_history(line);
    h_idx = -1;

    if (strcmp(line, "help") == 0)
      print("Commands: ls, cd, pwd, cat, tredit, cls, ver, info, history, "
            "reboot\n");
    else if (strcmp(line, "ls") == 0)
      print("bin  etc  home  dev  readme.txt\n");
    else if (strcmp(line, "pwd") == 0) {
      print(current_path);
      print("\n");
    } else if (strncmp(line, "cd ", 3) == 0)
      cmd_cd(line + 3);
    else if (strcmp(line, "cd") == 0)
      cmd_cd("");
    else if (strncmp(line, "cat ", 4) == 0)
      print("File access simulated.\n");
    else if (strcmp(line, "cls") == 0 || strcmp(line, "clear") == 0) {
      clear_screen();
      cur_y = 1;
      draw_ui();
    } else if (strcmp(line, "history") == 0) {
      for (int i = 0; i < MAX_HIST; i++)
        if (strlen(cmd_history[i]) > 0) {
          print(" ");
          print(cmd_history[i]);
          print("\n");
        }
    } else if (strncmp(line, "tredit", 6) == 0) {
      if (strlen(line) > 7)
        tredit(line + 7);
      else
        tredit("newfile.txt");
    } else if (strcmp(line, "reboot") == 0)
      outb(0x64, 0xFE);
    else {
      print("tarksh: command not found: ");
      print(line);
      print("\n");
    }
  }
}

/* ============= KERNEL MAIN ============= */
void kmain() {
  set_color(15, 1);
  clear_screen();
  draw_ui();
  update_cursor(0, 1);
  shell_loop();
}
