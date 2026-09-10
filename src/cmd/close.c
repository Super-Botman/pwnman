#include "pwnman.h"

void closedb(struct db *db, char *_) {
  if (!db->entries) {
    puts("db not initialized");
    return;
  }

  savedb(db, "");
  free(db->entries - 16);
  free(db->path);
  memset((char *)db, 0, sizeof(struct db));
}

