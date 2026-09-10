#include "pwnman.h"

void createdb(struct db *db, char *path) {
  if (db->entries) {
    puts("db already opened, close it before opening another");
    return;
  }

  while (*path == ' ')
    path++;

  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (fd < 0) {
    puts("failed to create the new db");
    return;
  }

  char header[16] = "B0T\0\0\0\0\0\0\0\0\0\0\0\0\0";
  if (write(fd, &header[0], 16) != 16) {
    puts("failed to write header");
    return;
  }
  close(fd);
  opendb(db, path);
}
