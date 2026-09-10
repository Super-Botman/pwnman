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

char *getline() {
  char *buf = malloc(256);
  int n = 0;
  while (1) {
    n = read(0, buf, 256);
    if (n>0 && buf[n - 1] == '\n')
      break;
    buf = realloc(buf, 256);
    buf += 256;
  }
  buf[n - 1] = '\0';
  return buf;
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
