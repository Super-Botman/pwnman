// #include "lib/io.h"
#include "lib/malloc.h"

int main(int argc, char *argv[], char *envp[]) {
 void* c1 = malloc(0x90);
 realloc(c1, 0x50);
 malloc(0x10);
 
 return 0;
}

// [MALLOC] ret: 0x404010, size: 0xa0
// [FREE] ptr: 0x404010, size: 0xa0
// [MALLOC] ret: 0x404010, size: 0x50
// [REALLOC] ret: 0x404010, size: 0x60
