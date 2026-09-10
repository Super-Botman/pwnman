#ifndef PWNMAN_H
#define PWNMAN_H

#include "libotman.h"
#include "ioctls.h"
#include "termbits.h"

typedef unsigned int u32;
typedef unsigned short u16;

// structs
struct fields {
  u32 crc32;
  u16 ulen;
  u16 plen;
  char username[64];
  char password[64];
};

struct entry {
  u32 tlen;
  u32 padding;
  char title[64];
  struct fields fields;
};

struct db {
  int edited;
  char *path;
  size_t count;
  char *entries;
};

// globals
extern char *master_passwd;

// cmd
void opendb(struct db *db, char *path);
void createdb(struct db *db, char *path);
void closedb(struct db *db, char *path);
void savedb(struct db *db, char *_);
void list(struct db *db, char *_);
void show(struct db *db, char *arg);
void delete(struct db *db, char *arg);
void edit(struct db *db, char *arg);
void new(struct db *db, char *_);

// ui
void print_separator(int len);
void print_pad_num(int num, int width);
void print_table(int col, ...);
void echo_off();
void echo_on();

// utils
int crypt(struct entry *entry, struct fields *fields, int encrypt);
void get_title(struct entry *entry, char *title);
void get_user(struct fields *fields, char *user);
void get_pass(struct fields *fields, char *pass);

#endif
