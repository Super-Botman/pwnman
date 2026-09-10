#include "pwnman.h"

void edit(struct db *db, char *arg) {
  while (*arg == ' ')
    arg++;
  int idx = atoi(arg);

  if (idx >= db->count) {
    puts("invalid index");
    return;
  }

  struct entry *entry =
      (struct entry *)(db->entries + (sizeof(struct entry) * idx));

  struct fields new_fields;
  if (crypt(entry, &new_fields, 0) < 0) {
    puts("invalid password");
    return;
  }

  char title[65];
  int title_len = entry->tlen;
  get_title(entry, &title[0]);

  while (1) {
    printf("[%s ~ %s]> ", db->path, title);

    char *line = getline();
    size_t len = strlen(line);

    if (strcmp(line, "help") == 0) {
      puts("Edit mode");
      puts("title <title>: change title");
      puts("username <username>: change username");
      puts("password <password>: change password");
      puts("save: save the updated entry into memory");
      puts("exit: exit without saving");
    } else if (strncmp(line, "title ", 6) == 0) {
      memset(&title[0], 0, sizeof(title));
      memcpy(&title[0], line + 6, len - 6);
      title_len = len - 6;
      puts("success");
    } else if (strncmp(line, "username ", 9) == 0) {
      len -= 9;
      memcpy(new_fields.username, line + 9, len);
      new_fields.ulen = len;
      getrandom(new_fields.username + len, 64 - len);
      puts("success");
    } else if (strncmp(line, "password ", 9) == 0) {
      len -= 9;
      memcpy(new_fields.password, line + 9, len);
      new_fields.plen = len;
      getrandom(new_fields.password + len, 64 - len);
      puts("success");
    } else if (strcmp(line, "save") == 0) {
      memcpy(entry->title, &title[0], title_len);
      entry->tlen = title_len;
      crypt(entry, &new_fields, 1);
      free(line);
      db->edited |= 1;
      break;
    } else if (strcmp(line, "exit") == 0) {
      free(line);
      return;
    } else {
      puts("unknown command");
    }

    free(line);
  }
}
