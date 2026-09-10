#include "pwnman.h"

void search(struct db *db, char *to_find) {
  if (is_database_empty(db) || !has_database(db) || !has_arguments(to_find)) {
      return; 
    }

  struct entry *entry = (struct entry *)db->entries;
  for (size_t i = 0; i < db->count; i++, entry++) {
    char title[TITLE_MAX + 1];
    get_title(entry, title);
    if ()
  }
}

