#include "libotman.h"

int opendb(char *path) {
  int fp = open(path, 0, 0);
  if (fp < 0)
    puts("error while opening the db");

  char magic[4];
  if (read(fp, (char *)&magic, 4) != 4) {
    puts("error while reading the db");
    return -1;
  };

  if (strcmp(magic, "B0T") != 0){
    puts("invalid db");
    return -1;
  }
}

int main(int argc, char *argv[], char *envp[]) {
  while (1) {
    printf("> ");
    char *line = getline();

    if (strcmp(line, "exit") == 0)
      break;
    else if (strcmp(line, "help") == 0) {
      puts("PASSWORD MANAGER 1.0");
    } else if (strcmp(line, "help") == 0) {
      puts("PASSWORD MANAGER 1.0");
    } else if (strncmp(line, "open ", 5) == 0) {
      opendb(line + 5);
    } else
      puts("unkown command");

    free(line);
  }
  puts("bye");
  return 0;
}
