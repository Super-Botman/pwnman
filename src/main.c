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

struct db {
  int edited;
  char *path;
  size_t count;
  char *entries;
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

void list(struct db *db) {
  char title[65];
  printf("number of entries: %d\n", db->count);

  char *entries = db->entries;
  for (int i = 0; i < db->count; i++) {
    struct entry *entry = (struct entry *)entries;
    get_title(entry, &title[0]);
    printf("%d: %s\n", i, title);
    entries += sizeof(struct entry);
  }
  return;
}

void show(struct db *db, size_t idx) {
  struct entry *entry =
      (struct entry *)(db->entries + (sizeof(struct entry) * idx));

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

int edit(struct db *db, size_t idx) {
  struct entry *entry =
      (struct entry *)(db->entries + (sizeof(struct entry) * idx));

  struct fields new_fields;
  if (crypt(entry, &new_fields, 0) < 0) {
    puts("invalid password");
    return 0;
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
      break;
    } else if (strcmp(line, "exit") == 0) {
      free(line);
      return 0;
    } else {
      puts("unknown command");
    }

    free(line);
  }
  return 1;
}

int new(struct db *db) {
  if (!master_passwd && db->count > 0) {
    struct fields fields;
    if (crypt((struct entry *)db->entries, &fields, 0) < 0) {
      puts("invalid password");
      return 0;
    }
  }

  db->count += 1;
  size_t new_size = db->count * sizeof(struct entry);
  char *file_content = db->entries - 16;
  db->entries = realloc(file_content, new_size + 16) + 16;

  struct entry *new_entry =
      (struct entry *)(db->entries + new_size - sizeof(struct entry));
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
  return 1;
}

int opendb(struct db *db) {
  int fd = open(db->path, O_RDWR, 0);
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
  size_t db_size = file_size - 16;
  size_t db_count = db_size / sizeof(struct entry);
  db->entries = file_content;
  db->count = db_count;

  return fd;
}

int save(struct db *db, char *path) {
  int tmp = open(".tmp.db", O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (tmp < 0) {
    puts("cannot save");
    return 1;
  }
  int len = db->count * (sizeof(struct entry)) + 16;
  char *file_content = (char *)(db->entries - 16);
  if (write(tmp, file_content, len) != len) {
    puts("failed to write");
    return 1;
  }
  if (rename(".tmp.db", path) < 0) {
    puts("failed to save");
    return 1;
  }
  return 0;
}

int createdb(struct db *db) {
  int fd = open(db->path, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (fd < 0) {
    puts("failed to create the new db");
    return -1;
  }

  char header[16] = "B0T\0\0\0\0\0\0\0\0\0\0\0\0\0";
  if (write(fd, &header[0], 16) != 16) {
    puts("failed to write header");
    return -1;
  }
  close(fd);
  return 0;
}

int delete(struct db *db, int idx){
  size_t offset = idx*sizeof(struct entry); 
  size_t size = db->count*sizeof(struct entry);

  struct entry *entry = (struct entry*)(db->entries + offset);
  struct entry *next = entry + 1; 

  size_t diff = size - (offset+sizeof(struct entry));
  memcpy((char*)entry, (char*)next, diff);

  db->count -= 1;
  size_t new_size = db->count * sizeof(struct entry);
  char *file_content = db->entries - 16;
  db->entries = realloc(file_content, new_size - 16) + 16;
  return 1;
}

int main(int argc, char *argv[], char *envp[]) {
  struct db db;
  memset((char *)&db, 0, sizeof(struct db));
  int fd = 0;

  puts("PASSWORD MANAGER 1.0");
  while (1) {
    if (!db.path)
      printf("> ");
    else if (db.edited)
      printf("[*%s]> ", db.path);
    else
      printf("[%s]> ", db.path);

    char *line = getline();

    if (strcmp(line, "help") == 0) {
      puts("open <file>: open the db specified by file");
      puts("close: close the db");
      puts("save: save the db");
      puts("list: show the list of password stored");
      puts("show <idx>: show an entry");
      puts("edit <idx>: edit an entry");
      puts("delete <idx>: delete an entry");
      puts("new: add an entry");
      puts("exit: save db and exit the program");
    } else if (strncmp(line, "open ", 5) == 0) {
      if (db.entries) {
        puts("db already opened, close it before opening another");
      } else {
        char *path = line + 5;
        db.path = strdup(path);
        fd = opendb(&db);
        if (fd < 0) {
          free(db.path);
          db.path = 0;
        }
      }
    } else if (strncmp(line, "create ", 7) == 0) {
      if (db.entries) {
        puts("db already opened, close it before opening another");
      } else {
        char *path = line + 7;
        db.path = strdup(path);
        createdb(&db);
        fd = opendb(&db);
        if (fd < 0) {
          free(db.path);
          db.path = 0;
        }
      }
    } else if (strcmp(line, "list") == 0) {
      if (db.count != 0)
        list(&db);
      else
        puts("no entry in db");
    } else if (strncmp(line, "show ", 5) == 0) {
      if (db.count != 0) {
        int idx = atoi(line + 5);
        if (idx < db.count)
          show(&db, idx);
        else
          puts("invalid index");
      } else {
        puts("no entry in db");
      }
    } else if (strncmp(line, "edit ", 5) == 0) {
      if (db.count != 0) {
        int idx = atoi(line + 5);
        if (idx < db.count)
          db.edited |= edit(&db, idx);
        else
          puts("invalid index");
      } else {
        puts("no entry in db");
      }
    } else if (strncmp(line, "delete ", 7) == 0) {
      if (db.count != 0) {
        int idx = atoi(line + 7);
        if (idx < db.count)
          db.edited |= delete(&db, idx);
        else
          puts("invalid index");
      } else {
        puts("no entry in db");
      }
    } else if (strcmp(line, "new") == 0) {
      if (db.entries)
        db.edited |= new(&db);
      else
        puts("db not initialized, did you open it ?");
    } else if (strcmp(line, "save") == 0) {
      if (db.entries)
        db.edited = save(&db, db.path);
      else
        puts("db not initialized, did you open it ?");
    } else if (strcmp(line, "close") == 0) {
      if (db.entries) {
        save(&db, db.path);
        free(db.entries - 16);
        memset((char *)&db, 0, sizeof(struct db));
        fd = 0;
        free(db.path);
        db.path = 0;
      } else {
        puts("db not initialized, did you open it ?");
      }
    } else if (strcmp(line, "exit") == 0) {
      save(&db, db.path);
      break;
    } else {
      puts("unkown command");
    }

    free(line);
  }
  puts("bye");
  return 0;
}
