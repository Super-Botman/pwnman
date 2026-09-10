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
    {"help", "help: show this help", help},
    {"open", "open <file>: open the db specified by file", opendb},
    {"create", "create <file>: create a new db called file", createdb},
    {"close", "close: save and close the actual db", closedb},
    {"save", "save: save the actual db", savedb},
    {"list", "list: list all the entries of a db", list},
    {"show", "show <idx>: show the password of one entry", show},
    {"delete", "delete <idx>: delete one entry", delete},
    {"edit", "edit <idx>: edit one entry", edit},
    {"new", "new: create a new entry", new},
    {"exit", "exit: exit without saving", exitdb},
    {"search", "search: search for an element", search}
};
size_t num_commands = sizeof(commands) / sizeof(commands[0]);

int main(int argc, char *argv[], char *envp[]) {
  puts("PASSWORD MANAGER 1.0");

  (void)argc;
  (void)argv;
  (void)envp;
  struct db db;
  memset((char *)&db, 0, sizeof(struct db));

  char *line = 0;

  while (1) {
    if (!db.path)
      printf("> ");
    else if (db.edited)
      printf("[*%s]> ", db.path);
    else
      printf("[%s]> ", db.path);

    line = getline();
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
    puts("unknown command");

  end:
    free(line);
  }

  puts("bye");
  return 0;
}
