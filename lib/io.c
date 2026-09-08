#include "libotman.h"

ssize_t write(int fd, char *buf, long count) {
  ssize_t ret;
  __asm__ volatile("mov $1, %%rax\n"
                   "mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"((long)fd), "r"(buf), "r"(count)
                   : "rdi", "rsi", "rdx");
  return ret;
}

ssize_t read(int fd, char *buf, long count) {
  ssize_t ret;
  __asm__ volatile("mov $0, %%rax\n"
                   "mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
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
    if (buf[n - 1] == '\n')
      break;
    buf = realloc(buf, 256);
    buf += 256;
  }
  buf[n - 1] = '\0';
  return buf;
}

int putc(char c) { return write(1, &c, 1); }

int puts(const char *str) {
  for (;;) {
    if (*str == 0)
      break;
    putc(*str);
    str++;
  }
  putc('\n');
  return 0;
}

int putn(int num, int base) {
  char str[16];
  int ret = itoa(num, (char *)&str, base);

  puts((char *)&str);
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
      count += putn(__builtin_va_arg(ap, int), 10);
      break;
    case 'p':
      puts("0x");
      count += putn((long)__builtin_va_arg(ap, void *), 16);
      break;
    case 's': {
      const char *s = __builtin_va_arg(ap, char *);
      puts(s);
      while (*s++)
        count++;
    } break;
    case 'c':
      putc(__builtin_va_arg(ap, int));
      count++;
      break;
    case '%':
      putc('%');
      count++;
      break;
    default:
      putc('%');
      putc(*fp);
      count += 2;
    }
  }

  __builtin_va_end(ap);
  return count;
}
