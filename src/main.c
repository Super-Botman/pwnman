#include "libotman.h"

int main(int argc, char *argv[], char *envp[]) {
  while(1){
    printf("> ");
    char* line = getline();

    if(strcmp(line, "exit") == 0) break;
    else if(strncmp(line, "echo ", 5) == 0) printf("%s\n", line+5);
    else puts("unknown command");

    free(line);
  }
  return 0;
}
