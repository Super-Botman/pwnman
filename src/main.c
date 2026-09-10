#include "../lib/libotman.h"
#include "./ioctls.h"
#include "./termbits.h"

typedef unsigned int u32;
typedef unsigned short u16;

struct fields {
  u32 crc32;
  u16 ulen;
  u16 plen;
  char username[64];
  char password[64];
};

struct entry {
  u32 tlen;
  u32 padding;
  char title[64];
  struct fields fields;
};

struct db {
  int edited;
  char *path;
  int fd;
  size_t count;
  char *entries;
};

struct command {
  char *command;
  char *usage;
  void (*func)(struct db *, char *arg);
};

char *master_passwd;

extern struct command commands[];
extern size_t num_commands;

void help(struct db *_, char *__);
void opendb(struct db *db, char *path);
void createdb(struct db *db, char *path);
void closedb(struct db *db, char *path);
void savedb(struct db *db, char *_);
void list(struct db *db, char *_);
void show(struct db *db, char *arg);
void delete(struct db *db, char *arg);
void edit(struct db *db, char *arg);
void new(struct db *db, char *_);
void exitdb(struct db *_, char *__);

void echo_off() {
  struct termios state;
  (void)ioctl(0, (int)TCGETS, (long)&state);
  state.c_lflag &= ~ECHO;
  (void)ioctl(0, (int)TCSETS, (long)&state);
}

void echo_on() {
  struct termios state;
  (void)ioctl(0, (int)TCGETS, (long)&state);
  state.c_lflag |= ECHO;
  (void)ioctl(0, (int)TCSETS, (long)&state);
}

int crypt(struct entry *entry, struct fields *fields, int encrypt) {
  char *key = master_passwd;
  if (!key) {
    printf("Master password: ");
    echo_off();
    key = getline();
    echo_on();
    puts("");
  }

  if (encrypt)
    fields->crc32 = crc32((char *)&fields->ulen, sizeof(struct fields) - 4);

  size_t password_len = strlen(key);

  unsigned char *dest;
  const unsigned char *src;
  if (!encrypt) {
    dest = (unsigned char *)fields;
    src = (const unsigned char *)&entry->fields;
  } else {
    dest = (unsigned char *)&entry->fields;
    src = (const unsigned char *)fields;
  }

  for (size_t i = 0; i < sizeof(struct fields); i++)
    dest[i] = src[i] ^ key[i % password_len];

  if (!encrypt) {
    u32 sig = crc32((char *)&fields->ulen, sizeof(struct fields) - 4);
    if (fields->crc32 != sig)
      return -1;
  }

  if (!master_passwd) {
    char c[2];
    printf("Save password [Y/n]: ");
    char *confirm = getline();
    if (strcmp(confirm, "n") == 0) {
      memset(key, 0, password_len);
      free(key);
    } else {
      master_passwd = key;
    }
    free(confirm);
  }
  return 0;
}

void get_title(struct entry *entry, char *title) {
  memset(title, 0, sizeof(entry->title));
  if (entry->tlen >= sizeof(entry->title)) {
    puts("invalid entry");
    return;
  }
  memcpy(&title[0], entry->title, entry->tlen);
}

void get_pass(struct fields *fields, char *pass) {
  if (fields->plen > sizeof(fields->password)) {
    puts("invalid entry");
    return;
  }
  memset(pass, 0, sizeof(fields->password));
  memcpy(pass, fields->password, fields->plen);
}

void get_user(struct fields *fields, char *user) {
  if (fields->ulen > sizeof(fields->username)) {
    puts("invalid entry");
    return;
  }
  memset(user, 0, sizeof(fields->username));
  memcpy(user, fields->username, fields->ulen);
}

void list(struct db *db, char *_) {
  if (db->count == 0) {
    puts("no entry in db");
    return;
  }

  char title[65];
  printf("number of entries: %d\n", db->count);

  char *entries = db->entries;
  for (int i = 0; i < db->count; i++) {
    struct entry *entry = (struct entry *)entries;
    get_title(entry, &title[0]);
    printf("%d: %s\n", i, title);
    entries += sizeof(struct entry);
  }
}

static void draw_line(int t, int u, int p, int s) {
  putc(s);
  for (int i = 0; i < t + 2; i++)
    putc('-');
  putc(s);
  for (int i = 0; i < u + 2; i++)
    putc('-');
  putc(s);
  for (int i = 0; i < p + 2; i++)
    putc('-');
  putc(s);
  putc('\n');
}
static void print_padded(const char *str, int width) {
  int len = (int)strlen(str);
  put(str);
  for (int i = len; i < width; i++)
    putc(' ');
}

static void print_row(int t, const char *title, int u, const char *username,
                      int p, const char *password) {
  printf("| ");
  print_padded(title, t);
  printf(" | ");
  print_padded(username, u);
  printf(" | ");
  print_padded(password, p);
  printf(" |\n");
}

void print_table(struct entry *entry, struct fields fields) {
  char title[65], username[65], password[65];
  get_title(entry, title);
  get_user(&fields, username);
  get_pass(&fields, password);

  /* largeur max entre le contenu et l'en-tête */
  int tlen = (int)strlen(title) > 5 ? (int)strlen(title) : 5;
  int ulen = (int)strlen(username) > 8 ? (int)strlen(username) : 8;
  int plen = (int)strlen(password) > 8 ? (int)strlen(password) : 8;

  draw_line(tlen, ulen, plen, '+');
  print_row(tlen, "title", ulen, "username", plen, "password");
  draw_line(tlen, ulen, plen, '|');
  print_row(tlen, title, ulen, username, plen, password);
  draw_line(tlen, ulen, plen, '+');
}

void show(struct db *db, char *arg) {
  if (db->count == 0) {
    puts("no entry in db");
    return;
  }

  while (*arg == ' ')
    arg++;
  int idx = atoi(arg);

  if (idx >= db->count) {
    puts("invalid index");
    return;
  }

  struct entry *entry =
      (struct entry *)(db->entries + (sizeof(struct entry) * idx));

  struct fields fields;
  if (crypt(entry, &fields, 0) < 0) {
    puts("invalid entry or password");
    return;
  }

  print_table(entry, fields);
}

void edit(struct db *db, char *arg) {
  while (*arg == ' ')
    arg++;
  int idx = atoi(arg);

  if (idx >= db->count) {
    puts("invalid index");
    return;
  }

  struct entry *entry =
      (struct entry *)(db->entries + (sizeof(struct entry) * idx));

  struct fields new_fields;
  if (crypt(entry, &new_fields, 0) < 0) {
    puts("invalid password");
    return;
  }

  char title[65];
  int title_len = entry->tlen;
  get_title(entry, &title[0]);

  while (1) {
    printf("[%s ~ %s]> ", db->path, title);

    char *line = getline();
    size_t len = strlen(line);

    if (strcmp(line, "help") == 0) {
      puts("Edit mode");
      puts("title <title>: change title");
      puts("username <username>: change username");
      puts("password <password>: change password");
      puts("save: save the updated entry into memory");
      puts("exit: exit without saving");
    } else if (strncmp(line, "title ", 6) == 0) {
      memset(&title[0], 0, sizeof(title));
      memcpy(&title[0], line + 6, len - 6);
      title_len = len - 6;
      puts("success");
    } else if (strncmp(line, "username ", 9) == 0) {
      len -= 9;
      memcpy(new_fields.username, line + 9, len);
      new_fields.ulen = len;
      getrandom(new_fields.username + len, 64 - len);
      puts("success");
    } else if (strncmp(line, "password ", 9) == 0) {
      len -= 9;
      memcpy(new_fields.password, line + 9, len);
      new_fields.plen = len;
      getrandom(new_fields.password + len, 64 - len);
      puts("success");
    } else if (strcmp(line, "save") == 0) {
      memcpy(entry->title, &title[0], title_len);
      entry->tlen = title_len;
      crypt(entry, &new_fields, 1);
      free(line);
      db->edited |= 1;
      break;
    } else if (strcmp(line, "exit") == 0) {
      free(line);
      return;
    } else {
      puts("unknown command");
    }

    free(line);
  }
}

void new(struct db *db, char *_) {
  if (!master_passwd && db->count > 0) {
    struct fields fields;
    if (crypt((struct entry *)db->entries, &fields, 0) < 0) {
      puts("invalid password");
      return;
    }
  }

  db->count += 1;
  size_t new_size = db->count * sizeof(struct entry);
  char *file_content = db->entries - 16;
  db->entries = realloc(file_content, new_size + 16) + 16;

  struct entry *new_entry =
      (struct entry *)(db->entries + new_size - sizeof(struct entry));
  memset((char *)new_entry, 0, sizeof(struct entry));

  printf("Title (max 64): ");
  size_t len = read(0, new_entry->title, 64);
  if (new_entry->title[len - 1] == '\n') {
    new_entry->title[len - 1] = '\0';
    len -= 1;
  }
  new_entry->tlen = len;

  struct fields *new_fields = &new_entry->fields;

  printf("Username (max 64): ");
  len = read(0, new_fields->username, 64);
  if (len > 0 && new_fields->username[len - 1] == '\n') {
    len -= 1;
  }
  new_fields->username[len] = '\0';
  new_fields->ulen = len;
  getrandom(new_fields->username + len + 1, 64 - len);

  printf("Password (max 64): ");
  len = read(0, new_fields->password, 64);
  if (len > 0 && new_fields->password[len - 1] == '\n') {
    len -= 1;
  }
  new_fields->password[len] = '\0';
  new_fields->plen = len;
  getrandom(new_fields->password + len + 1, 64 - len);

  crypt(new_entry, new_fields, 1);
  db->edited |= 1;
}

void delete(struct db *db, char *arg) {
  while (*arg == ' ')
    arg++;
  int idx = atoi(arg);

  if (idx >= db->count) {
    puts("invalid index");
    return;
  }

  size_t offset = idx * sizeof(struct entry);
  size_t size = db->count * sizeof(struct entry);

  struct entry *entry = (struct entry *)(db->entries + offset);
  struct entry *next = entry + 1;

  size_t diff = size - (offset + sizeof(struct entry));
  memcpy((char *)entry, (char *)next, diff);

  db->count -= 1;
  size_t new_size = db->count * sizeof(struct entry);
  char *file_content = db->entries - 16;
  db->entries = realloc(file_content, new_size - 16) + 16;
  db->edited |= 1;
}

void opendb(struct db *db, char *path) {
  if (db->entries) {
    puts("db already opened, close it before opening another");
    return;
  }

  while (*path == ' ')
    path++;

  int fd = open(path, O_RDWR, 0);
  if (fd < 0) {
    puts("error while opening the db");
    return;
  }

  size_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *file_content = malloc(file_size);
  if (read(fd, file_content, file_size) != file_size) {
    puts("error while reading the db");
    free(file_content);
    return;
  }

  if (strncmp(file_content, "B0T", 4) != 0) {
    puts("invalid db");
    free(file_content);
    return;
  }

  file_content += 16;
  size_t db_size = file_size - 16;
  size_t db_count = db_size / sizeof(struct entry);
  db->entries = file_content;
  db->count = db_count;
  db->fd = fd;
  db->path = strdup(path);
}

void savedb(struct db *db, char *_) {
  if (!db->entries) {
    puts("db not initialized");
    return;
  }

  int tmp = open(".tmp.db", O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (tmp < 0) {
    puts("cannot save");
    return;
  }

  int len = db->count * (sizeof(struct entry)) + 16;
  char *file_content = (char *)(db->entries - 16);
  if (write(tmp, file_content, len) != len) {
    puts("failed to write");
    return;
  }
  if (rename(".tmp.db", db->path) < 0) {
    puts("failed to save");
    return;
  }
  db->edited = 0;
}

void createdb(struct db *db, char *path) {
  if (db->entries) {
    puts("db already opened, close it before opening another");
    return;
  }

  while (*path == ' ')
    path++;

  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (fd < 0) {
    puts("failed to create the new db");
    return;
  }

  char header[16] = "B0T\0\0\0\0\0\0\0\0\0\0\0\0\0";
  if (write(fd, &header[0], 16) != 16) {
    puts("failed to write header");
    return;
  }
  close(fd);
  opendb(db, path);
}

void closedb(struct db *db, char *path) {
  if (!db->entries) {
    puts("db not initialized");
    return;
  }

  savedb(db, "");
  free(db->entries - 16);
  free(db->path);
  memset((char *)db, 0, sizeof(struct db));
}

void exitdb(struct db *_, char *__) { exit(0); }

void help(struct db *_, char *__) {
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
};
size_t num_commands = sizeof(commands) / sizeof(commands[0]);

int main(int argc, char *argv[], char *envp[]) {
  puts("PASSWORD MANAGER 1.0");

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

      commands[i].func(&db, line + cmd_len);
      goto end;
    }
    puts("unknown command");

  end:
    free(line);
  }

  puts("bye");
  return 0;
}

// TODO: no password = segfault;
