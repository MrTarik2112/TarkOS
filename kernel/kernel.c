/**
 * TarkOS v0.4 - Advanced Features
 * File System, Text Editor, Snake Game, System Monitor
 */

/* ============= TYPES ============= */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef int bool;
#define true 1
#define false 0
#define NULL 0

/* ============= VGA DRIVER ============= */
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum VGA_COLOR {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  BROWN,
  LIGHT_GRAY,
  DARK_GRAY,
  LIGHT_BLUE,
  LIGHT_GREEN,
  LIGHT_CYAN,
  LIGHT_RED,
  LIGHT_MAGENTA,
  YELLOW,
  WHITE
};

static uint16_t *vga = (uint16_t *)VGA_MEMORY;
static int cursor_x = 0, cursor_y = 0;
static uint8_t fg_color = WHITE, bg_color = BLUE;

#define VGA_ENTRY(c, fg, bg) ((uint16_t)((c) | ((fg) << 8) | ((bg) << 12)))

void set_color(uint8_t fg, uint8_t bg) {
  fg_color = fg;
  bg_color = bg;
}

void put_char_at(int x, int y, char c, uint8_t fg, uint8_t bg) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    vga[y * VGA_WIDTH + x] = VGA_ENTRY(c, fg, bg);
}

void clear_screen(void) {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = VGA_ENTRY(' ', fg_color, bg_color);
  cursor_x = 0;
  cursor_y = 0;
}

void scroll(void) {
  for (int y = 1; y < VGA_HEIGHT - 1; y++)
    for (int x = 0; x < VGA_WIDTH; x++)
      vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
  for (int x = 0; x < VGA_WIDTH; x++)
    vga[(VGA_HEIGHT - 2) * VGA_WIDTH + x] = VGA_ENTRY(' ', fg_color, bg_color);
}

void put_char(char c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
  } else if (c == '\b') {
    if (cursor_x > 0) {
      cursor_x--;
      put_char_at(cursor_x, cursor_y, ' ', fg_color, bg_color);
    }
  } else if (c == '\t') {
    cursor_x = (cursor_x + 4) & ~3;
  } else {
    put_char_at(cursor_x, cursor_y, c, fg_color, bg_color);
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
void print_at(int x, int y, const char *s, uint8_t fg, uint8_t bg) {
  while (*s)
    put_char_at(x++, y, *s++, fg, bg);
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

/* ============= I/O PORTS ============= */
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ============= TIMER ============= */
static volatile uint32_t ticks = 0;
uint32_t get_ticks(void) { return ticks; }
void sleep(uint32_t ms) {
  volatile uint32_t end = ticks + (ms / 10);
  while (ticks < end)
    ;
}

/* ============= KEYBOARD ============= */
static const char scancode_lower[] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

static const char scancode_upper[] = {
    0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
    'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' '};

static bool shift_pressed = false;

char get_key(void) {
  while (!(inb(0x64) & 1)) {
    ticks++;
  }
  uint8_t sc = inb(0x60);
  if (sc == 0x2A || sc == 0x36) {
    shift_pressed = true;
    return 0;
  }
  if (sc == 0xAA || sc == 0xB6) {
    shift_pressed = false;
    return 0;
  }
  if (sc & 0x80)
    return 0;
  if (sc < sizeof(scancode_lower))
    return shift_pressed ? scancode_upper[sc] : scancode_lower[sc];
  return 0;
}

/* ============= STRING FUNCTIONS ============= */
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
int atoi(const char *s) {
  int n = 0, neg = 0;
  if (*s == '-') {
    neg = 1;
    s++;
  }
  while (*s >= '0' && *s <= '9')
    n = n * 10 + (*s++ - '0');
  return neg ? -n : n;
}

/* ============= RANDOM ============= */
static uint32_t rand_seed = 12345;
uint32_t rand(void) {
  rand_seed = rand_seed * 1103515245 + 12345;
  return (rand_seed >> 16) & 0x7FFF;
}

/* ============= FILE SYSTEM (RAM DISK) ============= */
#define MAX_FILES 16
#define MAX_FILENAME 16
#define MAX_FILESIZE 1024

typedef struct {
  char name[MAX_FILENAME];
  char content[MAX_FILESIZE];
  int size;
  bool used;
} file_t;

static file_t files[MAX_FILES];

void fs_init(void) {
  for (int i = 0; i < MAX_FILES; i++)
    files[i].used = false;

  // Create sample files
  strcpy(files[0].name, "readme.txt");
  strcpy(files[0].content,
         "Welcome to TarkOS v0.4!\n\nThis is a simple RAM disk file "
         "system.\nFiles are stored in memory.\n\nTry these commands:\n  ls - "
         "list files\n  cat readme.txt - view file\n  edit myfile.txt - "
         "create/edit file\n");
  files[0].size = strlen(files[0].content);
  files[0].used = true;
}

int fs_create(const char *name) {
  for (int i = 0; i < MAX_FILES; i++) {
    if (!files[i].used) {
      strcpy(files[i].name, name);
      files[i].size = 0;
      files[i].content[0] = 0;
      files[i].used = true;
      return i;
    }
  }
  return -1;
}

int fs_find(const char *name) {
  for (int i = 0; i < MAX_FILES; i++) {
    if (files[i].used && strcmp(files[i].name, name) == 0)
      return i;
  }
  return -1;
}

/* ============= SNAKE GAME ============= */
#define SNAKE_MAX 100
typedef struct {
  int x, y;
} point_t;
static point_t snake[SNAKE_MAX];
static int snake_len = 3;
static point_t food;
static int dx = 1, dy = 0;
static int score = 0;

void snake_game(void) {
  // Initialize
  snake_len = 3;
  for (int i = 0; i < snake_len; i++) {
    snake[i].x = 10 - i;
    snake[i].y = 10;
  }
  food.x = 20;
  food.y = 15;
  score = 0;
  dx = 1;
  dy = 0;

  set_color(WHITE, BLACK);
  clear_screen();
  print_at(2, 0, "SNAKE GAME - WASD to move, Q to quit", YELLOW, BLACK);

  // Draw border
  for (int x = 0; x < 40; x++) {
    put_char_at(x, 2, '#', WHITE, BLACK);
    put_char_at(x, 22, '#', WHITE, BLACK);
  }
  for (int y = 2; y < 23; y++) {
    put_char_at(0, y, '#', WHITE, BLACK);
    put_char_at(39, y, '#', WHITE, BLACK);
  }

  while (1) {
    // Draw
    put_char_at(food.x, food.y, '*', RED, BLACK);
    for (int i = 0; i < snake_len; i++)
      put_char_at(snake[i].x, snake[i].y, i == 0 ? '@' : 'o', GREEN, BLACK);

    // Score
    char buf[20];
    buf[0] = 'S';
    buf[1] = 'c';
    buf[2] = 'o';
    buf[3] = 'r';
    buf[4] = 'e';
    buf[5] = ':';
    buf[6] = ' ';
    buf[7] = '0' + (score / 10);
    buf[8] = '0' + (score % 10);
    buf[9] = 0;
    print_at(45, 10, buf, YELLOW, BLACK);

    // Input (non-blocking check)
    if (inb(0x64) & 1) {
      char c = get_key();
      if (c == 'w' && dy == 0) {
        dx = 0;
        dy = -1;
      } else if (c == 's' && dy == 0) {
        dx = 0;
        dy = 1;
      } else if (c == 'a' && dx == 0) {
        dx = -1;
        dy = 0;
      } else if (c == 'd' && dx == 0) {
        dx = 1;
        dy = 0;
      } else if (c == 'q')
        break;
    }

    // Erase tail
    put_char_at(snake[snake_len - 1].x, snake[snake_len - 1].y, ' ', BLACK,
                BLACK);

    // Move
    for (int i = snake_len - 1; i > 0; i--)
      snake[i] = snake[i - 1];
    snake[0].x += dx;
    snake[0].y += dy;

    // Check wall collision
    if (snake[0].x <= 0 || snake[0].x >= 39 || snake[0].y <= 2 ||
        snake[0].y >= 22) {
      print_at(10, 12, "GAME OVER! Press any key", RED, BLACK);
      while (!get_key())
        ;
      break;
    }

    // Check self collision
    for (int i = 1; i < snake_len; i++) {
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        print_at(10, 12, "GAME OVER! Press any key", RED, BLACK);
        while (!get_key())
          ;
        goto done;
      }
    }

    // Check food
    if (snake[0].x == food.x && snake[0].y == food.y) {
      score += 10;
      if (snake_len < SNAKE_MAX)
        snake_len++;
      food.x = (rand() % 37) + 2;
      food.y = (rand() % 18) + 3;
    }

    sleep(150);
  }
done:
  set_color(WHITE, BLUE);
}

/* ============= TEXT EDITOR ============= */
void text_editor(const char *filename) {
  static char buffer[MAX_FILESIZE];
  int pos = 0;

  // Load file if exists
  int fid = fs_find(filename);
  if (fid >= 0) {
    strcpy(buffer, files[fid].content);
    pos = files[fid].size;
  } else {
    buffer[0] = 0;
  }

  set_color(WHITE, BLUE);
  clear_screen();
  print_at(0, 0, "EDITOR: ", YELLOW, CYAN);
  print_at(8, 0, filename, WHITE, CYAN);
  print_at(60, 0, "Ctrl-S Save Ctrl-Q Quit", BLACK, CYAN);
  cursor_y = 2;
  cursor_x = 0;

  // Display content
  for (int i = 0; i < pos; i++)
    put_char(buffer[i]);

  bool ctrl = false;
  while (1) {
    char c = get_key();
    if (c == 0)
      continue;

    uint8_t sc = inb(0x60);
    if (sc == 0x1D) {
      ctrl = true;
      continue;
    }
    if (sc == 0x9D) {
      ctrl = false;
      continue;
    }

    if (ctrl && c == 's') {
      // Save
      buffer[pos] = 0;
      fid = fs_find(filename);
      if (fid < 0)
        fid = fs_create(filename);
      if (fid >= 0) {
        strcpy(files[fid].content, buffer);
        files[fid].size = pos;
        print_at(0, VGA_HEIGHT - 1, "Saved!         ", BLACK, LIGHT_GREEN);
        sleep(500);
      }
    } else if (ctrl && c == 'q') {
      break;
    } else if (c == '\b' && pos > 0) {
      pos--;
      put_char('\b');
    } else if (c >= 32 && c < 127 && pos < MAX_FILESIZE - 1) {
      buffer[pos++] = c;
      put_char(c);
    } else if (c == '\n' && pos < MAX_FILESIZE - 1) {
      buffer[pos++] = '\n';
      put_char('\n');
    }
  }
}

/* ============= UI ============= */
void draw_header(void) {
  for (int x = 0; x < VGA_WIDTH; x++)
    put_char_at(x, 0, ' ', WHITE, CYAN);
  print_at(2, 0, "TarkOS v0.4", WHITE, CYAN);
  print_at(60, 0, "Uptime: ", YELLOW, CYAN);
}

void draw_status_bar(void) {
  for (int x = 0; x < VGA_WIDTH; x++)
    put_char_at(x, VGA_HEIGHT - 1, ' ', BLACK, LIGHT_GRAY);
  print_at(2, VGA_HEIGHT - 1, "F1:Help | Type 'help' for all commands", BLACK,
           LIGHT_GRAY);
}

void update_uptime(void) {
  uint32_t secs = ticks / 100;
  int h = secs / 3600, m = (secs / 60) % 60, s = secs % 60;
  char buf[12];
  buf[0] = '0' + h / 10;
  buf[1] = '0' + h % 10;
  buf[2] = ':';
  buf[3] = '0' + m / 10;
  buf[4] = '0' + m % 10;
  buf[5] = ':';
  buf[6] = '0' + s / 10;
  buf[7] = '0' + s % 10;
  buf[8] = 0;
  print_at(68, 0, buf, YELLOW, CYAN);
}

/* ============= COMMANDS ============= */
static char cmd[80];
static char history[10][80];
static int history_count = 0;

void cmd_help(void) {
  set_color(LIGHT_CYAN, BLUE);
  print("\n=== TarkOS v0.4 Commands ===\n");
  set_color(YELLOW, BLUE);
  print("  help  clear  info  about  echo  calc  color  reboot\n");
  print("  ls  cat  edit  rm  game  snake  matrix  art  mem  top\n");
  set_color(WHITE, BLUE);
}

void cmd_ls(void) {
  print("\nFiles:\n");
  int count = 0;
  for (int i = 0; i < MAX_FILES; i++) {
    if (files[i].used) {
      set_color(LIGHT_CYAN, BLUE);
      print("  ");
      print(files[i].name);
      set_color(DARK_GRAY, BLUE);
      print(" (");
      print_int(files[i].size);
      print(" bytes)\n");
      count++;
    }
  }
  set_color(WHITE, BLUE);
  if (count == 0)
    print("  (no files)\n");
  print("Total: ");
  print_int(count);
  print(" files\n");
}

void cmd_cat(const char *arg) {
  int fid = fs_find(arg);
  if (fid < 0) {
    set_color(LIGHT_RED, BLUE);
    print("\nFile not found: ");
    print(arg);
    print("\n");
    set_color(WHITE, BLUE);
  } else {
    print("\n");
    print(files[fid].content);
    print("\n");
  }
}

void cmd_rm(const char *arg) {
  int fid = fs_find(arg);
  if (fid < 0) {
    print("\nFile not found\n");
  } else {
    files[fid].used = false;
    print("\nDeleted: ");
    print(arg);
    print("\n");
  }
}

void cmd_top(void) {
  set_color(WHITE, BLUE);
  clear_screen();
  cursor_y = 1;
  print("=== System Monitor ===\n\n");
  print("Uptime: ");
  print_int(ticks / 100);
  print(" seconds\n");
  print("Ticks:  ");
  print_int(ticks);
  print("\n");
  print("Files:  ");
  int fc = 0;
  for (int i = 0; i < MAX_FILES; i++)
    if (files[i].used)
      fc++;
  print_int(fc);
  print(" / ");
  print_int(MAX_FILES);
  print("\n");
  print("Memory: 128 MB (simulated)\n\n");
  print("Press any key to exit...");
  while (!get_key())
    ;
}

void process_command(void) {
  // Add to history
  if (cmd[0] && history_count < 10) {
    strcpy(history[history_count++], cmd);
  }

  set_color(WHITE, BLUE);
  if (strcmp(cmd, "help") == 0)
    cmd_help();
  else if (strcmp(cmd, "clear") == 0) {
    clear_screen();
    cursor_y = 1;
    draw_header();
    draw_status_bar();
  } else if (strcmp(cmd, "ls") == 0)
    cmd_ls();
  else if (strncmp(cmd, "cat ", 4) == 0)
    cmd_cat(cmd + 4);
  else if (strncmp(cmd, "rm ", 3) == 0)
    cmd_rm(cmd + 3);
  else if (strncmp(cmd, "edit ", 5) == 0) {
    text_editor(cmd + 5);
    clear_screen();
    cursor_y = 1;
    draw_header();
    draw_status_bar();
  } else if (strcmp(cmd, "snake") == 0) {
    snake_game();
    clear_screen();
    cursor_y = 1;
    draw_header();
    draw_status_bar();
  } else if (strcmp(cmd, "top") == 0) {
    cmd_top();
    clear_screen();
    cursor_y = 1;
    draw_header();
    draw_status_bar();
  } else if (strcmp(cmd, "history") == 0) {
    print("\nCommand history:\n");
    for (int i = 0; i < history_count; i++) {
      print("  ");
      print_int(i + 1);
      print(". ");
      print(history[i]);
      print("\n");
    }
  } else if (strcmp(cmd, "reboot") == 0) {
    print("\nRebooting...\n");
    outb(0x64, 0xFE);
    while (1)
      ;
  } else if (cmd[0]) {
    set_color(LIGHT_RED, BLUE);
    print("\nUnknown: ");
    print(cmd);
    print("\n");
  } else
    print("\n");
  set_color(WHITE, BLUE);
}

void show_prompt(void) {
  set_color(LIGHT_GREEN, BLUE);
  print("tarkos");
  set_color(WHITE, BLUE);
  print("@");
  set_color(LIGHT_CYAN, BLUE);
  print("kernel");
  set_color(WHITE, BLUE);
  print(":~$ ");
  set_color(YELLOW, BLUE);
}

/* ============= MAIN ============= */
void kmain(void) {
  fs_init();
  set_color(WHITE, BLUE);
  clear_screen();
  draw_header();
  draw_status_bar();

  cursor_y = 2;
  set_color(LIGHT_GREEN, BLUE);
  print("Welcome to TarkOS v0.4!\n");
  set_color(WHITE, BLUE);
  print("New: File system, Text editor, Snake game!\n");
  print("Type 'help' for commands.\n\n");

  while (1) {
    show_prompt();
    int pos = 0;
    cmd[0] = 0;

    while (1) {
      char c = get_key();
      update_uptime();
      if (c == 0)
        continue;
      if (c == '\n') {
        cmd[pos] = 0;
        process_command();
        break;
      } else if (c == '\b') {
        if (pos > 0) {
          pos--;
          put_char('\b');
        }
      } else if (pos < 78) {
        cmd[pos++] = c;
        put_char(c);
      }
    }
  }
}
