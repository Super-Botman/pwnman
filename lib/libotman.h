#ifndef LIBOTMAN_H
#define LIBOTMAN_H

typedef unsigned long long size_t;
typedef long long ssize_t;


#define SEEK_SET	0	/* seek relative to beginning of file */
#define SEEK_CUR	1	/* seek relative to current file position */
#define SEEK_END	2	/* seek relative to end of file */

#define	LOCK_SH	1	/* Shared lock.  */
#define	LOCK_EX	2 	/* Exclusive lock.  */
#define	LOCK_UN	8	/* Unlock.  */

/* Standard File Access Modes (2 low bits) */
#define O_RDONLY        00000000    /* Hex: 0x0000 */
#define O_WRONLY        00000001    /* Hex: 0x0001 */
#define O_RDWR          00000002    /* Hex: 0x0002 */

/* File Creation Flags */
#define O_CREAT         00000100    /* Hex: 0x0040 */
#define O_EXCL          00000200    /* Hex: 0x0080 */
#define O_NOCTTY        00000400    /* Hex: 0x0100 */
#define O_TRUNC         00001000    /* Hex: 0x0200 */
#define O_APPEND        00002000    /* Hex: 0x0400 */
#define O_NONBLOCK      00004000    /* Hex: 0x0800 */
#define O_NDELAY        O_NONBLOCK
#define O_SYNC          04010000    /* Hex: 0x101000 */
#define O_FSYNC         O_SYNC
#define O_ASYNC         00020000    /* Hex: 0x2000 */
#define O_DIRECT        00040000    /* Hex: 0x4000 */
#define O_DIRECTORY     00200000    /* Hex: 0x10000 */
#define O_NOFOLLOW      00400000    /* Hex: 0x20000 */
#define O_NOATIME       01000000    /* Hex: 0x40000 */
#define O_CLOEXEC       02000000    /* Hex: 0x80000 */

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
void close(int fd);
ssize_t getrandom(char *buf, size_t count);
void *memcpy(char *dest, char* src, size_t n);
void *memset(char* src, int c, size_t n);
int itoa(int num, char *str, int base);
ssize_t execve(const char *path, const char *argv, const char *envp);
int fork();
size_t lseek(int fd, size_t offset, int whence);
int wait(int pid, int wstatus);
size_t strlen(const char* s);
ssize_t ioctl(unsigned int fd, unsigned int cmd, unsigned long args);
ssize_t open(const char *path, int flags, int mode);
char* strdup(char* str);
int rename(char* oldpath, char* newpath);
int atoi(char* str);

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
