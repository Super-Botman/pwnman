#include "pwnman.h"

void delete(struct db *db, char *arg) {
  size_t idx;
  if (is_database_empty(db) || !has_arguments(arg) || !is_right_index(db, arg, &idx)) {
    return;
  }

  size_t offset = idx * sizeof(struct entry);
  size_t size = db->count * sizeof(struct entry);

  struct entry *entry = (struct entry *)(db->entries + offset);
  struct entry *next = entry + 1;

  size_t diff = size - (offset + sizeof(struct entry));
  memcpy((char *)entry, (char *)next, diff);

  db->count -= 1;
  size_t new_size = db->count * sizeof(struct entry);
  char *file_content = db->entries - 16;
  db->entries = realloc(file_content, new_size - 16) + 16;
  db->edited |= 1;
}
