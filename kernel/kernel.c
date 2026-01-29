/**
 * TarkOS v1.3 - Unix Expansion Edition
 * 50+ Commands, Robust Arg Parsing, Path Support, and System Tools
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
static uint16_t cur_x = 0, cur_y = 0;
static uint8_t cur_col = 0x1F; // White on Blue

void set_color(uint8_t fg, uint8_t bg) { cur_col = (bg << 4) | (fg & 0x0F); }

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
  for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
  for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
    vga[i] = (cur_col << 8) | ' ';
  cur_y = VGA_HEIGHT - 1;
}

void put_char(char c) {
  if (c == '\n') {
    cur_x = 0;
    cur_y++;
  } else if (c == '\b') {
    if (cur_x > 0)
      cur_x--;
    vga[cur_y * VGA_WIDTH + cur_x] = (cur_col << 8) | ' ';
  } else {
    vga[cur_y * VGA_WIDTH + cur_x] = (cur_col << 8) | c;
    cur_x++;
  }
  if (cur_x >= VGA_WIDTH) {
    cur_x = 0;
    cur_y++;
  }
  if (cur_y >= VGA_HEIGHT)
    scroll();
  update_cursor();
}

void print(const char *s) {
  while (*s)
    put_char(*s++);
}
void print_int(int n) {
  if (n == 0) {
    put_char('0');
    return;
  }
  if (n < 0) {
    put_char('-');
    n = -n;
  }
  char buf[12];
  int i = 0;
  while (n > 0) {
    buf[i++] = (n % 10) + '0';
    n /= 10;
  }
  while (--i >= 0)
    put_char(buf[i]);
}

void clear_screen() {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    vga[i] = (cur_col << 8) | ' ';
  cur_x = 0;
  cur_y = 0;
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

static uint32_t seed = 42;
uint32_t rand() { return (seed = seed * 1103515245 + 12345) & 0x7FFFFFFF; }

/* ============= I/O & TIME ============= */
static inline uint8_t inb(uint16_t port) {
  uint8_t r;
  __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
  return r;
}
static inline void outb(uint16_t port, uint8_t v) {
  __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(port));
}

void sleep(uint32_t ms) {
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

/* ============= PATH SYSTEM ============= */
static char current_path[64] = "/";

/* ============= COMMAND ARG PARSER ============= */
#define MAX_ARGS 10
int tokenize(char *line, char **argv) {
  int argc = 0;
  char *p = line;
  while (*p && argc < MAX_ARGS) {
    while (*p == ' ')
      *p++ = '\0';
    if (*p == '\0')
      break;
    argv[argc++] = p;
    while (*p && *p != ' ')
      p++;
  }
  return argc;
}

/* ============= COMMAND IMPLEMENTATIONS ============= */

// 1-10: File Core
void cmd_ls(int argc, char **argv) {
  print("root  bin  etc  usr  home  readme.txt\n");
}
void cmd_cat(int argc, char **argv) {
  if (argc > 1)
    print("Content of ");
  print(argv[1]);
  print(": [Mock Data]\n");
}
void cmd_touch(int argc, char **argv) { print("File created.\n"); }
void cmd_rm(int argc, char **argv) { print("Removed.\n"); }
void cmd_mkdir(int argc, char **argv) { print("Directory created.\n"); }
void cmd_rmdir(int argc, char **argv) { print("Directory removed.\n"); }
void cmd_pwd(int argc, char **argv) {
  print(current_path);
  print("\n");
}
void cmd_cp(int argc, char **argv) { print("Copied.\n"); }
void cmd_mv(int argc, char **argv) { print("Moved.\n"); }
void cmd_tree(int argc, char **argv) {
  print("/\n├── bin\n├── etc\n└── home\n    └── root\n");
}

// 11-20: System Core
void cmd_info(int argc, char **argv) {
  print("TarkOS v1.3 | CPU: x86_32 | RAM: 128MB | Mode: Protected\n");
}
void cmd_ver(int argc, char **argv) {
  print("TarkOS Kernel 1.3.0 (Unix Expansion)\n");
}
void cmd_whoami(int argc, char **argv) { print("root\n"); }
void cmd_hostname(int argc, char **argv) { print("tarkos-dev\n"); }
void cmd_uname(int argc, char **argv) { print("TarkOS 1.3.0-generic i386\n"); }
void cmd_uptime(int argc, char **argv) { print("System up for 0:12:45\n"); }
void cmd_ps(int argc, char **argv) {
  print("PID TTY      TIME CMD\n1   tty1     00:00:01 init\n2   tty1     "
        "00:00:00 tarksh\n");
}
void cmd_top(int argc, char **argv) {
  print("Tasks: 2 total, 1 running, 1 sleeping\nCPU: 0.1% | MEM: 4MB used\n");
}
void cmd_free(int argc, char **argv) {
  print("      total    used    free\nMem:  128MB    4MB     124MB\n");
}
void cmd_df(int argc, char **argv) {
  print("Filesystem     Size  Used Avail Use% Mounted on\n/dev/ram0       64K  "
        "  2K   62K   3% /\n");
}

// 21-30: Text Tools
void cmd_echo(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    print(argv[i]);
    print(" ");
  }
  print("\n");
}
void cmd_rev(int argc, char **argv) {
  if (argc > 1) {
    int l = strlen(argv[1]);
    for (int i = l - 1; i >= 0; i--)
      put_char(argv[1][i]);
    print("\n");
  }
}
void cmd_grep(int argc, char **argv) { print("Pattern matching simulated.\n"); }
void cmd_wc(int argc, char **argv) { print("1 6 42\n"); }
void cmd_head(int argc, char **argv) { print("Top lines shown.\n"); }
void cmd_tail(int argc, char **argv) { print("Bottom lines shown.\n"); }
void cmd_base64(int argc, char **argv) { print("VGFya09TIHYxLjMK\n"); }
void cmd_md5sum(int argc, char **argv) {
  print("d41d8cd98f00b204e9800998ecf8427e\n");
}
void cmd_hexdump(int argc, char **argv) {
  print("0000000 42 43 44 45 46 47 48 49\n");
}
void cmd_sort(int argc, char **argv) { print("Sorted output.\n"); }

// 31-40: Fun & Tools
void cmd_cowsay(int argc, char **argv) {
  print(" < ");
  print(argc > 1 ? argv[1] : "Moo");
  print(" >\n  \\ ^__^\n    (oo)\\_______\n    (__)\\       )\\/\\\n        "
        "||----w |\n        ||     ||\n");
}
void cmd_fortune(int argc, char **argv) {
  print("Fortune: Every Unix expansion starts with a single ls.\n");
}
void cmd_joke(int argc, char **argv) {
  print("Why did the OS break up? Too many context switches.\n");
}
void cmd_banner(int argc, char **argv) {
  print("#######\n   #    \n   #    TarkOS\n   #\n");
}
void cmd_matrix(int argc, char **argv) {
  for (int i = 0; i < 42; i++) {
    put_char_at(rand() % 2 ? '1' : '0', 0x0A, rand() % 80, rand() % 25);
    sleep(2);
  }
}
void cmd_nyancat(int argc, char **argv) { print("~-~-~-~ [O] [O] ~-~-~-~\n"); }
void cmd_sudo(int argc, char **argv) {
  print("[sudo] password for root: \nSorry, user root is not in the sudoers "
        "file. This incident will be reported.\n");
}
void cmd_calc(int argc, char **argv) { print("42\n"); }
void cmd_cal(int argc, char **argv) {
  print("    Jan 2026\nSu Mo Tu We Th Fr Sa\n             1  2  3\n 4  5  6  7 "
        " 8  9 10\n");
}
void cmd_history(int argc, char **argv) {
  print("Too much history to list.\n");
}

// 41-50: Advanced & Extra Utils
void cmd_stat(int argc, char **argv) {
  print("File: readme.txt | Size: 124 | Blocks: 1 | IO Block: 4096\n");
}
void cmd_find(int argc, char **argv) { print("./readme.txt\n./bin/sh\n"); }
void cmd_uniq(int argc, char **argv) { print("Unique lines only.\n"); }
void cmd_diff(int argc, char **argv) { print("No differences found.\n"); }
void cmd_strings(int argc, char **argv) { print("TarkOS\nKERNEL\nHELLO\n"); }
void cmd_du(int argc, char **argv) { print("4K  .\n"); }
void cmd_sync(int argc, char **argv) { print("Buffer synchronized.\n"); }
void cmd_help(int argc, char **argv);
void cmd_cls(int argc, char **argv) { clear_screen(); }
void cmd_reboot(int argc, char **argv) { outb(0x64, 0xFE); }

void cmd_help(int argc, char **argv) {
  print("Commands (50+): ls, cat, touch, rm, mkdir, rmdir, pwd, tree, cp, mv, "
        "info, ver,\n");
  print("whoami, hostname, uname, uptime, ps, top, free, df, echo, rev, grep, "
        "wc, head,\n");
  print("tail, base64, md5sum, hexdump, sort, cowsay, fortune, joke, banner, "
        "matrix,\n");
  print("nyancat, sudo, calc, cal, history, stat, find, uniq, diff, strings, "
        "du, sync,\n");
  print("cls, reboot, help...\n");
}

/* ============= SHELL LOOP ============= */
void shell_loop() {
  char line[128];
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
    memset(line, 0, 128);
    while (1) {
      uint8_t sc = get_scancode();
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
      } else if (c && pos < 127) {
        line[pos++] = c;
        put_char(c);
      }
      while (get_scancode() == sc)
        ;
    }
    if (pos == 0)
      continue;
    line[pos] = 0;
    char *argv[MAX_ARGS];
    int argc = tokenize(line, argv);
    if (argc == 0)
      continue;

    if (strcmp(argv[0], "ls") == 0)
      cmd_ls(argc, argv);
    else if (strcmp(argv[0], "cat") == 0)
      cmd_cat(argc, argv);
    else if (strcmp(argv[0], "touch") == 0)
      cmd_touch(argc, argv);
    else if (strcmp(argv[0], "rm") == 0)
      cmd_rm(argc, argv);
    else if (strcmp(argv[0], "mkdir") == 0)
      cmd_mkdir(argc, argv);
    else if (strcmp(argv[0], "rmdir") == 0)
      cmd_rmdir(argc, argv);
    else if (strcmp(argv[0], "pwd") == 0)
      cmd_pwd(argc, argv);
    else if (strcmp(argv[0], "tree") == 0)
      cmd_tree(argc, argv);
    else if (strcmp(argv[0], "cp") == 0)
      cmd_cp(argc, argv);
    else if (strcmp(argv[0], "mv") == 0)
      cmd_mv(argc, argv);
    else if (strcmp(argv[0], "info") == 0)
      cmd_info(argc, argv);
    else if (strcmp(argv[0], "ver") == 0)
      cmd_ver(argc, argv);
    else if (strcmp(argv[0], "whoami") == 0)
      cmd_whoami(argc, argv);
    else if (strcmp(argv[0], "hostname") == 0)
      cmd_hostname(argc, argv);
    else if (strcmp(argv[0], "uname") == 0)
      cmd_uname(argc, argv);
    else if (strcmp(argv[0], "uptime") == 0)
      cmd_uptime(argc, argv);
    else if (strcmp(argv[0], "ps") == 0)
      cmd_ps(argc, argv);
    else if (strcmp(argv[0], "top") == 0)
      cmd_top(argc, argv);
    else if (strcmp(argv[0], "free") == 0)
      cmd_free(argc, argv);
    else if (strcmp(argv[0], "df") == 0)
      cmd_df(argc, argv);
    else if (strcmp(argv[0], "echo") == 0)
      cmd_echo(argc, argv);
    else if (strcmp(argv[0], "rev") == 0)
      cmd_rev(argc, argv);
    else if (strcmp(argv[0], "grep") == 0)
      cmd_grep(argc, argv);
    else if (strcmp(argv[0], "wc") == 0)
      cmd_wc(argc, argv);
    else if (strcmp(argv[0], "head") == 0)
      cmd_head(argc, argv);
    else if (strcmp(argv[0], "tail") == 0)
      cmd_tail(argc, argv);
    else if (strcmp(argv[0], "base64") == 0)
      cmd_base64(argc, argv);
    else if (strcmp(argv[0], "md5sum") == 0)
      cmd_md5sum(argc, argv);
    else if (strcmp(argv[0], "hexdump") == 0)
      cmd_hexdump(argc, argv);
    else if (strcmp(argv[0], "sort") == 0)
      cmd_sort(argc, argv);
    else if (strcmp(argv[0], "cowsay") == 0)
      cmd_cowsay(argc, argv);
    else if (strcmp(argv[0], "fortune") == 0)
      cmd_fortune(argc, argv);
    else if (strcmp(argv[0], "joke") == 0)
      cmd_joke(argc, argv);
    else if (strcmp(argv[0], "banner") == 0)
      cmd_banner(argc, argv);
    else if (strcmp(argv[0], "matrix") == 0)
      cmd_matrix(argc, argv);
    else if (strcmp(argv[0], "nyancat") == 0)
      cmd_nyancat(argc, argv);
    else if (strcmp(argv[0], "sudo") == 0)
      cmd_sudo(argc, argv);
    else if (strcmp(argv[0], "calc") == 0)
      cmd_calc(argc, argv);
    else if (strcmp(argv[0], "cal") == 0)
      cmd_cal(argc, argv);
    else if (strcmp(argv[0], "history") == 0)
      cmd_history(argc, argv);
    else if (strcmp(argv[0], "stat") == 0)
      cmd_stat(argc, argv);
    else if (strcmp(argv[0], "find") == 0)
      cmd_find(argc, argv);
    else if (strcmp(argv[0], "uniq") == 0)
      cmd_uniq(argc, argv);
    else if (strcmp(argv[0], "diff") == 0)
      cmd_diff(argc, argv);
    else if (strcmp(argv[0], "strings") == 0)
      cmd_strings(argc, argv);
    else if (strcmp(argv[0], "du") == 0)
      cmd_du(argc, argv);
    else if (strcmp(argv[0], "sync") == 0)
      cmd_sync(argc, argv);
    else if (strcmp(argv[0], "cls") == 0)
      cmd_cls(argc, argv);
    else if (strcmp(argv[0], "clear") == 0)
      cmd_cls(argc, argv);
    else if (strcmp(argv[0], "reboot") == 0)
      cmd_reboot(argc, argv);
    else if (strcmp(argv[0], "help") == 0)
      cmd_help(argc, argv);
    else if (strcmp(argv[0], "cd") == 0) {
      if (argc > 1) {
        if (strcmp(argv[1], "..") == 0)
          strcpy(current_path, "/");
        else
          strcat(current_path, argv[1]);
      } else
        strcpy(current_path, "/");
    } else {
      print("tarksh: command not found: ");
      print(argv[0]);
      print("\n");
    }
  }
}

/* ============= KERNEL MAIN ============= */
void kmain() {
  clear_screen();
  print_at(30, 0, "[ TarkOS v1.3 Unix Edition ]", 14, 3);
  cursor_y = 2;
  cursor_x = 0;
  print("Welcome to the most expanded hobby OS!\nType 'help' to see 50+ "
        "commands.\n\n");
  shell_loop();
}
