#include "libotman.h"

ssize_t write(int fd, char *buf, long count) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "mov $1, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"((long)fd), "r"(buf), "r"(count)
                   : "rdi", "rsi", "rdx");
  return ret;
}

ssize_t read(int fd, char *buf, long count) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "mov $0, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"((long)fd), "r"(buf), "r"(count)
                   : "rdi", "rsi", "rdx");
  return ret;
}

char getchar() {
  char chr;
  read(0, &chr, 1);
  return chr;
}

ssize_t readline(char *buf, size_t n) {
  if (!buf || n < 2) {
    return -1;
  }

  int len = read(0, buf, (int)n);
  if(len < 0)
    return -1;

  if (buf[len - 1] == '\n') {
    buf[--len] = '\0';
    return (ssize_t)len;
  }

  while (buf[len] != '\n' && getchar() != '\n'){}

  return (ssize_t)len;
}

ssize_t getline(char **lineptr, size_t *n) {
  if (!lineptr || !n) {
    return -1;
  }
  if (*n == 0)
    *lineptr = 0;

  char *buf = *lineptr;
  size_t cap = buf ? *n : 0;
  size_t used = 0;

  for (;;) {
    if (used + 1 >= cap) {
      size_t newcap = cap ? cap * 2 : 128;
      char *tmp = realloc(buf, newcap);
      buf = tmp;
      cap = newcap;
    }

    ssize_t r = read(0, buf + used, cap - used - 1);
    if (r < 0) {
      return -1;
    }
    if (r == 0)
      break;

    used += (size_t)r;
    if (buf[used - 1] == '\n')
      break;
  }

  buf[used - 1] = '\0';
  *lineptr = buf;
  *n = cap;
  return (ssize_t)used-1;
}

int putc(char c) { return write(1, &c, 1); }

int put(const char *str) {
  int len = strlen(str);
  return write(0, (char *)str, len);
}

int puts(const char *str) {
  put(str);
  putc('\n');
  return 0;
}

int putn(int num, int base) {
  char str[16];
  int ret = itoa(num, (char *)&str, base);
  put((char *)&str);
  return ret;
}

int printf(const char *format, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, format);
  int count = 0;

  for (const char *fp = format; *fp != 0; fp++) {
    if (*fp != '%') {
      putc(*fp);
      count++;
      continue;
    }

    switch (*++fp) {
    case 'd':
      putn(__builtin_va_arg(ap, int), 10);
      break;
    case 'p':
      put("0x");
      putn((long)__builtin_va_arg(ap, void *), 16);
      break;
    case 's':
      const char *s = __builtin_va_arg(ap, char *);
      put(s);
      break;
    case 'c':
      putc(__builtin_va_arg(ap, int));
      break;
    case '%':
      putc('%');
      break;
    default:
      putc('%');
      putc(*fp);
    }
  }

  __builtin_va_end(ap);
  return count;
}
