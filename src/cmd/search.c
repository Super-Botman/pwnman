#include "pwnman.h"

size_t levenshtein(char *s1, char *s2) {
  size_t m = strlen(s1);
  size_t n = strlen(s2);

  int **d = malloc((m + 1) * sizeof(int *));
  for (size_t i = 0; i <= m; i++) {
    d[i] = malloc((n + 1) * sizeof(int));
  }

  for (size_t i = 0; i <= m; i++)
    d[i][0] = i;
  for (size_t j = 0; j <= n; j++)
    d[0][j] = j;

  for (size_t i = 1; i <= m; i++) {
    for (size_t j = 1; j <= n; j++) {
      int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

      int del = d[i - 1][j] + 1;
      int ins = d[i][j - 1] + 1;
      int sub = d[i - 1][j - 1] + cost;

      int min = del < ins ? del : ins;
      d[i][j] = min < sub ? min : sub;
    }
  }

  size_t ret = d[m][n];

  for (size_t i = 0; i <= m; i++) {
    free(d[i]);
  }
  free(d);

  return ret;
}

struct distance {
  size_t idx;
  int value;
};

void sort(struct distance *a, int n) {
  for (int i = 1; i < n; i++) {
    struct distance temp = a[i];
    int j = i - 1;

    while (j >= 0 && a[j].value > temp.value) {
      a[j + 1] = a[j];
      j--;
    }

    a[j + 1] = temp;
  }
}

void search(struct db *db, char *to_find) {
  if (is_database_empty(db) || !has_database(db) || !has_arguments(to_find)) {
    return;
  }

  struct entry *entries = (struct entry *)db->entries;
  struct distance *distances = malloc(db->count * sizeof(struct distance));

  for (size_t i = 0; i < db->count; i++) {
    char title[TITLE_MAX + 1];
    get_title(&entries[i], title);
    distances[i].idx = i;
    distances[i].value = levenshtein(title, to_find);
  }
  sort(distances, db->count);

  const int width = ID_WIDTH + TITLE_MAX + 4;
  print_separator(width);
  puts("   id | title");
  print_separator(width);
  size_t limit = db->count < 4 ? db->count : 4;
  for (size_t i = 0; i < limit; i++) {
    struct entry *entry = &entries[distances[i].idx];
    char title[TITLE_MAX + 1];
    get_title(entry, title);
    
    putc(' ');
    print_pad_num(distances[i].idx, ID_WIDTH);
    printf(" | %s", title);
    putc('\n');
    print_separator(width);

  }

  free(distances);
}
