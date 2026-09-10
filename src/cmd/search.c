#include "pwnman.h"

void search(struct db *db, char *to_find) {
  if (is_database_empty(db) || !has_database(db) || !has_arguments(to_find)) {
      return; 
    }

  bool already_found_one = false;
  struct entry *entry = (struct entry *)db->entries;
  for (size_t i = 0; i < db->count; i++, entry++) {
    char title[TITLE_MAX + 1];
    get_title(entry, &title[0]);
    if (strcmp(to_find, title) == 0) {
        if (!already_found_one) {
          puts("---- Header ----");
          already_found_one = true;
        }
        puts(title);
    }
  }

  if (!already_found_one)
    puts("found nothing.");
}

