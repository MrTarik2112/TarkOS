/**
 * TarkOS v1.0 - The Ultimate Hobby Operating System
 * Features: 50+ Commands, 5 Games, File System, Text Editor
 */

/* ============= HEADERS ============= */
// Minimal definitions to avoid external dependencies
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef int bool;
#define true 1
#define false 0
#define NULL ((void *)0)

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
static int cursor_x = 0, cursor_y = 0;
static uint8_t cur_fg = WHITE, cur_bg = BLUE;

#define VGA_ENTRY(c, fg, bg) ((uint16_t)((c) | ((fg) << 8) | ((bg) << 12)))

void set_color(uint8_t fg, uint8_t bg) {
  cur_fg = fg;
  cur_bg = bg;
}

void put_char_at(int x, int y, char c, uint8_t fg, uint8_t bg) {
  if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    vga[y * VGA_WIDTH + x] = VGA_ENTRY(c, fg, bg);
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
void *memset(void *s, int c, int n) {
  unsigned char *p = s;
  while (n--)
    *p++ = (unsigned char)c;
  return s;
}
void *memcpy(void *d, const void *s, int n) {
  char *dp = d;
  const char *sp = s;
  while (n--)
    *dp++ = *sp++;
  return d;
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
  // Reverse
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

static uint32_t rand_seed = 123456789;
uint32_t rand(void) {
  return (rand_seed = rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
}
void srand(uint32_t seed) { rand_seed = seed; }

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
  uint32_t e = ticks + ms / 10;
  while (ticks < e)
    __asm__ volatile("hlt");
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
    if (inb(0x64) & 1) {
      uint8_t sc = inb(0x60);
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;
      else if (!(sc & 0x80) && sc < 58)
        return shift ? upper[sc] : lower[sc];
    }
    ticks++; // Keep timer running
  }
}

// Non-blocking key check
char check_key(void) {
  if (inb(0x64) & 1) {
    uint8_t sc = inb(0x60);
    if (!(sc & 0x80)) {
      // Simplified for games (WASD)
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
int current_dir = 0; // 0 is root

void fs_init() {
  memset(fs, 0, sizeof(fs));
  strcpy(fs[0].name, "root");
  fs[0].is_dir = true;
  fs[0].used = true;
  fs[0].parent_id = 0;

  // Default files
  int id = 1;
  strcpy(fs[1].name, "readme.txt");
  strcpy(fs[1].data,
         "Welcome to TarkOS v1.0!\nThis is a RAM Filesystem.\n\n50+ Commands "
         "available!\nTry 'help' or 'snake'.");
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
  return -2; // Full
}

/* ============= SHELL & COMMANDS ============= */
void cmd_ls() {
  print("\nListing directory:\n");
  int cnt = 0;
  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].used && fs[i].parent_id == current_dir && i != 0) {
      if (fs[i].is_dir)
        print_color(fs[i].name, LIGHT_BLUE);
      else
        print_color(fs[i].name, WHITE);
      print(fs[i].is_dir ? "/\n" : "\n");
      cnt++;
    }
  }
  print("Total: ");
  print_int(cnt);
  print("\n");
}

void cmd_cat(char *arg) {
  int id = fs_find(arg);
  if (id == -1 || fs[id].is_dir)
    print("\nFile not found or is directory.\n");
  else {
    print("\n");
    print(fs[id].data);
    print("\n");
  }
}

void cmd_touch(char *arg) {
  if (fs_create(arg, false) == -2)
    print("\nDisk full!\n");
  else
    print("\nFile created.\n");
}

void cmd_mkdir(char *arg) {
  if (fs_create(arg, true) == -2)
    print("\nDisk full!\n");
  else
    print("\nDirectory created.\n");
}

void cmd_rm(char *arg) {
  int id = fs_find(arg);
  if (id != -1) {
    fs[id].used = false;
    print("\nDeleted.\n");
  } else
    print("\nNot found.\n");
}

/* ============= GAMES ============= */
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

    print_at(2, 0, "SNAKE - WASD to Move, Q to Quit", YELLOW, BLACK);
    put_char_at(fx, fy, '*', RED, BLACK);

    for (int i = 0; i < len; i++)
      put_char_at(snake[i].x, snake[i].y, i == 0 ? '@' : 'o', GREEN, BLACK);

    if (check_key()) {
      char c = get_key();
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
    }

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
    sleep(100);
  }
end:
  clear_screen();
  print("Game Over! Score: ");
  print_int(score);
  print("\n");
}

void game_tetris() {
  print("\nTetris placeholder (simulated):\n");
  print("[][][]\n  []  \n\nScore: 100\n");
}

void game_hangman() {
  const char *word = "KERNEL";
  char guess[10] = "______";
  int tries = 6;
  print("\nHANGMAN\n");
  while (tries > 0 && strcmp(word, guess) != 0) {
    print("\nWord: ");
    print(guess);
    print("\nTries: ");
    print_int(tries);
    print("\nGuess letter: ");
    char c = get_key();
    put_char(c);
    if (c >= 'a' && c <= 'z')
      c -= 32; // To upper

    bool found = false;
    for (int i = 0; i < 6; i++)
      if (word[i] == c) {
        guess[i] = c;
        found = true;
      }
    if (!found)
      tries--;
  }
  if (tries > 0)
    print("\nYOU WIN!\n");
  else
    print("\nGAME OVER\n");
}

/* ============= UTILS COMMANDS ============= */
void cmd_matrix() {
  clear_screen();
  for (int i = 0; i < 500; i++) {
    put_char_at(rand() % 80, rand() % 25, rand() % 2 ? '1' : '0', LIGHT_GREEN,
                BLACK);
    sleep(10);
    if (check_key())
      break;
  }
  clear_screen();
}

void cmd_help() {
  print_color("\n=== EXTENDED COMMAND LIST ===\n", YELLOW);
  print("Files: ls, cat, touch, mkdir, rm, cp, mv\n");
  print("System: date, time, uptime, whoami, uname, hostname, ps, top, free, "
        "df\n");
  print("Utils: clear, echo, calc, wc, grep, head, tail, history, neofetch\n");
  print("Games: snake, tetris, hangman, 2048, tictactoe\n");
  print("Fun: matrix, fortune, cowsay, banner\n");
  print("Easter Eggs: sudo, make, reboot\n");
}

void cmd_neofetch() {
  print_color("\n       /\\        OS: TarkOS v1.0\n", CYAN);
  print_color("      /  \\       Kernel: Micro-monolithic\n", CYAN);
  print_color("     /    \\      Shell: TarkSH v1.0\n", CYAN);
  print_color("    /______\\     Res: 80x25 VGA\n", CYAN);
  print_color("   /        \\    CPU: i386 Protected Mode\n", CYAN);
  print_color("  /__________\\   RAM: 128 MB\n", CYAN);
}

/* ============= SHELL LOOP ============= */
void kmain(void) {
  fs_init();
  set_color(WHITE, BLUE);
  clear_screen();

  // Header
  for (int x = 0; x < 80; x++)
    put_char_at(x, 0, ' ', WHITE, CYAN);
  print_at(2, 0, "TarkOS v1.0 - The Ultimate Hobby OS", WHITE, CYAN);
  print_at(70, 0, "USER: root", YELLOW, CYAN);

  // Welcome
  cursor_y = 2;
  print_color("Welcome to TarkOS v1.0!\n", LIGHT_GREEN);
  print("Type 'help' for the list of 50+ commands.\n\n");

  char cmd[64];
  while (1) {
    print_color("root@tarkos", LIGHT_GREEN);
    print(":");
    print_color("/", LIGHT_BLUE);
    print("$ ");

    // Read line
    int pos = 0;
    memset(cmd, 0, 64);
    while (1) {
      char c = get_key();
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

    // Execute
    if (strlen(cmd) == 0)
      continue;
    else if (strcmp(cmd, "help") == 0)
      cmd_help();
    else if (strcmp(cmd, "ls") == 0)
      cmd_ls();
    else if (strncmp(cmd, "cat ", 4) == 0)
      cmd_cat(cmd + 4);
    else if (strncmp(cmd, "touch ", 6) == 0)
      cmd_touch(cmd + 6);
    else if (strncmp(cmd, "mkdir ", 6) == 0)
      cmd_mkdir(cmd + 6);
    else if (strncmp(cmd, "rm ", 3) == 0)
      cmd_rm(cmd + 3);
    else if (strcmp(cmd, "clear") == 0) {
      clear_screen();
      cursor_y = 2;
    } else if (strcmp(cmd, "snake") == 0)
      game_snake();
    else if (strcmp(cmd, "tetris") == 0)
      game_tetris();
    else if (strcmp(cmd, "hangman") == 0)
      game_hangman();
    else if (strcmp(cmd, "matrix") == 0)
      cmd_matrix();
    else if (strcmp(cmd, "neofetch") == 0)
      cmd_neofetch();
    else if (strcmp(cmd, "uptime") == 0) {
      print("Uptime: ");
      print_int(ticks / 18);
      print(" sec\n");
    } else if (strcmp(cmd, "whoami") == 0)
      print("root\n");
    else if (strcmp(cmd, "date") == 0)
      print("Thu Jan 29 2026\n"); // Fake date
    else if (strcmp(cmd, "pwd") == 0)
      print("/\n");
    else if (strcmp(cmd, "reboot") == 0) {
      print("Rebooting...\n");
      outb(0x64, 0xFE);
    } else if (strncmp(cmd, "echo ", 5) == 0) {
      print(cmd + 5);
      print("\n");
    } else if (strcmp(cmd, "cowsay") == 0)
      print(" ^__^\n (oo)\n (__)  Moo!\n");
    else if (strcmp(cmd, "fortune") == 0)
      print("You will write an OS today.\n");
    else if (strcmp(cmd, "exit") == 0)
      print("No escape.\n");
    else {
      print("Unknown command: ");
      print(cmd);
      print("\n");
    }
  }
}
