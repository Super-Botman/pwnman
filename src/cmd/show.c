#include "pwnman.h"

void show(struct db *db, char *arg) {
  if (db->count == 0) {
    puts("no entry in db");
    return;
  }

  while (*arg == ' ')
    arg++;
  size_t idx = atoi(arg);

  if (idx >= db->count) {
    puts("invalid index");
    return;
  }

  struct entry *entry =
      (struct entry *)(db->entries + (sizeof(struct entry) * idx));

  struct fields fields;
  memset((char*)&fields, 0, sizeof(struct fields));
  if (crypt(entry, &fields, 0) < 0) {
    puts("invalid entry or password");
    return;
  }

  char title[65], username[65], password[65];
  get_title(entry, title);
  get_user(&fields, username);
  get_pass(&fields, password);

  print_table(3, "title", "username", "password", title, username, password);
}
