#include "types.h"
#include "utils.h"

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

int putc(char c) { return write(1, &c, 1); }

int puts(const char *str) {
  for (;;) {
    if (*str == 0)
      break;
    putc(*str);
    str++;
  }
  return 0;
}

int putn(int num, int base) {
  char str[16];
  int ret = itoa(num, (char *)&str, base);

  puts((char *)&str);
  return ret;
}

int printf(const char *format, ...){
    __builtin_va_list ap;
    __builtin_va_start(ap, format); 

    char* fp = (char*)format;
    do {
      if (*fp != '%') {
        putc(*fp);
        continue;
      }

      switch(*++fp){
        case 'd':
          putn(__builtin_va_arg(ap, int), 10);
          break;

        case 'p':
          putn(__builtin_va_arg(ap, int), 16);
          break;

        case 's':
          puts(__builtin_va_arg(ap, char*));
          break;

        case 'c':
          putc(__builtin_va_arg(ap, int));
          break;

        default:
          putc('%');
          putc(*fp);
      }
    } while(*fp++);

    __builtin_va_end(ap);
}
