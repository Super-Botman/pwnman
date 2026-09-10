#include "pwnman.h"

struct command {
  char *command;
  char *usage;
  void (*func)(struct db *, char *arg);
};

char *master_passwd = 0;
extern struct command commands[];
extern size_t num_commands;

void exitdb(struct db *_, char *__) { (void)_; (void)__; exit(0); }

void help(struct db *_, char *__) {
  (void)_; (void)__;
  for (size_t i = 0; i < num_commands; i++)
    puts(commands[i].usage);
}

struct command commands[] = {
    {"help", BOLD BLUE"help:"RESET" show this help", help},
    {"open", BOLD BLUE"open <file>:"RESET" open the db specified by file", opendb},
    {"create", BOLD BLUE"create <file>:"RESET" create a new db called file", createdb},
    {"close", BOLD BLUE"close:"RESET" save and close the actual db", closedb},
    {"save", BOLD BLUE"save:"RESET" save the actual db", savedb},
    {"list", BOLD BLUE"list:"RESET" list all the entries of a db", list},
    {"search", BOLD BLUE"search <title>:"RESET"search for an element by title", search},
    {"show", BOLD BLUE"show <idx>:"RESET"show the password of one entry", show},
    {"delete", BOLD BLUE"delete <idx>:"RESET" delete one entry", delete},
    {"edit", BOLD BLUE"edit <idx>:"RESET" edit one entry", edit},
    {"new", BOLD BLUE"new:"RESET" create a new entry", new},
    {"exit", BOLD BLUE"exit:"RESET" exit without saving", exitdb},
};
size_t num_commands = sizeof(commands) / sizeof(commands[0]);

int main(int argc, char *argv[], char *envp[]) {
  (void)argc;
  (void)argv;
  (void)envp;

  puts(BOLD MAGENTA
    "$$$$$$$\\  $$\\      $$\\ $$\\   $$\\ $$\\      $$\\  $$$$$$\\  $$\\   $$\\ \n"
    "$$  __$$\\ $$ | $\\  $$ |$$$\\  $$ |$$$\\    $$$ |$$  __$$\\ $$$\\  $$ |\n"
    "$$ |  $$ |$$ |$$$\\ $$ |$$$$\\ $$ |$$$$\\  $$$$ |$$ /  $$ |$$$$\\ $$ |\n"
    "$$$$$$$  |$$ $$ $$\\$$ |$$ $$\\$$ |$$\\$$\\$$ $$ |$$$$$$$$ |$$ $$\\$$ |\n"
    "$$  ____/ $$$$  _$$$$ |$$ \\$$$$ |$$ \\$$$  $$ |$$  __$$ |$$ \\$$$$ |\n"
    "$$ |      $$$  / \\$$$ |$$ |\\$$$ |$$ |\\$  /$$ |$$ |  $$ |$$ |\\$$$ |\n"
    "$$ |      $$  /   \\$$ |$$ | \\$$ |$$ | \\_/ $$ |$$ |  $$ |$$ | \\$$ |\n"
    "\\__|      \\__/     \\__|\\__|  \\__|\\__|     \\__|\\__|  \\__|\\__|  \\__|\n\n"
    BOLD BLUE"                           [ 0.1.0 ]"RESET);

  struct db db;
  memset((char *)&db, 0, sizeof(struct db));

  char *line = 0;
  size_t len = 0;

  while (1) {
    if (!db.path)
      printf(BLUE"> " RESET);
    else if (db.edited)
      printf(BOLD BLUE "[*%s]> " RESET, db.path);
    else
      printf(BLUE "[%s]> " RESET, db.path);

    getline(&line, &len);
    for (size_t i = 0; i < num_commands; i++) {
      struct command c = commands[i];
      size_t cmd_len = strlen(c.command);

      if (strncmp(line, c.command, cmd_len) != 0)
        continue;

      char* arg = line + cmd_len;
      while (*arg == ' ') arg++;
      commands[i].func(&db, arg);
      goto end;
    }
    puts(RED"unknown command"RESET);

  end:
    free(line);
    line = 0;
  }

  puts("bye");
  return 0;
}
