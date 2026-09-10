#include "pwnman.h"

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
