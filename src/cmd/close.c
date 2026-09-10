#include "pwnman.h"

void closedb(struct db *db, char *_) {
  (void)_;
  if (!db->entries) {
    puts(RED"db not initialized"RESET);
    return;
  }

  savedb(db, "");
  free(db->entries - 16);
  free(db->path);
  memset((char *)db, 0, sizeof(struct db));
  free(master_passwd);
  master_passwd = 0;
}

