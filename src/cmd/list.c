#include "pwnman.h"

#define TITLE_MAX 64
#define ID_WIDTH 4

void list(struct db *db, char *_) {
  (void)_;
  if (db->count == 0) {
    puts("no entry in db");
    return;
  }

  const int width = ID_WIDTH + TITLE_MAX + 4;
  struct entry *entry = (struct entry *)db->entries;

  print_separator(width);
  puts("   id | title");
  print_separator(width);

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
