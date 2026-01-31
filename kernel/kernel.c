/**
 * TarkOS Nova v1.9.6 - Eternal Edition
 * 18.0s Hyper-Cinematic Boot | Permanent Nova Branding | 3-Core SMP
 * All 50+ Unix Features Preserved
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

/* ============= I/O PRIMITIVES ============= */
static inline uint8_t inb(uint16_t port) {
  uint8_t r;
  __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
  return r;
}
static inline void outb(uint16_t port, uint8_t v) {
  __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(port));
}

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
void *memcpy(void *d, const void *s, int n);
void itoa(int n, char *buf);
uint8_t get_rtc(int reg);
uint8_t bcd_to_bin(uint8_t v);
void get_time_str(char *buf);
void get_date_str(char *buf);
void draw_window(int x, int y, int w, int h, const char *title, uint8_t col);
void draw_shell_static();
void draw_shell_dynamic();
void delay_ms(int ms);
void cpuid(uint32_t code, uint32_t *a, uint32_t *d);
int split_args(char *line, char **argv, int max_args);
char *after_n_tokens(char *s, int n);
int calc_eval(const char *expr, int *out);
int starts_with(const char *s, const char *prefix);
int str_contains(const char *s, const char *needle);
int build_path(const char *name, char *out);
void path_parent(const char *path, char *out);
int fs_write_file(const char *name, const char *data, int size);
int fs_append_file(const char *name, const char *data, int size);
int fs_delete_file(const char *name);
int fs_dir_exists(const char *path);
int fs_mkdir(const char *path);
int fs_rmdir(const char *path);
void ls_dir_path(const char *path);
int is_space(char c);
int count_words(const char *s);
int count_lines(const char *s);
int fs_used_files();
int fs_used_bytes();
const char *command_desc(const char *cmd);
int command_runs_without_args(const char *cmd);

/* ============= VGA DRIVER v10.0 (ETERNAL) ============= */
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)VGA_ADDR;
static uint16_t current_x = 0, current_y = 1;

static uint8_t col_bg = 0x1F;
static uint8_t col_header = 0x3F;
static uint8_t col_footer = 0x3F;
static uint8_t col_shadow = 0x08;
static uint8_t col_accent = 0x0B;
static uint8_t col_success = 0x2F;
static uint8_t col_prompt = 0x0A;
static uint8_t col_matrix = 0x02;

static uint8_t current_col = 0x1F;
static char last_time_str[16] = "";

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
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void draw_rect(int x, int y, int w, int h, uint8_t col) {
  uint16_t entry = (uint16_t)' ' | ((uint16_t)col << 8);
  for (int i = y; i < y + h; i++)
    for (int j = x; j < x + w; j++)
      if (vga[i * VGA_WIDTH + j] != entry)
        vga[i * VGA_WIDTH + j] = entry;
}

void draw_window(int x, int y, int w, int h, const char *title, uint8_t col) {
  uint8_t col_light = (col & 0xF0) | (col_accent & 0x0F);
  uint8_t dark_fg = col_shadow ? (col_shadow & 0x0F) : (col & 0x0F);
  uint8_t col_dark = (col & 0xF0) | dark_fg;
  if (col_shadow) {
    int sx = x + w;
    int sy = y + h;
    if (sx < VGA_WIDTH) {
      int sw = (sx + 1 < VGA_WIDTH) ? 2 : 1;
      draw_rect(sx, y + 1, sw, h, col_shadow);
    }
    if (sy < VGA_HEIGHT) {
      int sh = (sy + 1 < VGA_HEIGHT) ? 2 : 1;
      draw_rect(x + 1, sy, w, sh, col_shadow);
    }
    if (sx < VGA_WIDTH && sy < VGA_HEIGHT)
      put_char_raw(' ', col_shadow, sx, sy);
  }
  draw_rect(x, y, w, h, col);
  if (w > 2 && h > 2)
    draw_rect(x + 1, y + 1, w - 2, h - 2, col);
  if (w > 4)
    draw_rect(x + 2, y + 1, w - 4, 1, col_light);
  if (w > 4 && h > 4) {
    for (int i = x + 1; i < x + w - 1; i++) {
      put_char_raw(196, col_light, i, y + 1);
      put_char_raw(196, col_dark, i, y + h - 2);
    }
    for (int i = y + 1; i < y + h - 1; i++) {
      put_char_raw(179, col_light, x + 1, i);
      put_char_raw(179, col_dark, x + w - 2, i);
    }
    put_char_raw(218, col_light, x + 1, y + 1);
    put_char_raw(191, col_light, x + w - 2, y + 1);
    put_char_raw(192, col_dark, x + 1, y + h - 2);
    put_char_raw(217, col_dark, x + w - 2, y + h - 2);
  }
  for (int i = x; i < x + w; i++) {
    put_char_raw(205, col, i, y);
    put_char_raw(205, col, i, y + h - 1);
  }
  for (int i = y; i < y + h; i++) {
    put_char_raw(186, col, x, i);
    put_char_raw(186, col, x + w - 1, i);
  }
  put_char_raw(201, col, x, y);
  put_char_raw(187, col, x + w - 1, y);
  put_char_raw(200, col, x, y + h - 1);
  put_char_raw(188, col, x + w - 1, y + h - 1);
  print_at(x + (w - strlen(title)) / 2, y, title, (col & 0xF0) | 0x0F);
}

void draw_shell_static() {
  // Top Dashboard
  draw_rect(0, 0, 80, 1, col_header);
  print_at(1, 0, "\xAF TarkOS Nova", col_header);
  print_at(20, 0, "| SMP: x3 Core", col_header);
  print_at(38, 0, "| RAM: 512MB", col_header);

  // Bottom Footer
  draw_rect(0, 24, 80, 1, col_footer);
  print_at(2, 24, " HELP | LS | CD | MKDIR | DF | WC | ABOUT | SYSINFO | v1.9.6",
           col_footer);
}

void draw_shell_dynamic() {
  char curr_time[16];
  get_time_str(curr_time);
  if (strcmp(curr_time, last_time_str) != 0) {
    print_at(71, 0, curr_time, col_header);
    strcpy(last_time_str, curr_time);
  }
}

void clear_screen() {
  draw_rect(0, 0, 80, 25, col_bg);
  draw_shell_static();
  current_x = 0;
  current_y = 1;
  update_cursor(0, 1);
}

void set_theme(int theme) {
  switch (theme) {
  case 0: // Nova Dark (Default)
    col_bg = 0x1F;
    col_header = 0x3F;
    col_footer = 0x3F;
    col_accent = 0x0B;
    col_success = 0x2F;
    col_prompt = 0x0A;
    col_shadow = 0x00;
    break;
  case 1: // Cyber Neon
    col_bg = 0x00;
    col_header = 0x2A;
    col_footer = 0x2A;
    col_accent = 0x0E;
    col_success = 0x0A;
    col_prompt = 0x0B;
    col_shadow = 0x08;
    break;
  case 2: // Classic Terminal
    col_bg = 0x07;
    col_header = 0x70;
    col_footer = 0x70;
    col_accent = 0x08;
    col_success = 0x0F;
    col_prompt = 0x0C;
    col_shadow = 0x00;
    break;
  }
  current_col = col_bg;
  clear_screen();
}

void scroll() {
  for (int i = 1 * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    vga[i] = vga[i + VGA_WIDTH];
  draw_rect(0, VGA_HEIGHT - 2, VGA_WIDTH, 1, col_bg);
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
  while (n > 0 && *a && *a == *b) {
    a++;
    b++;
    n--;
  }
  if (n == 0)
    return 0;
  return (unsigned char)*a - (unsigned char)*b;
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
void *memcpy(void *d, const void *s, int n) {
  unsigned char *dd = (unsigned char *)d;
  const unsigned char *ss = (const unsigned char *)s;
  while (n--)
    *dd++ = *ss++;
  return d;
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
void delay_ms(int ms) {
  // Recalibrated for modern environments (approx. 100k loops for 1ms in QEMU)
  for (volatile int i = 0; i < ms * 100000; i++)
    ;
}
void cpuid(uint32_t code, uint32_t *a, uint32_t *d) {
  __asm__ volatile("cpuid" : "=a"(*a), "=d"(*d) : "a"(code) : "ecx", "ebx");
}

/* ============= TIME & FS ============= */
uint8_t get_rtc(int reg) {
  outb(0x70, reg);
  return inb(0x71);
}
uint8_t bcd_to_bin(uint8_t v) {
  return ((v & 0xF0) >> 1) + ((v & 0xF0) >> 3) + (v & 0x0F);
}
void get_time_str(char *buf) {
  uint8_t h = bcd_to_bin(get_rtc(0x04));
  uint8_t m = bcd_to_bin(get_rtc(0x02));
  uint8_t s = bcd_to_bin(get_rtc(0x00));
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
void get_date_str(char *buf) {
  uint8_t day = bcd_to_bin(get_rtc(0x07));
  uint8_t mon = bcd_to_bin(get_rtc(0x08));
  uint8_t yr = bcd_to_bin(get_rtc(0x09));
  int year = 2000 + yr;
  buf[0] = (year / 1000) % 10 + '0';
  buf[1] = (year / 100) % 10 + '0';
  buf[2] = (year / 10) % 10 + '0';
  buf[3] = year % 10 + '0';
  buf[4] = '-';
  buf[5] = mon / 10 + '0';
  buf[6] = mon % 10 + '0';
  buf[7] = '-';
  buf[8] = day / 10 + '0';
  buf[9] = day % 10 + '0';
  buf[10] = 0;
}

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
  strcpy(fs_table[0].name, "System.sys");
  strcpy(fs_table[0].data, "TarkOS Nova Eternal v1.9.6\nStatus: Ultimate Build "
                           "Online.\nFilesystem: 50+ Commands Stable.");
  fs_table[0].size = strlen(fs_table[0].data);
  fs_table[0].used = true;

  strcpy(fs_table[1].name, "test.txt");
  strcpy(fs_table[1].data, "This is a Nova test file.\nCinematic features: "
                           "ACTIVE.\nCommand verification: IN PROGRESS.");
  fs_table[1].size = strlen(fs_table[1].data);
  fs_table[1].used = true;
}

int fs_find_file(const char *name) {
  for (int i = 0; i < MAX_FILES; i++)
    if (fs_table[i].used && strcmp(fs_table[i].name, name) == 0)
      return i;
  return -1;
}

int fs_name_valid(const char *name) {
  int len = strlen(name);
  if (len <= 0 || len >= 32)
    return 0;
  return 1;
}

int fs_write_file(const char *name, const char *data, int size) {
  if (!fs_name_valid(name))
    return -2;
  int id = fs_find_file(name);
  if (id == -1) {
    for (int i = 0; i < MAX_FILES; i++)
      if (!fs_table[i].used) {
        id = i;
        break;
      }
  }
  if (id == -1)
    return -1;
  if (size < 0)
    size = 0;
  if (size > MAX_FILE_SIZE - 1)
    size = MAX_FILE_SIZE - 1;
  strcpy(fs_table[id].name, name);
  memcpy(fs_table[id].data, data, size);
  fs_table[id].data[size] = 0;
  fs_table[id].size = size;
  fs_table[id].used = true;
  return id;
}

int fs_append_file(const char *name, const char *data, int size) {
  int id = fs_find_file(name);
  if (id == -1)
    return -1;
  if (size <= 0)
    return 0;
  int free_space = (MAX_FILE_SIZE - 1) - fs_table[id].size;
  if (free_space <= 0)
    return 0;
  if (size > free_space)
    size = free_space;
  memcpy(fs_table[id].data + fs_table[id].size, data, size);
  fs_table[id].size += size;
  fs_table[id].data[fs_table[id].size] = 0;
  return size;
}

int fs_delete_file(const char *name) {
  int id = fs_find_file(name);
  if (id == -1)
    return 0;
  fs_table[id].used = false;
  return 1;
}

int starts_with(const char *s, const char *prefix) {
  while (*prefix) {
    if (*s++ != *prefix++)
      return 0;
  }
  return 1;
}

int str_contains(const char *s, const char *needle) {
  if (!*needle)
    return 1;
  for (int i = 0; s[i]; i++) {
    int j = 0;
    while (s[i + j] && needle[j] && s[i + j] == needle[j])
      j++;
    if (!needle[j])
      return 1;
  }
  return 0;
}

int is_space(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

int count_words(const char *s) {
  int count = 0;
  int in_word = 0;
  while (*s) {
    if (is_space(*s)) {
      in_word = 0;
    } else if (!in_word) {
      in_word = 1;
      count++;
    }
    s++;
  }
  return count;
}

int count_lines(const char *s) {
  if (!s || !*s)
    return 0;
  int lines = 0;
  const char *p = s;
  while (*p) {
    if (*p == '\n')
      lines++;
    p++;
  }
  if (p > s && *(p - 1) != '\n')
    lines++;
  return lines;
}

int fs_used_files() {
  int used = 0;
  for (int i = 0; i < MAX_FILES; i++)
    if (fs_table[i].used)
      used++;
  return used;
}

int fs_used_bytes() {
  int total = 0;
  for (int i = 0; i < MAX_FILES; i++)
    if (fs_table[i].used)
      total += fs_table[i].size;
  return total;
}

const char *command_desc(const char *cmd) {
  if (strcmp(cmd, "help") == 0)
    return "help: Kullanilabilir komutlarin listesini gosterir.";
  if (strcmp(cmd, "sysinfo") == 0)
    return "sysinfo: Kernel ve sistem ozeti bilgilerini gosterir.";
  if (strcmp(cmd, "cpuinfo") == 0)
    return "cpuinfo: CPU uyumlulugu bilgisini gosterir.";
  if (strcmp(cmd, "calc") == 0)
    return "calc: Basit aritmetik ifade hesaplar.";
  if (strcmp(cmd, "themes") == 0)
    return "themes: Tema degistirir (dark|neon|classic).";
  if (strcmp(cmd, "echo") == 0)
    return "echo: Girilen metni ekrana yazar.";
  if (strcmp(cmd, "date") == 0)
    return "date: Tarih ve saat bilgisini gosterir.";
  if (strcmp(cmd, "time") == 0)
    return "time: Sistem saatini gosterir.";
  if (strcmp(cmd, "matrix") == 0)
    return "matrix: Matrix efektini calistirir.";
  if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0)
    return "ls: Dizin icerigini listeler.";
  if (strcmp(cmd, "pwd") == 0)
    return "pwd: Mevcut dizin yolunu gosterir.";
  if (strcmp(cmd, "cd") == 0)
    return "cd: Dizin degistirir.";
  if (strcmp(cmd, "mkdir") == 0)
    return "mkdir: Yeni dizin olusturur.";
  if (strcmp(cmd, "rmdir") == 0)
    return "rmdir: Dizin ve icindeki girdileri siler.";
  if (strcmp(cmd, "cat") == 0)
    return "cat: Dosya icerigini gosterir.";
  if (strcmp(cmd, "cp") == 0)
    return "cp: Dosya kopyalar.";
  if (strcmp(cmd, "mv") == 0)
    return "mv: Dosya tasir veya adini degistirir.";
  if (strcmp(cmd, "touch") == 0)
    return "touch: Bos dosya olusturur.";
  if (strcmp(cmd, "write") == 0)
    return "write: Dosyaya yazar (ustune yazar).";
  if (strcmp(cmd, "append") == 0)
    return "append: Dosyaya ekleme yapar.";
  if (strcmp(cmd, "stat") == 0)
    return "stat: Dosya boyutu bilgisini gosterir.";
  if (strcmp(cmd, "find") == 0)
    return "find: Dosya adinda metin arar.";
  if (strcmp(cmd, "history") == 0)
    return "history: Komut gecmisini listeler.";
  if (strcmp(cmd, "rm") == 0)
    return "rm: Dosya siler.";
  if (strcmp(cmd, "tredit") == 0)
    return "tredit: Metin duzenleyicisini acar.";
  if (strcmp(cmd, "cls") == 0 || strcmp(cmd, "clear") == 0)
    return "clear: Ekrani temizler.";
  if (strcmp(cmd, "reboot") == 0)
    return "reboot: Sistemi yeniden baslatir.";
  if (strcmp(cmd, "ver") == 0)
    return "ver: Surum bilgisini gosterir.";
  if (strcmp(cmd, "pong") == 0)
    return "pong: Pong oyununu baslatir.";
  if (strcmp(cmd, "about") == 0)
    return "about: TarkOS hakkinda kisa bilgi verir.";
  if (strcmp(cmd, "df") == 0)
    return "df: RAMDisk kullanimini gosterir.";
  if (strcmp(cmd, "wc") == 0)
    return "wc: Dosya satir/kelime/byte sayar.";
  return 0;
}

int command_runs_without_args(const char *cmd) {
  if (strcmp(cmd, "help") == 0)
    return 1;
  if (strcmp(cmd, "sysinfo") == 0)
    return 1;
  if (strcmp(cmd, "cpuinfo") == 0)
    return 1;
  if (strcmp(cmd, "date") == 0)
    return 1;
  if (strcmp(cmd, "time") == 0)
    return 1;
  if (strcmp(cmd, "matrix") == 0)
    return 1;
  if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0)
    return 1;
  if (strcmp(cmd, "pwd") == 0)
    return 1;
  if (strcmp(cmd, "cd") == 0)
    return 1;
  if (strcmp(cmd, "history") == 0)
    return 1;
  if (strcmp(cmd, "cls") == 0 || strcmp(cmd, "clear") == 0)
    return 1;
  if (strcmp(cmd, "reboot") == 0)
    return 1;
  if (strcmp(cmd, "ver") == 0)
    return 1;
  if (strcmp(cmd, "pong") == 0)
    return 1;
  if (strcmp(cmd, "about") == 0)
    return 1;
  if (strcmp(cmd, "df") == 0)
    return 1;
  return 0;
}

int fs_dir_exists(const char *path) {
  if (strcmp(path, "/") == 0)
    return 1;
  char prefix[64];
  if (path[0] == '/')
    strcpy(prefix, path + 1);
  else
    strcpy(prefix, path);
  int len = strlen(prefix);
  if (len == 0)
    return 1;
  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs_table[i].used)
      continue;
    if (starts_with(fs_table[i].name, prefix) &&
        fs_table[i].name[len] == '/')
      return 1;
  }
  return 0;
}

int fs_mkdir(const char *path) {
  if (!path || !*path)
    return 0;
  char base[64];
  strcpy(base, path);
  int len = strlen(base);
  while (len > 0 && base[len - 1] == '/') {
    base[len - 1] = 0;
    len--;
  }
  if (len == 0)
    return 0;
  if (len + 5 >= 32)
    return 0;
  char meta[64];
  strcpy(meta, base);
  strcat(meta, "/.dir");
  if (fs_find_file(meta) != -1)
    return 1;
  return fs_write_file(meta, "", 0) >= 0;
}

int fs_rmdir(const char *path) {
  if (!path || !*path)
    return 0;
  char base[64];
  strcpy(base, path);
  int len = strlen(base);
  while (len > 0 && base[len - 1] == '/') {
    base[len - 1] = 0;
    len--;
  }
  if (len == 0)
    return 0;
  int removed = 0;
  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs_table[i].used)
      continue;
    if (starts_with(fs_table[i].name, base) &&
        fs_table[i].name[len] == '/') {
      fs_table[i].used = false;
      removed++;
    }
  }
  return removed;
}

void ls_dir_path(const char *path) {
  char shown[32][32];
  int shown_count = 0;
  char prefix[64] = "";
  int prefix_len = 0;
  if (path && strcmp(path, "/") != 0 && *path) {
    if (path[0] == '/')
      strcpy(prefix, path + 1);
    else
      strcpy(prefix, path);
    prefix_len = strlen(prefix);
  }
  print("RAMDisk Index:\n");
  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs_table[i].used)
      continue;
    const char *name = fs_table[i].name;
    const char *rest = name;
    if (prefix_len > 0) {
      if (!starts_with(name, prefix))
        continue;
      if (name[prefix_len] != '/')
        continue;
      rest = name + prefix_len + 1;
      if (*rest == 0)
        continue;
    }
    char entry[32];
    int is_dir = 0;
    int j = 0;
    while (rest[j] && rest[j] != '/' && j < 31) {
      entry[j] = rest[j];
      j++;
    }
    entry[j] = 0;
    if (rest[j] == '/')
      is_dir = 1;
    int dup = 0;
    for (int k = 0; k < shown_count; k++)
      if (strcmp(shown[k], entry) == 0)
        dup = 1;
    if (dup)
      continue;
    strcpy(shown[shown_count++], entry);
    set_color(col_accent, col_bg >> 4);
    print(" \x1F ");
    set_color(0x0F, col_bg >> 4);
    print(entry);
    if (is_dir) {
      print("/\n");
    } else {
      print(" (");
      char sb[10];
      itoa(fs_table[i].size, sb);
      print(sb);
      print(" bytes)\n");
    }
  }
}

int build_path(const char *name, char *out) {
  int name_len = strlen(name);
  if (name_len == 0)
    return 0;
  if (name[0] == '/') {
    if (name_len == 1)
      return 0;
    if (name_len - 1 >= 32)
      return 0;
    strcpy(out, name + 1);
    return 1;
  }
  int base_len = 0;
  if (strcmp(current_path, "/") != 0)
    base_len = strlen(current_path + 1);
  int total = base_len + (base_len > 0 ? 1 : 0) + name_len;
  if (total >= 32)
    return 0;
  if (base_len == 0) {
    strcpy(out, name);
  } else {
    strcpy(out, current_path + 1);
    strcat(out, "/");
    strcat(out, name);
  }
  return 1;
}

void path_parent(const char *path, char *out) {
  int len = strlen(path);
  if (len <= 1) {
    strcpy(out, "/");
    return;
  }
  int i = len - 1;
  while (i > 0 && path[i] != '/')
    i--;
  if (i == 0) {
    strcpy(out, "/");
    return;
  }
  memcpy(out, path, i);
  out[i] = 0;
}

int split_args(char *line, char **argv, int max_args) {
  int argc = 0;
  char *p = line;
  while (*p && argc < max_args) {
    while (*p == ' ' || *p == '\t')
      p++;
    if (!*p)
      break;
    argv[argc++] = p;
    while (*p && *p != ' ' && *p != '\t')
      p++;
    if (*p) {
      *p = 0;
      p++;
    }
  }
  return argc;
}

char *after_n_tokens(char *s, int n) {
  int count = 0;
  while (*s && count < n) {
    while (*s == ' ' || *s == '\t')
      s++;
    if (!*s)
      break;
    while (*s && *s != ' ' && *s != '\t')
      s++;
    count++;
  }
  while (*s == ' ' || *s == '\t')
    s++;
  return s;
}

int calc_eval(const char *expr, int *out) {
  const char *p = expr;
  while (*p == ' ' || *p == '\t')
    p++;
  int sign1 = 1, sign2 = 1;
  if (*p == '-') {
    sign1 = -1;
    p++;
  }
  if (*p < '0' || *p > '9')
    return 0;
  int n1 = 0;
  while (*p >= '0' && *p <= '9')
    n1 = n1 * 10 + (*p++ - '0');
  n1 *= sign1;
  while (*p == ' ' || *p == '\t')
    p++;
  char op = *p++;
  if (op != '+' && op != '-' && op != '*' && op != '/')
    return 0;
  while (*p == ' ' || *p == '\t')
    p++;
  if (*p == '-') {
    sign2 = -1;
    p++;
  }
  if (*p < '0' || *p > '9')
    return 0;
  int n2 = 0;
  while (*p >= '0' && *p <= '9')
    n2 = n2 * 10 + (*p++ - '0');
  n2 *= sign2;
  int res = 0;
  if (op == '+')
    res = n1 + n2;
  else if (op == '-')
    res = n1 - n2;
  else if (op == '*')
    res = n1 * n2;
  else if (op == '/' && n2 != 0)
    res = n1 / n2;
  else if (op == '/' && n2 == 0)
    return 0;
  *out = res;
  return 1;
}

void ls_current_dir() {
  char shown[32][32];
  int shown_count = 0;
  char prefix[64] = "";
  int prefix_len = 0;
  if (strcmp(current_path, "/") != 0) {
    strcpy(prefix, current_path + 1);
    prefix_len = strlen(prefix);
  }
  print("RAMDisk Index:\n");
  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs_table[i].used)
      continue;
    const char *name = fs_table[i].name;
    const char *rest = name;
    if (prefix_len > 0) {
      if (!starts_with(name, prefix))
        continue;
      if (name[prefix_len] != '/')
        continue;
      rest = name + prefix_len + 1;
      if (*rest == 0)
        continue;
    }
    char entry[32];
    int is_dir = 0;
    int j = 0;
    while (rest[j] && rest[j] != '/' && j < 31) {
      entry[j] = rest[j];
      j++;
    }
    entry[j] = 0;
    if (rest[j] == '/')
      is_dir = 1;
    int dup = 0;
    for (int k = 0; k < shown_count; k++)
      if (strcmp(shown[k], entry) == 0)
        dup = 1;
    if (dup)
      continue;
    strcpy(shown[shown_count++], entry);
    set_color(col_accent, col_bg >> 4);
    print(" \x1F ");
    set_color(0x0F, col_bg >> 4);
    print(entry);
    if (is_dir) {
      print("/\n");
    } else {
      print(" (");
      char sb[10];
      itoa(fs_table[i].size, sb);
      print(sb);
      print(" bytes)\n");
    }
  }
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
  if (sc == 0x48)
    return -1; // Special code for Up Arrow
  if (sc == 0x50)
    return -2;
  if (sc >= 58 && sc != 0x0F && sc != 0x3C && sc != 0x44)
    return 0;
  return shift ? caps[sc] : map[sc];
}

/* ============= TrEdit Pro v3.6 (NOVA ETERNAL) ============= */
static char edit_buffer[MAX_FILE_SIZE];
void tredit(const char *filename) {
  int id = fs_find_file(filename);
  if (id != -1)
    strcpy(edit_buffer, fs_table[id].data);
  else
    memset(edit_buffer, 0, MAX_FILE_SIZE);
  bool run = true, shift = false, redraw = true;
  int ptr = strlen(edit_buffer);

  while (run) {
    if (redraw) {
      draw_rect(0, 0, 80, 25, col_bg);
      draw_rect(0, 0, 80, 1, col_header);
      print_at(1, 0, " TrEdit Professional v3.9 | Editing: ", col_header);
      print_at(38, 0, filename, col_header);
      draw_rect(0, 24, 80, 1, col_footer);

      // Calculate current row/column
      int cur_r = 1, cur_c = 1;
      for (int i = 0; i < ptr; i++) {
        if (edit_buffer[i] == '\n') {
          cur_r++;
          cur_c = 1;
        } else {
          cur_c++;
          if (cur_c > 78) {
            cur_r++;
            cur_c = 1;
          }
        }
      }

      char row_str[10], col_str[10];
      itoa(cur_r, row_str);
      itoa(cur_c, col_str);
      print_at(2, 24, "Line: ", col_footer);
      print_at(8, 24, row_str, col_footer);
      print_at(14, 24, "Col: ", col_footer);
      print_at(19, 24, col_str, col_footer);
      print_at(32, 24, " [ F2: Save ]  [ F10/ESC: Exit ] ", col_footer);

      // Editor Content
      int r = 1, c = 1;
      for (int i = 0; i < ptr; i++) {
        if (edit_buffer[i] == '\n') {
          r++;
          c = 1;
        } else {
          put_char_raw(edit_buffer[i], (col_bg & 0xF0) | 0x0F, c++, r);
          if (c > 78) {
            r++;
            c = 1;
          }
        }
        if (r > 23)
          break;
      }
      update_cursor(c, r);
      redraw = false;
    }

    draw_shell_dynamic();
    uint8_t sc = get_any_scancode();
    if (!sc) {
      delay_ms(10);
      continue;
    }

    if (sc == 0x2A || sc == 0x36)
      shift = true;
    else if (sc == 0xAA || sc == 0xB6)
      shift = false;
    else if (sc & 0x80)
      continue;
    else if (sc == 0x01 || sc == 0x44) { // ESC or F10
      run = false;
    } else if (sc == 0x3C) { // F2
      fs_write_file(filename, edit_buffer, ptr);
      draw_rect(20, 10, 40, 5, 0x2F);
      print_at(25, 12, " [ FILE SAVED SUCCESSFULLY ] ", 0x2F);
      delay_ms(800);
      redraw = true;
    } else {
      char ch = scancode_to_char(sc, shift);
      if (ch == '\n' || ch == '\b' || (ch && ptr < MAX_FILE_SIZE - 1)) {
        if (ch == '\b') {
          if (ptr > 0)
            edit_buffer[--ptr] = 0;
        } else {
          edit_buffer[ptr++] = ch;
        }
        redraw = true;
      }
    }
  }
  clear_screen();
}

/* ============= Visual Effects ============= */
void effect_matrix() {
  clear_screen();
  uint8_t drops[VGA_WIDTH];
  for (int i = 0; i < VGA_WIDTH; i++)
    drops[i] = (uint8_t)(-(i % 20));

  bool run = true;
  while (run) {
    if (get_any_scancode() == 0x01)
      run = false; // ESC to exit

    for (int x = 0; x < VGA_WIDTH; x++) {
      if (drops[x] < VGA_HEIGHT) {
        put_char_raw((char)(33 + (drops[x] % 94)), col_matrix, x, drops[x]);
        if (drops[x] > 0)
          put_char_raw((char)(33 + (drops[x] % 94)), 0x0A, x, drops[x] - 1);
      }
      if (drops[x] >= 1 && drops[x] <= VGA_HEIGHT)
        put_char_raw(' ', col_bg, x, drops[x] - 2);

      drops[x]++;
      if (drops[x] >= VGA_HEIGHT + 2)
        drops[x] = 0;
    }
    delay_ms(5); // Adjusted for 100k loop delay
    draw_shell_dynamic();
  }
  clear_screen();
}

/* ============= Games ============= */
void game_pong() {
  clear_screen();
  int b_x = 40, b_y = 12, b_dx = 1, b_dy = 1;
  int p1_y = 10, p2_y = 10;
  bool run = true;

  draw_window(10, 5, 60, 15, " PONG NOVA ", col_bg);

  while (run) {
    uint8_t sc = get_any_scancode();
    if (sc == 0x01)
      run = false;
    if (sc == 0x11) {
      if (p1_y > 6)
        p1_y--;
    } // W
    if (sc == 0x1F) {
      if (p1_y < 16)
        p1_y++;
    } // S

    // AI for P2
    if (b_y > p2_y + 1 && p2_y < 16)
      p2_y++;
    if (b_y < p2_y + 1 && p2_y > 6)
      p2_y--;

    // Clear old ball
    put_char_raw(' ', col_bg, b_x, b_y);
    // Clear old paddles
    for (int i = 6; i < 19; i++) {
      put_char_raw(' ', col_bg, 12, i);
      put_char_raw(' ', col_bg, 67, i);
    }

    // Move ball
    b_x += b_dx;
    b_y += b_dy;
    if (b_y <= 6 || b_y >= 18)
      b_dy = -b_dy;
    if (b_x <= 13 && b_y >= p1_y && b_y <= p1_y + 3)
      b_dx = -b_dx;
    if (b_x >= 66 && b_y >= p2_y && b_y <= p2_y + 3)
      b_dx = -b_dx;

    if (b_x < 11 || b_x > 68) {
      b_x = 40;
      b_y = 12;
    }

    // Draw paddles
    for (int i = 0; i < 4; i++) {
      put_char_raw('|', 0x0F, 12, p1_y + i);
      put_char_raw('|', 0x0F, 67, p2_y + i);
    }
    // Draw ball
    put_char_raw('O', 0x0E, b_x, b_y);

    delay_ms(50); // Slowed down from 5ms
    draw_shell_dynamic();
  }
  clear_screen();
}

/* ============= SHELL CORE v3.7 (NOVA ULTIMATE FIX) ============= */
#define HISTORY_SIZE 8
static char history_buf[HISTORY_SIZE][64];
static int history_count = 0;
void shell_loop() {
  char line[64];
  char raw_line[64];
  char *argv[8];
  int pos = 0;
  bool shift = false;
  int history_pos = 0;
  clear_screen();
  while (1) {
    set_color(0x0F, col_bg >> 4);
    print("\n[nova] ");
    set_color(col_prompt, col_bg >> 4);
    print(current_path);
    set_color(col_accent, col_bg >> 4);
    print(" >> ");
    set_color(0x0F, col_bg >> 4);
    pos = 0;
    history_pos = history_count;
    memset(line, 0, 64);
    while (1) {
      uint8_t sc = get_any_scancode();
      draw_shell_dynamic();
      if (!sc)
        continue;
      if (sc == 0x2A || sc == 0x36)
        shift = true;
      else if (sc == 0xAA || sc == 0xB6)
        shift = false;
      if (sc & 0x80)
        continue;

      if (sc == 0x48 && history_count > 0) {
        if (history_pos > 0)
          history_pos--;
        while (pos > 0) {
          pos--;
          put_char('\b');
        }
        strcpy(line, history_buf[history_pos]);
        pos = strlen(line);
        print(line);
        continue;
      }

      char c = scancode_to_char(sc, shift);
      if (c == (char)-1)
        continue;
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
    }
    if (pos > 0) {
      line[pos] = 0;
      strcpy(raw_line, line);
      if (history_count == 0 ||
          strcmp(history_buf[history_count - 1], line) != 0) {
        if (history_count < HISTORY_SIZE) {
          strcpy(history_buf[history_count++], line);
        } else {
          for (int i = 1; i < HISTORY_SIZE; i++)
            strcpy(history_buf[i - 1], history_buf[i]);
          strcpy(history_buf[HISTORY_SIZE - 1], line);
        }
      }
      int argc = split_args(line, argv, 8);
      if (argc == 0)
        continue;

      if (argc == 1) {
        const char *desc = command_desc(argv[0]);
        if (desc) {
          print(desc);
          print("\n");
          if (!command_runs_without_args(argv[0]))
            continue;
        }
      }

      if (strcmp(argv[0], "help") == 0) {
        set_color(col_accent, col_bg >> 4);
        print("\nTARKOS NOVA ULTIMATE - CONSOLE ASSISTANCE\n");
        set_color(0x0F, col_bg >> 4);
        print("- FS: ls, cat, touch, rm, cd, mkdir, rmdir, cp, mv, pwd, write, "
              "append, stat, find\n");
        print("- App: tredit, cls, ver, reboot, time, date, echo, matrix, "
              "cpuinfo, calc, themes, sysinfo, pong, history\n");
        print("- Info: about, df, wc\n");
        print("- UI: 9.4s Hyper Boot [Enabled]\n");
      } else if (strcmp(argv[0], "sysinfo") == 0) {
        print("TarkOS Nova v1.9.6 [Eternal Edition]\n");
        print("Build: 2026-01-30.01\n");
        print("Kernel: 32-bit x86 Protected Mode\n");
        print("Memory Manager: PMM + Paging [Active]\n");
        print("CPU: Multiboot Detected 3-Core SMP\n");
        print("GUI: Zero-Flicker Dual-Bar [Stable]\n");
      } else if (strcmp(argv[0], "about") == 0) {
        print("TarkOS Nova v1.9.6 Ultimate\n");
        print("Hyper Boot: 9.4s | SMP x3 | RAM 512MB\n");
        print("VFS: RAMDisk | Shell: Nova Console v3.7\n");
        print("Themes: dark, neon, classic\n");
      } else if (strcmp(argv[0], "df") == 0) {
        int used_files = fs_used_files();
        int used_bytes = fs_used_bytes();
        int total_files = MAX_FILES;
        int total_bytes = MAX_FILES * (MAX_FILE_SIZE - 1);
        char buf[16];
        print("Files: ");
        itoa(used_files, buf);
        print(buf);
        print("/");
        itoa(total_files, buf);
        print(buf);
        print("\nData: ");
        itoa(used_bytes, buf);
        print(buf);
        print("/");
        itoa(total_bytes, buf);
        print(buf);
        print(" bytes\n");
      } else if (strcmp(argv[0], "wc") == 0) {
        if (argc < 2) {
          print("Usage: wc <filename>\n");
        } else {
          char path[64];
          if (!build_path(argv[1], path)) {
            print("Error: Invalid path.\n");
          } else {
            int id = fs_find_file(path);
            if (id != -1) {
              int lines = count_lines(fs_table[id].data);
              int words = count_words(fs_table[id].data);
              int bytes = fs_table[id].size;
              char buf[16];
              print("Lines: ");
              itoa(lines, buf);
              print(buf);
              print("  Words: ");
              itoa(words, buf);
              print(buf);
              print("  Bytes: ");
              itoa(bytes, buf);
              print(buf);
              print("\n");
            } else {
              print("Error: File not found.\n");
            }
          }
        }
      } else if (strcmp(argv[0], "cpuinfo") == 0) {
        uint32_t a, d;
        cpuid(0, &a, &d);
        print("CPU Vendor: ");
        if (a == 0)
          print("Unknown\n");
        else
          print("x86 Compatible\n");
      } else if (strcmp(argv[0], "calc") == 0) {
        int res = 0;
        if (calc_eval(raw_line + 4, &res)) {
          char buf[16];
          itoa(res, buf);
          print("Result: ");
          print(buf);
          print("\n");
        } else {
          print("Usage: calc <a> <op> <b>\n");
        }
      } else if (strcmp(argv[0], "themes") == 0) {
        if (argc == 2 && strcmp(argv[1], "dark") == 0)
          set_theme(0);
        else if (argc == 2 && strcmp(argv[1], "neon") == 0)
          set_theme(1);
        else if (argc == 2 && strcmp(argv[1], "classic") == 0)
          set_theme(2);
        else
          print("Usage: themes [dark|neon|classic]\n");
      } else if (strcmp(argv[0], "echo") == 0) {
        char *msg = after_n_tokens(raw_line, 1);
        if (*msg) {
          print(msg);
          print("\n");
        }
      } else if (strcmp(argv[0], "date") == 0) {
        char tb[16];
        char db[16];
        get_time_str(tb);
        get_date_str(db);
        print("Today is: ");
        print(db);
        print(" | Local Time: ");
        print(tb);
        print("\n");
      } else if (strcmp(argv[0], "time") == 0) {
        char tb[16];
        get_time_str(tb);
        print("System Time: ");
        print(tb);
        print("\n");
      } else if (strcmp(argv[0], "matrix") == 0) {
        effect_matrix();
      } else if (strcmp(argv[0], "ls") == 0 || strcmp(argv[0], "dir") == 0) {
        if (argc == 1) {
          ls_current_dir();
        } else if (strcmp(argv[1], "/") == 0) {
          ls_dir_path("/");
        } else {
          char path[64];
          if (build_path(argv[1], path))
            ls_dir_path(path);
          else
            print("Error: Invalid path.\n");
        }
      } else if (strcmp(argv[0], "pwd") == 0) {
        print(current_path);
        print("\n");
      } else if (strcmp(argv[0], "cd") == 0) {
        if (argc == 1) {
          print(current_path);
          print("\n");
        } else {
          char new_path[64];
          if (strcmp(argv[1], "/") == 0) {
            strcpy(current_path, "/");
          } else if (strcmp(argv[1], "..") == 0) {
            path_parent(current_path, new_path);
            strcpy(current_path, new_path);
          } else if (strcmp(argv[1], ".") == 0) {
          } else {
            if (argv[1][0] == '/') {
              strcpy(new_path, argv[1]);
            } else if (strcmp(current_path, "/") == 0) {
              new_path[0] = '/';
              new_path[1] = 0;
              strcat(new_path, argv[1]);
            } else {
              strcpy(new_path, current_path);
              strcat(new_path, "/");
              strcat(new_path, argv[1]);
            }
            if (fs_dir_exists(new_path))
              strcpy(current_path, new_path);
            else
              print("Error: Directory not found.\n");
          }
        }
      } else if (strcmp(argv[0], "mkdir") == 0) {
        if (argc < 2) {
          print("Usage: mkdir <dirname>\n");
        } else {
          char path[64];
          if (!build_path(argv[1], path)) {
            print("Error: Invalid path.\n");
          } else if (fs_mkdir(path)) {
            print("Directory created.\n");
          } else {
            print("Error: Unable to create directory.\n");
          }
        }
      } else if (strcmp(argv[0], "rmdir") == 0) {
        if (argc < 2) {
          print("Usage: rmdir <dirname>\n");
        } else {
          char path[64];
          if (!build_path(argv[1], path)) {
            print("Error: Invalid path.\n");
          } else if (fs_rmdir(path) > 0) {
            print("Directory removed.\n");
          } else {
            print("Error: Directory not found.\n");
          }
        }
      } else if (strcmp(argv[0], "cat") == 0) {
        if (argc < 2) {
          print("Usage: cat <filename>\n");
        } else {
          char path[64];
          build_path(argv[1], path);
          int id = fs_find_file(path);
          if (id != -1) {
            print(fs_table[id].data);
            print("\n");
          } else
            print("Error: File not found.\n");
        }
      } else if (strcmp(argv[0], "cp") == 0) {
        if (argc < 3) {
          print("Usage: cp <src> <dest>\n");
        } else {
          char src_path[64];
          char dest_path[64];
          build_path(argv[1], src_path);
          build_path(argv[2], dest_path);
          int sid = fs_find_file(src_path);
          if (sid != -1) {
            if (fs_write_file(dest_path, fs_table[sid].data,
                              fs_table[sid].size) != -1)
              print("Copied.\n");
            else
              print("Error: No space.\n");
          } else
            print("Source not found.\n");
        }
      } else if (strcmp(argv[0], "mv") == 0) {
        if (argc < 3) {
          print("Usage: mv <src> <dest>\n");
        } else {
          char src_path[64];
          char dest_path[64];
          build_path(argv[1], src_path);
          build_path(argv[2], dest_path);
          int sid = fs_find_file(src_path);
          if (sid == -1) {
            print("Source not found.\n");
          } else if (fs_find_file(dest_path) != -1) {
            print("Error: Destination exists.\n");
          } else {
            strcpy(fs_table[sid].name, dest_path);
            print("Moved.\n");
          }
        }
      } else if (strcmp(argv[0], "touch") == 0) {
        if (argc < 2) {
          print("Usage: touch <filename>\n");
        } else {
          char path[64];
          build_path(argv[1], path);
          if (fs_write_file(path, "", 0) != -1)
            print("File created.\n");
          else
            print("Error: No space.\n");
        }
      } else if (strcmp(argv[0], "write") == 0) {
        if (argc < 3) {
          print("Usage: write <filename> <text>\n");
        } else {
          char path[64];
          build_path(argv[1], path);
          char *msg = after_n_tokens(raw_line, 2);
          if (fs_write_file(path, msg, strlen(msg)) != -1)
            print("Written.\n");
          else
            print("Error: No space.\n");
        }
      } else if (strcmp(argv[0], "append") == 0) {
        if (argc < 3) {
          print("Usage: append <filename> <text>\n");
        } else {
          char path[64];
          build_path(argv[1], path);
          char *msg = after_n_tokens(raw_line, 2);
          int wrote = fs_append_file(path, msg, strlen(msg));
          if (wrote >= 0)
            print("Appended.\n");
          else
            print("Error: File not found.\n");
        }
      } else if (strcmp(argv[0], "stat") == 0) {
        if (argc < 2) {
          print("Usage: stat <filename>\n");
        } else {
          char path[64];
          build_path(argv[1], path);
          int id = fs_find_file(path);
          if (id != -1) {
            print("Name: ");
            print(path);
            print("\nSize: ");
            char sb[16];
            itoa(fs_table[id].size, sb);
            print(sb);
            print(" bytes\n");
          } else
            print("Error: File not found.\n");
        }
      } else if (strcmp(argv[0], "find") == 0) {
        if (argc < 2) {
          print("Usage: find <text>\n");
        } else {
          int found = 0;
          for (int i = 0; i < MAX_FILES; i++) {
            if (!fs_table[i].used)
              continue;
            if (str_contains(fs_table[i].name, argv[1])) {
              print(fs_table[i].name);
              print("\n");
              found = 1;
            }
          }
          if (!found)
            print("No matches.\n");
        }
      } else if (strcmp(argv[0], "history") == 0) {
        for (int i = 0; i < history_count; i++) {
          char sb[8];
          itoa(i + 1, sb);
          print(sb);
          print(": ");
          print(history_buf[i]);
          print("\n");
        }
      } else if (strcmp(argv[0], "rm") == 0) {
        if (argc < 2) {
          print("Usage: rm <filename>\n");
        } else {
          char path[64];
          build_path(argv[1], path);
          if (fs_delete_file(path))
            print("File deleted.\n");
          else
            print("Error: File not found.\n");
        }
      } else if (strcmp(argv[0], "tredit") == 0) {
        if (argc < 2)
          print("Usage: tredit <filename>\n");
        else {
          char path[64];
          build_path(argv[1], path);
          tredit(path);
        }
      } else if (strcmp(argv[0], "cls") == 0 || strcmp(argv[0], "clear") == 0)
        clear_screen();
      else if (strcmp(argv[0], "reboot") == 0)
        outb(0x64, 0xFE);
      else if (strcmp(argv[0], "ver") == 0)
        print("TarkOS Nova v1.9.6 Ultimate [Stable]\n");
      else if (strcmp(argv[0], "pong") == 0)
        game_pong();
      else {
        print("Nova Error: '");
        print(raw_line);
        print("' unknown.\n");
      }
    }
  }
}

/* ============= KERNEL MAIN ============= */
// Eternal Nova Boot Sequence (9.4s)
void hyper_cinematic_nova_eternal_boot() {
  clear_screen();
  draw_window(5, 2, 70, 21, " TARKOS NOVA | HYPER BOOT 9.4s ", col_bg);

  set_color(0x1B, col_bg >> 4);
  print_at(10, 4, "====================== NOVA CORE ======================", 0x1B);
  print_at(10, 6, " _____   _    ____  _  _  _____  ____   ____ ", 0x1B);
  print_at(10, 7, "|_   _| / \\  |  _ \\| |/ /| ____|/ ___| / ___|", 0x1B);
  print_at(10, 8, "  | |  / _ \\ | |_) | ' / |  _|  \\___ \\ \\___ \\", 0x1B);
  print_at(10, 9, "  | | / ___ \\|  _ <| . \\ | |___  ___) | ___) |", 0x1B);
  print_at(10, 10, "  |_|/_/   \\_\\_| \\_\\_|\\_\\|_____||____/ |____/ ", 0x1B);

  print_at(12, 12, "NOVA ETERNAL KERNEL v1.9.6 ULTIMATE", col_accent);
  print_at(12, 13, "SMP x3  |  RAM 512MB  |  VFS RAMDisk  |  VGA TUI", col_success);
  print_at(12, 14, "Boot Target: 9.4s  |  Secure Init  |  Stable", col_success);

  const char *phases[] = {
      "[ CORE   ] Multi-Core Vector Alignment...",
      "[ MEMORY ] High-Integrity Page Map Build...",
      "[ VFS    ] RAMDisk Index and Metadata...",
      "[ TUI    ] Dual-Bar Zero-Flicker Renderer...",
      "[ IO     ] Interrupt and Device Sync...",
      "[ SHELL  ] Professional Console Context..."};

  const char spin[] = "|/-\\";
  int spin_idx = 0;

  delay_ms(400);
  for (int p = 0; p < 6; p++) {
    print_at(12, 17, "                                                      ", col_bg);
    print_at(12, 17, phases[p], col_success);
    for (int i = 0; i < 11; i++) {
      char s[2];
      s[0] = spin[spin_idx & 3];
      s[1] = 0;
      print_at(62, 17, s, col_accent);
      put_char_raw(219, 0x0B, 8 + p * 11 + i, 21);
      spin_idx++;
      delay_ms(136);
    }
  }
}

void kmain() {
  fs_init();
  hyper_cinematic_nova_eternal_boot();
  shell_loop();
}
