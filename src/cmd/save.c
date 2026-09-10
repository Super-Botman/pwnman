#include "pwnman.h"

void savedb(struct db *db, char *_) {
  if (!db->entries) {
    puts("db not initialized");
    return;
  }

  int tmp = open(".tmp.db", O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (tmp < 0) {
    puts("cannot save");
    return;
  }

  int len = db->count * (sizeof(struct entry)) + 16;
  char *file_content = (char *)(db->entries - 16);
  if (write(tmp, file_content, len) != len) {
    puts("failed to write");
    return;
  }
  if (rename(".tmp.db", db->path) < 0) {
    puts("failed to save");
    return;
  }
  db->edited = 0;
}
