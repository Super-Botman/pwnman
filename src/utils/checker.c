#include "pwnman.h"

bool is_database_empty(struct db *db) {
  if (db->count == 0) {
    puts("no entry in db");
    return TRUE;
  }
  return FALSE;
}

bool has_database(struct db *db) {
    if (!db->entries) {
      puts("db not initialized");
      return FALSE;
    }
    return TRUE;
}

bool has_arguments(char *arg) {
    if (!arg || strlen(arg) == 0) {
        printf("no arguments given\n");
        return FALSE;
    }
    return TRUE;
}

bool is_right_index(struct db *db, char *arg, size_t *idx) {
  *idx = atoi(arg);
  if (*idx >= db->count) {
    puts("invalid index");
    return FALSE;
  }
  return TRUE;
}
