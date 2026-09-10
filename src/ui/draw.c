#include "pwnman.h"

#define MAX_COLS 8

void echo_off() {
  struct termios state;
  (void)ioctl(0, (int)TCGETS, (long)&state);
  state.c_lflag &= ~ECHO;
  (void)ioctl(0, (int)TCSETS, (long)&state);
}

void echo_on() {
  struct termios state;
  (void)ioctl(0, (int)TCGETS, (long)&state);
  state.c_lflag |= ECHO;
  (void)ioctl(0, (int)TCSETS, (long)&state);
}

void print_separator(int len) {
  for (int i = 0; i < len; i++)
    putc('-');
  putc('\n');
}

void print_pad_num(int num, int width) {
  char buf[width + 1];
  int len = itoa(num, buf, 10);

  for (int i = len; i < width; i++)
    putc(' ');
  put(buf);
}

static void draw_line(const int *widths, int col) {
  putc('-');
  for (int c = 0; c < col; c++)
    for (int i = 0; i < widths[c] + 2; i++)
      putc('-');
  putc('-');
  putc('\n');
}

static void print_padded(const char *str, int width) {
  put(str);
  for (int i = (int)strlen(str); i < width; i++)
    putc(' ');
}

static void print_row(const int *widths, int col, char *const *items) {
  for (int c = 0; c < col; c++) {
    if (c)
      printf("| ");
    print_padded(items[c], widths[c]);
    putc(' ');
  }
  putc('\n');
}

void print_table(int col, ...) {
  if (col < 1 || col > MAX_COLS)
    return;

  __builtin_va_list ap;
  __builtin_va_start(ap, col);

  char *headers[MAX_COLS];
  char *values[MAX_COLS];
  int widths[MAX_COLS];

  for (int c = 0; c < col; c++)
    headers[c] = __builtin_va_arg(ap, char *);

  for (int c = 0; c < col; c++) {
    values[c] = __builtin_va_arg(ap, char *);
    int h = (int)strlen(headers[c]);
    int v = (int)strlen(values[c]);
    widths[c] = h > v ? h : v;
  }

  __builtin_va_end(ap);

  draw_line(widths, col);
  print_row(widths, col, headers);
  draw_line(widths, col);
  print_row(widths, col, values);
  draw_line(widths, col);
}
