#include "libotman.h"

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, int n) {
  int i = 1;
  while((i < n) && (*s1 == *s2)){
    s1++;
    s2++;
    i++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

size_t strlen(const char* s){
  const char* start = s;
  while(*s) ++s;
  return s-start; 
}
