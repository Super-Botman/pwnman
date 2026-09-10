#include "pwnman.h"

void opendb(struct db *db, char *path) {
  if (db->entries) {
    puts(RED"db already opened, close it before opening another"RESET);
    return;
  }

  while (*path == ' ')
    path++;

  int fd = open(path, O_RDWR, 0);
  if (fd < 0) {
    puts(RED"error while opening the db"RESET);
    return;
  }

  ssize_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *file_content = malloc(file_size);
  if (read(fd, file_content, file_size) != file_size) {
    puts(RED"error while reading the db"RESET);
    free(file_content);
    return;
  }

  if (strncmp(file_content, "B0T", 4) != 0) {
    puts(RED"invalid db"RESET);
    free(file_content);
    return;
  }

  file_content += 16;
  size_t db_size = file_size - 16;
  size_t db_count = db_size / sizeof(struct entry);
  db->entries = file_content;
  db->count = db_count;
  db->path = strdup(path);
}
