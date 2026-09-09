#ifndef LIBOTMAN_H
#define LIBOTMAN_H

typedef unsigned long long size_t;
typedef long long ssize_t;

// io
ssize_t write(int fd, char *buf, long count);
ssize_t read(int fd, char *buf, long count);
char* getline();
int putc(char c);
int puts(const char *str);
int put(const char *str);
int putn(int num, int base);
int printf(const char *format, ...);

// utils
[[noreturn]] void exit(int status);
ssize_t getrandom(char *buf, size_t count);
void *memcpy(char *dest, char* src, size_t n);
int itoa(int num, char *str, int base);
ssize_t execve(const char *path, const char *argv, const char *envp);
int fork();
int wait(int pid, int wstatus);
int strlen(const char* s);
ssize_t open(const char *path, int flags, int *mode);

// crypto
unsigned int crc32(const unsigned char *message, unsigned int len);

// strings
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);

// malloc
struct chunk {
  long sig;
  int prev;
  int size;
};

struct meta {
  void *fd;
  void *bk;
};

void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

#endif // H_LIB
