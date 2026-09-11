#ifndef PWNMAN_H
#define PWNMAN_H

#include "include/libotman.h"
#include "ioctls.h"
#include "termbits.h"

#define RESET        "\033[0m"
#define BOLD         "\033[1m"
#define RED          "\033[31m"
#define GREEN        "\033[32m"
#define YELLOW       "\033[33m"
#define BLUE         "\033[34m"
#define MAGENTA      "\033[35m"
#define CYAN         "\033[36m"
#define WHITE        "\033[37m"
#define DEFAULT      "\033[39m"

#define TRUE 1
#define FALSE 0

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
void add(struct db *db, char *_);
void search(struct db *db, char *arg);

// ui
#define TITLE_MAX 64
#define ID_WIDTH 4

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

// checker
bool has_database(struct db *db);
bool has_arguments(char *arg);
bool is_right_index(struct db *db, char *arg, size_t *idx);
bool is_database_empty(struct db *db);

#endif
