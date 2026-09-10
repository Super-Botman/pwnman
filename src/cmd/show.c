#include "pwnman.h"

void show(struct db *db, char *arg) {

  size_t idx;
  if (is_database_empty(db) || !has_arguments(arg) || !is_right_index(db, arg, &idx)) {
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
