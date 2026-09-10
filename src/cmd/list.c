#include "pwnman.h"

void list(struct db *db, char *_) {
  (void)_;
  if (is_database_empty(db))
    return;

  const int width = ID_WIDTH + TITLE_MAX + 4;
  print_separator(width);
  puts("   id | title");
  print_separator(width);

  struct entry *entry = (struct entry *)db->entries;
  for (size_t i = 0; i < db->count; i++, entry++) {
    char title[TITLE_MAX + 1];

    get_title(entry, title);

    putc(' ');
    print_pad_num(i, ID_WIDTH);
    printf(" | %s", title);
    putc('\n');

    print_separator(width);
  }
}
