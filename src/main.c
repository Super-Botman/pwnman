// #include "lib/io.h"
#include "lib/malloc.h"

int main(int argc, char *argv[], char *envp[]) {
 void* c1 = malloc(0x40);
 c1 = realloc(c1, 0x60);
 free(c1);

 // read(0, 0, 1);
 
 return 0;
}
