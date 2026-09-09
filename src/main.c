#include "../lib/libotman.h"

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

struct entries {
  size_t count;
  char *db;
};

char *master_passwd;

int crypt(struct entry *entry, struct fields *fields, int encrypt) {
  char *key = master_passwd;
  if (!key) {
    printf("Enter password > ");
    key = getline();
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

  for (size_t i = 0; i < sizeof(struct fields); i++) {
    dest[i] = src[i] ^ key[i % password_len];
  }

  if (!encrypt) {
    u32 sig = crc32((char *)&fields->ulen, sizeof(struct fields) - 4);
    if (fields->crc32 != sig) {
      return -1;
    }
  }

  if (!master_passwd) {
    char c[2];
    printf("Save password [Y/n]: ");
    char *confirm = getline();
    if (strcmp(confirm, "n") == 0) {
      memset(key, 0, password_len);
      free(key);
    } else
      master_passwd = key;
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

void list(struct entries *entries) {
  char title[65];
  printf("number of entries: %d\n", entries->count);

  char *db = entries->db;
  for (int i = 0; i < entries->count; i++) {
    struct entry *entry = (struct entry *)db;
    get_title(entry, &title[0]);
    printf("%d: %s\n", i, title);
    db += sizeof(struct entry);
  }
  return;
}

void show(struct entries *entries, size_t idx) {
  struct entry *entry =
      (struct entry *)(entries->db + (sizeof(struct entry) * idx));

  char title[65];
  get_title(entry, &title[0]);
  printf("title: %s\n", title);

  struct fields fields;
  if (crypt(entry, &fields, 0) < 0) {
    puts("invalid entry or password");
    return;
  }

  char tmp[65];
  get_user(&fields, &tmp[0]);
  printf("username: %s\n", tmp);

  get_pass(&fields, &tmp[0]);
  printf("password: %s\n", tmp);
}

void edit(struct entries *entries, size_t idx) {
  struct entry *entry =
      (struct entry *)(entries->db + (sizeof(struct entry) * idx));

  struct fields new_fields;
  if (crypt(entry, &new_fields, 0) < 0) {
    puts("invalid password");
    return;
  }

  char title[65];
  int title_len = entry->tlen;
  get_title(entry, &title[0]);
  while (1) {
    printf("[%s]> ", title);
    char *line = getline();
    size_t len = strlen(line);

    if (strcmp(line, "help") == 0) {
      puts("Edit mode");
      puts("title: change title");
      puts("username: change username");
      puts("password: change password");
      puts("save: save the updated entry");
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
      break;
    } else if (strcmp(line, "exit") == 0) {
      free(line);
      return;
    } else {
      puts("unknown command");
    }

    free(line);
  }
  return;
}

void new(struct entries *entries) {
  if (!master_passwd && entries->count > 0) {
    struct fields fields;
    if (crypt((struct entry *)entries->db, &fields, 0) < 0) {
      puts("invalid password");
      return;
    }
  }

  entries->count += 1;
  size_t new_size = entries->count * sizeof(struct entry);
  entries->db = realloc(entries->db, new_size);

  struct entry *new_entry =
      (struct entry *)(entries->db + new_size - sizeof(struct entry));
  memset((char *)new_entry, 0, sizeof(struct entry));

  printf("Title (max 64) > ");
  size_t len = read(0, new_entry->title, 64);
  if (new_entry->title[len - 1] == '\n') {
    new_entry->title[len - 1] = '\0';
    len -= 1;
  }
  new_entry->tlen = len;

  struct fields *new_fields = &new_entry->fields;
  printf("Username (max 64) > ");
  len = read(0, new_fields->username, 64);
  if (new_fields->username[len - 1] == '\n') {
    new_fields->username[len - 1] = '\0';
    len -= 1;
  }
  getrandom(new_fields->username + len, 64 - len);
  new_fields->ulen = len;

  printf("Password (max 64) > ");
  len = read(0, new_fields->password, 64);
  if (new_fields->password[len - 1] == '\n') {
    new_fields->password[len - 1] = '\0';
    len -= 1;
  }
  getrandom(new_fields->password + len, 64 - len);
  new_fields->plen = len;

  crypt(new_entry, new_fields, 1);
}

int opendb(char *path, struct entries *entries) {
  int fd = open(path, O_RDWR, 0);
  if (fd < 0) {
    puts("error while opening the db");
    return -1;
  }

  size_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *file_content = malloc(file_size);
  if (read(fd, file_content, file_size) != file_size) {
    puts("error while reading the db");
    free(file_content);
    return -1;
  }

  if (strncmp(file_content, "B0T", 4) != 0) {
    puts("invalid db");
    free(file_content);
    return -1;
  }

  file_content += 16;
  size_t entries_size = file_size - 16;
  size_t entries_count = entries_size / sizeof(struct entry);
  entries->db = file_content;
  entries->count = entries_count;

  return fd;
}

void save(struct entries *entries, char *path) {
  int tmp = open(".tmp.db", O_WRONLY | O_CREAT, 0644);
  int len = entries->count * (sizeof(struct entry)) + 16;
  char *file_content = (char *)(entries->db - 16);
  if (write(tmp, file_content, len) != len) {
    puts("failed to write");
    return;
  }
  if (rename(".tmp.db", path) < 0) {
    puts("failed to save");
    return;
  }
}

int main(int argc, char *argv[], char *envp[]) {
  struct entries entries;
  memset((char *)&entries, 0, sizeof(struct entries));
  int fd = 0;
  char *dbpath;

  while (1) {
    printf("> ");
    char *line = getline();

    if (strcmp(line, "help") == 0) {
      puts("PASSWORD MANAGER 1.0");
      puts("open <file>: open the db specified by file");
      puts("show <idx>: show the password");
      puts("list: show the list of password stored");
      puts("exit: exit the program");
    } else if (strncmp(line, "open ", 5) == 0) {
      if (entries.db) {
        puts("db already opened, close it before opening another");
      } else {
        char *path = line + 5;
        dbpath = strdup(path);
        fd = opendb(path, &entries);
        if (fd > 0)
          puts("success");
      }
    } else if (strcmp(line, "list") == 0) {
      if (entries.count != 0)
        list(&entries);
      else
        puts("no entries in db");
    } else if (strncmp(line, "show ", 5) == 0) {
      if (entries.count != 0)
        // TODO: atoi
        show(&entries, *(line + 5) - '0');
      else
        puts("no entries in db");
    } else if (strncmp(line, "edit ", 5) == 0) {
      if (entries.count != 0)
        // TODO: atoi
        edit(&entries, *(line + 5) - '0');
      else
        puts("no entries in db");
    } else if (strcmp(line, "new") == 0) {
      if (entries.db)
        new(&entries);
      else
        puts("db not initialized, did you open it ?");
    } else if (strcmp(line, "save") == 0) {
      if (entries.db)
        save(&entries, dbpath);
      else
        puts("db not initialized, did you open it ?");
    } else if (strcmp(line, "close") == 0) {
      if (entries.db) {
        save(&entries, dbpath);
        free(entries.db - 16);
        memset((char *)&entries, 0, sizeof(struct entries));
        fd = 0;
        free(dbpath);
        dbpath = 0;
      } else {
        puts("db not initialized, did you open it ?");
      }
    } else if (strcmp(line, "exit") == 0) {
      save(&entries, dbpath);
      break;
    } else {
      puts("unkown command");
    }

    free(line);
  }
  puts("bye");
  return 0;
}
