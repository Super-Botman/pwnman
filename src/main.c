#include "libotman.h"

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

void list(struct entries *entries) {
  char title[65];
  printf("number of entries: %d\n", entries->count);

  char *db = entries->db;
  for (int i = 0; i < entries->count; i++) {
    struct entry *entry = (struct entry *)db;
    memset(&title[0], 0, sizeof(title));
    if (entry->tlen >= sizeof(title)) {
      puts("invalid entry");
      return;
    }
    memcpy(&title[0], entry->title, entry->tlen);
    printf("%d: %s\n", i, title);
    db += sizeof(struct entry);
  }
  return;
}

void show(struct entries *entries, size_t idx) {
  struct entry *entry =
      (struct entry *)(entries->db + (sizeof(struct entry) * idx));

  char title[65];
  memset(&title[0], 0, sizeof(title));
  if (entry->tlen >= sizeof(title)) {
    puts("invalid entry");
    return;
  }
  memcpy(&title[0], entry->title, entry->tlen);
  printf("title: %s\n", title);

  char *key = master_passwd;
  if (!key) {
    printf("Enter password > ");
    key = getline();
  }
  
  size_t password_len = strlen(key);
  struct fields fields;
  unsigned char *dest = (unsigned char *)&fields;
  const unsigned char *src = (const unsigned char *)&entry->fields;

  for (size_t i = 0; i < sizeof(struct fields); i++) {
    dest[i] = src[i] ^ key[i % password_len];
  }

  u32 sig = crc32((char *)&fields + 4, sizeof(struct fields) - 4);
  if (fields.crc32 != sig) {
    puts("invalid entry or password");
    return;
  }

  if (!master_passwd) {
    char c[2];
    printf("Save password [Y/n]: ");
    char* confirm = getline();
    if (strcmp(confirm, "n") == 0) {
      memset(key, 0, password_len);
      free(key);
    } else master_passwd = key;
    free(confirm);
  }

  if (fields.ulen > sizeof(fields.username) ||
      fields.plen > sizeof(fields.password)) {
    puts("invalid entry");
    return;
  }

  char tmp[65];
  memset(&tmp[0], 0, sizeof(tmp));
  memcpy(&tmp[0], (char *)&fields.username, fields.ulen);
  printf("username: %s\n", tmp);

  memset(&tmp[0], 0, sizeof(tmp));
  memcpy(&tmp[0], (char *)&fields.password, fields.plen);
  printf("password: %s\n", tmp);
}

int opendb(char *path, struct entries *entries) {
  // TODO: flock
  int fd = open(path, 0, 0);
  if (fd < 0)
    puts("error while opening the db");

  size_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *file_content = malloc(file_size);
  if (read(fd, file_content, file_size) != file_size) {
    puts("error while reading the db");
    return -1;
  }

  if (strncmp(file_content, "B0T", 4) != 0) {
    puts("invalid db");
    return -1;
  }

  file_content += 16;
  size_t entries_size = file_size - 16;
  size_t entries_count = entries_size / sizeof(struct entry);
  entries->db = file_content;
  entries->count = entries_count;

  return fd;
}

int main(int argc, char *argv[], char *envp[]) {
  struct entries entries;
  memset((char *)&entries, 0, sizeof(struct entries));
  int fd = 0;

  while (1) {
    printf("> ");
    char *line = getline();

    if (strcmp(line, "exit") == 0) {
      break;
    } else if (strcmp(line, "help") == 0) {
      puts("PASSWORD MANAGER 1.0");
      puts("open <file>: open the db specified by file");
      puts("exit: exit the program");
    } else if (strncmp(line, "open ", 5) == 0) {
      int fd = opendb(line + 5, &entries);
      if (fd < 0)
        puts("error while opening the db");
      else
        puts("done");
    } else if (strcmp(line, "list") == 0) {
      if (entries.count != 0)
        list(&entries);
      else
        puts("db not initialized, did you open it ?");
    } else if (strncmp(line, "show ", 5) == 0) {
      // TODO: atoi
      if (entries.count != 0)
        show(&entries, *(line + 5) - '0');
      else
        puts("db not initialized, did you open it ?");
    } else {
      puts("unkown command");
    }

    free(line);
  }
  puts("bye");
  return 0;
}
