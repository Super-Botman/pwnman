#include "pwnman.h"

void new(struct db *db, char *_) {

  (void)_;
  if (!has_database(db))
    return;

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

