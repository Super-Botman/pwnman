#include "include/libotman.h"
#include "pwnman.h"

char randchar(void) {
    unsigned char b;
    while (1) {
        getrandom((char *)&b, 1);
        if (b < 52) {
            return (b < 26) ? 'a' + b : 'A' + (b - 26);
        }
    }
}

void new(struct db *db, char *_) {
  (void)_;
  if (!has_database(db))
    return;

  if (!master_passwd && db->count > 0) {
    struct fields fields;
    if (crypt((struct entry *)db->entries, &fields, 0) < 0) {
      puts(RED"invalid password"RESET);
      return;
    }
  }

  db->count += 1;
  size_t new_size = db->count * sizeof(struct entry);
  char *file_content = db->entries - 16;
  db->entries = realloc(file_content, new_size + 16) + 16;

  struct entry *new_entry = (struct entry *)db->entries + db->count - 1;
  memset((char *)new_entry, 0, sizeof(struct entry));

  printf("Title (max 64): ");
  ssize_t len = readline(new_entry->title, sizeof new_entry->title);
  new_entry->tlen = len;

  struct fields *new_fields = &new_entry->fields;

  printf("Username (max 64): ");
  len = readline(new_fields->username, sizeof new_fields->username);
  getrandom(new_fields->username + len + 1, 64 - len - 1);
  new_fields->ulen = len;

  puts("You can press enter to generate a random password");
  printf("Password (max 64): ");
  len = readline(new_fields->password, sizeof new_fields->password);
  if (len == 0) {
    printf("Password: ");
    for(int i = 0; i < 64; i++){
      char chr = randchar();
      putc(chr);
      new_fields->password[i] = chr;
    }
    putc('\n');
    len = 64;
  }
  getrandom(new_fields->password + len + 1, 64 - len - 1);
  new_fields->plen = len;

  crypt(new_entry, new_fields, 1);
  db->edited |= 1;
}
