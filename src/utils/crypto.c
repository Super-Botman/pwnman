#include "pwnman.h"

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
  unsigned char *src;
  if (!encrypt) {
    dest = (unsigned char *)fields;
    src = (unsigned char *)&entry->fields;
  } else {
    dest = (unsigned char *)&entry->fields;
    src = (unsigned char *)fields;
  }

  if (password_len) {
    for (size_t i = 0; i < sizeof(struct fields); i++)
      dest[i] = src[i] ^ key[i % password_len];
  } else {
    memcpy(dest, src, sizeof(struct fields));
  }

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
