#include "types.h"

int strcmp(const char *s1, const char *s2){
  int ret = 0;
  while(*s1 || *s2) ret += *s1 == *s2;
  return ret;
}
