#include "pwnman.h"

bool is_database_empty(struct db *db) {
  if (db->count == 0) {
    puts(RED"no entry in db"RESET);
    return TRUE;
  }
  return FALSE;
}

bool has_database(struct db *db) {
    if (!db->entries) {
      puts(RED"db not initialized"RESET);
      return FALSE;
    }
    return TRUE;
}

bool has_arguments(char *arg) {
    if (!arg || strlen(arg) == 0) {
        printf(RED"no arguments given\n"RESET);
        return FALSE;
    }
    return TRUE;
}

bool is_right_index(struct db *db, char *arg, size_t *idx) {
  *idx = atoi(arg);
  if (*idx >= db->count) {
    puts(RED"invalid index"RESET);
    return FALSE;
  }
  return TRUE;
}
