#include "libotman.h"

[[noreturn]] void exit(int status) {
  __asm__ volatile("mov $60, %%rax\n"
                   "mov %0, %%rdi\n"
                   "syscall\n"
                   :
                   : "r"((long)status)
                   :);
  __builtin_unreachable();
}

void close(int fd) {
  __asm__ volatile("mov $3, %%rax\n"
                   "mov %0, %%rdi\n"
                   "syscall\n"
                   :
                   : "r"((long)fd)
                   :);
}

ssize_t getrandom(char *buf, size_t count) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov $318, %%rax\n"
                   "mov $0, %%rdx\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"(buf), "r"(count)
                   : "%rdi", "%rsi");
  return ret;
}

int rename(char *oldpath, char* newpath) {
  int ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov $82, %%rax\n"
                   "mov $0, %%rdx\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"(oldpath), "r"(newpath)
                   : "%rdi", "%rsi");
  return ret;
}

ssize_t execve(const char *path, const char *argv, const char *envp) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "mov $59, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"(path), "r"(argv), "r"(envp)
                   : "rdi", "rsi", "rdx");
  return ret;
}


ssize_t ioctl(unsigned int fd, unsigned int cmd, unsigned long args) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "mov $16, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"((long)fd), "r"((long)cmd), "r"(args)
                   : "rdi", "rsi", "rdx");
  return ret;
}

int fork() {
  ssize_t ret;
  __asm__ volatile("mov $58, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   :
                   :);
  return ret;
}

int wait(int pid, int wstatus) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov $0, %%rdx\n"
                   "mov $0, %%r10\n"
                   "mov $61, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"((long)pid), "r"((long)wstatus)
                   : "rdi", "rsi", "rdx");
  return ret;
}

ssize_t open(const char *path, int flags, int mode) {
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "mov $2, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"(path), "r"((long)flags), "r"((long)mode)
                   : "rdi", "rsi", "rdx");
  return ret;
}

size_t lseek(int fd, size_t offset, int whence){
  ssize_t ret;
  __asm__ volatile("mov %1, %%rdi\n"
                   "mov %2, %%rsi\n"
                   "mov %3, %%rdx\n"
                   "mov $8, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "r"((long)fd), "r"(offset), "r"((long)whence)
                   : "rdi", "rsi", "rdx");
  return ret;
}

void *memcpy(char *dest, char *src, size_t n) {
  for (int i = 0; i < n; i++)
    dest[i] = src[i];
  return dest;
}

void *memset(char *src, int c, size_t n) {
  for (int i = 0; i < n; i++)
    src[i] = 0;
  return src;
}

char* strdup(char* str){
  size_t len = strlen(str);
  char *ret = malloc(len);
  if (ret>0)
    memcpy(ret, str, len);
  return ret;
}

int itoa(int num, char *str, int base) {
  if (num < 0) {
    *str++ = '-';
    num = -num;
  }

  char buf[17];
  buf[16] = '\0';

  int i = 15;
  while (i) {
    buf[i] = num % base;

    if (buf[i] < 10)
      buf[i] += '0';
    else
      buf[i] += 'a' - 10;

    num /= base;
    if (num == 0)
      break;
    i--;
  }

  char *tp = (char *)&buf + i;
  while (*tp)
    *str++ = *tp++;
  *str++ = '\0';

  return 16 - i;
}

int atoi(char* str) {
  int ret = 0;
  for(int i = 0; i < 16; i++){
    if (str[i] == '\0') break;
    ret += (str[i]-'0') << (i*4);
  }
  return ret;
}
