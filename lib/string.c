#include "libotman.h"

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, int n) {
  int ret = 0;
  for (int i = 0; i < n && *s1 && *s2; i++)
    ret += *s1 == *s2;
  return ret;
}
