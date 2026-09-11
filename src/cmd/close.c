#include "pwnman.h"
#include <string.h>

void closedb(struct db *db, char *_) {
  (void)_;
  if (!db->entries) {
    puts(RED"db not initialized"RESET);
    return;
  }

  savedb(db, "");
  memset(db->entries, 0, db->count*sizeof(struct entry));
  memset(db->path, 0, strlen(db->path));
  free(db->entries - 16);
  free(db->path);
  memset((char *)db, 0, sizeof(struct db));
  free(master_passwd);
  master_passwd = 0;
}

