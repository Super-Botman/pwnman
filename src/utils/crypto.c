#include "pwnman.h"

int crypt(struct entry *entry, struct fields *fields, int encrypt) {
  char *key = master_passwd;
  size_t password_len = 0;
  if (!key) {
    printf("Master password: ");
    echo_off();
    size_t n;
    password_len = getline(&key, &n);
    echo_on();
    puts("");
  } else {
    password_len = strlen(key);
  }

  if (encrypt)
    fields->crc32 = crc32((unsigned char *)&fields->ulen, sizeof(struct fields) - 4);

  char *dest;
  char *src;
  if (!encrypt) {
    dest = (char *)fields;
    src = (char *)&entry->fields;
  } else {
    dest = (char *)&entry->fields;
    src = (char *)fields;
  }

  if (password_len) {
    for (size_t i = 0; i < sizeof(struct fields); i++)
      dest[i] = src[i] ^ key[i % password_len];
  } else {
    memcpy(dest, src, sizeof(struct fields));
  }

  if (!encrypt) {
    u32 sig = crc32((unsigned char *)&fields->ulen, sizeof(struct fields) - 4);
    if (fields->crc32 != sig)
      return -1;
  }

  if (!master_passwd) {
    printf("Keep password in memory? [Y/n]: ");
    int c = getchar();
    while (c != '\n' && getchar() != '\n') { }
    if (c == 'n') {
      memset(key, 0, password_len);
      free(key);
    } else {
      master_passwd = key;
    }
  }
  return 0;
}
