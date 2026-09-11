#include "pwnman.h"

void edit(struct db *db, char *arg) {
  size_t idx;
  if (!has_database(db) || is_database_empty(db) || !has_arguments(arg) ||
      !is_right_index(db, arg, &idx)) {
    return;
  }

  struct entry *entry = (struct entry *)db->entries + idx;

  struct fields new_fields;
  if (crypt(entry, &new_fields, 0) < 0) {
    puts(RED "invalid password" RESET);
    return;
  }

  char title[65];
  int title_len = entry->tlen;
  get_title(entry, &title[0]);

  int edited = 0;
  while (1) {
    if (!edited)
      printf(BLUE "[%s ~ %s]> " RESET, db->path, title);
    else
      printf(BLUE "[%s ~ " BOLD "*%s" RESET BLUE "]> " RESET, db->path, title);

    char *line;
    size_t n = 0;
    size_t len = getline(&line, &n);

    if (strcmp(line, "help") == 0) {
      puts("Edit mode");
      puts("title <title>: change title");
      puts("username <username>: change username");
      puts("password <password>: change password (don't set password to "
           "generate a random one)");
      puts("save: save the updated entry into memory");
      puts("exit: exit edit mode");

    } else if (strncmp(line, "title ", 6) == 0) {
      len -= 6;
      line += 6;
      while (*line == ' ') {
        line++;
        len--;
      }
      memset(&title[0], 0, sizeof(title));
      memcpy(&title[0], line, len);
      title_len = len;
      puts(BOLD GREEN "success" RESET);
      edited = 1;
    } else if (strncmp(line, "username ", 9) == 0) {
      len -= 9;
      line += 9;
      while (*line == ' ') {
        line++;
        len--;
      }

      memcpy(new_fields.username, line, len);
      new_fields.ulen = len;
      getrandom(new_fields.username + len, 64 - len);
      puts(BOLD GREEN "success" RESET);
      edited = 1;
    } else if (strncmp(line, "password", 8) == 0) {
      len -= 8;
      line += 8;
      while (*line == ' ') {
        line++;
        len--;
      }

      if (len == 0) {
        printf("Password: ");
        for (int i = 0; i < 64; i++) {
          char chr = randchar();
          putc(chr);
          new_fields.password[i] = chr;
        }
        putc('\n');
        len = 64;
      } else {
        memcpy(new_fields.password, line, len);
      }

      new_fields.plen = len;
      getrandom(new_fields.password + len, 64 - len);
      puts(BOLD GREEN "success" RESET);
      edited = 1;
    } else if (strcmp(line, "save") == 0) {
      memcpy(entry->title, &title[0], sizeof(entry->title));
      entry->tlen = title_len;
      crypt(entry, &new_fields, 1);
      db->edited |= 1;
      break;
    } else if (strcmp(line, "exit") == 0) {
      free(line);
      return;
    } else {
      puts(RED "unknown command" RESET);
    }
  }
}
