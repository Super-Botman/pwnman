#include "types.h"

[[noreturn]] void exit(int status) {
  __asm__ volatile("mov $60, %%rax\n"
                   "mov %0, %%rdi\n"
                   "syscall\n"
                   :
                   : "r"((long)status)
                   :);
}

int getrandom(char *buf, size_t count) {
  int ret;
  __asm__ volatile("mov $318, %%rax\n"
                   "mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov $0, %%rdx\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"(buf), "r"(count)
                   : "rdi", "rsi", "rdx");
  return ret;
}

int itoa(int num, char *str, int base) {
  if (num < 0) {
    *str++ = '-';
    num = -num;
  }

  char tmp[17];
  tmp[16] = '\0';

  int i = 15;
  while (i) {
    tmp[i] = num % base;

    if (tmp[i] < 10)
      tmp[i] += '0';
    else
      tmp[i] += 'a' - 10;

    num /= base;
    if (num == 0)
      break;
    i--;
  }

  char *tp = (char *)&tmp + i;
  while (*tp)
    *str++ = *tp++;
  *str++ = '\0';

  return 16 - i;
}
