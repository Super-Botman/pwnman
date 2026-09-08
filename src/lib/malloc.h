#include "crypto.h"
#include "io.h"
#include "types.h"

struct chunk {
  long sig;
  int prev;
  int size;
};

struct meta {
  void *fd;
  void *bk;
};

void *base = 0;
void *top = 0;

struct meta *freed = 0;
long key = 0xdeadbeef;

void *brk(void *brk) {
  void *ret;
  __asm__ volatile("mov $12, %%rax\n"
                   "mov %1, %%rdi\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "g"(brk)
                   : "rdi", "memory");

  return ret;
}

#define FREE 0x1

#define ptr2chk(ptr) (struct chunk *)((long)ptr - sizeof(struct chunk))
#define chk2ptr(ptr) (void *)((long)ptr + sizeof(struct chunk))
#define chk2meta(ptr) (struct meta *)((long)ptr + sizeof(struct chunk))
#define getsize(chk) (chk->size & ~0xf)
#define align(size) ((size & ~0xf) + 0x10)
#define isfree(chk) (chk->size & FREE)
#define inheap(chk) ((void *)chk >= base && (void *)chk <= top)
#define checksig(chk) (chk->sig == sig(chk))

long sig(struct chunk *chk) {
  unsigned long cookie = chk->size + chk->prev + key;
  if (isfree(chk)) {
    struct meta *meta = chk2meta(chk);
    cookie += (long)meta->fd + (long)meta->bk;
  };
  return crc32((char *)&cookie, sizeof(unsigned long));
}

void *malloc(size_t size) {
  void *ret = (void *)-1;
  if (!base) {
    top = base = brk(0);
    getrandom((char *)&key, 8);
  }

  size = align(size);

  if (!freed) {
    struct chunk *chk = (struct chunk *)top;
    top = brk(chk2ptr((char *)top + size));

    chk->size = size;
    chk->sig = sig(chk);
    ret = chk2ptr(chk);
    goto ret;
  }

  struct meta *curr = freed;

  while (curr && inheap(curr)) {
    struct chunk *chk = ptr2chk(curr);
    size_t chk_size = getsize(chk);

    if (!checksig(chk)) {
      puts("invalid malloc operation (invalid sig)\n");
      exit(-1);
    }

    if (chk_size < size) {
      curr = curr->fd;
      continue;
    }

    struct meta *next_meta = curr->fd;
    struct meta *prev_meta = curr->bk;
    void *updated_bk = prev_meta;
    void *updated_fd = next_meta;

    if (chk_size > size) {
      struct chunk *new_chk = (struct chunk *)((char *)chk + size);
      struct meta *new_meta = chk2meta(new_chk);

      new_chk->prev = size;
      new_chk->size = (chk_size - size) | FREE;

      new_meta->fd = next_meta;
      new_meta->bk = prev_meta;

      updated_fd = updated_bk = new_meta;
      new_chk->sig = sig(new_chk);
    }

    if (next_meta) {
      next_meta->bk = updated_bk;
      struct chunk *next_chk = ptr2chk(next_meta);
      next_chk->sig = sig(next_chk);
    }

    if (prev_meta) {
      prev_meta->fd = updated_fd;
      struct chunk *prev_chk = ptr2chk(prev_meta);
      prev_chk->sig = sig(prev_chk);
    } else
      freed = (struct meta*)updated_fd;

    curr->fd = 0;
    curr->bk = 0;
    chk->size = size;
    chk->sig = sig(chk);

    ret = chk2ptr(chk);
    goto ret;
  }

  struct chunk *chk = (struct chunk *)top;
  top = brk(chk2ptr((char *)top + size));

  chk->size = size;
  chk->sig = sig(chk);
  ret = chk2ptr(chk);
ret:
  printf("[MALLOC] ret: 0x%p, size: 0x%p\n", ret, size);
  return ret;
}

void free(void *ptr) {
  if (!ptr)
    return;

  if (!inheap(ptr)) {
    puts("invalid free operation (out of bounds)\n");
    exit(-1);
  }

  struct chunk *chk = ptr2chk(ptr);
  printf("[FREE] ptr: 0x%p, size: 0x%p\n", ptr, getsize(chk));

  if (isfree(chk)) {
    puts("invalid free operation (double free)\n");
    exit(-1);
  }

  if (!checksig(chk)) {
    puts("invalid free operation (invalid sig)\n");
    exit(-1);
  }

  size_t chk_size = getsize(chk);
  chk->size = chk_size | FREE;
  struct meta *meta = (struct meta *)ptr;

  struct chunk *next_chk = (struct chunk *)((char *)ptr + chk_size);
  if (inheap(next_chk) && isfree(next_chk) && checksig(next_chk)) {
    size_t next_size = getsize(next_chk);
    chk_size += next_size;
    chk->size = chk_size | FREE;

    struct meta *next_meta = chk2meta(next_chk);
    struct meta *n_fd = next_meta->fd;
    struct meta *n_bk = next_meta->bk;

    if (n_fd) {
      n_fd->bk = n_bk;
      struct chunk *c = ptr2chk(n_fd);
      c->sig = sig(c);
    }
    if (n_bk) {
      n_bk->fd = n_fd;
      struct chunk *c = ptr2chk(n_bk);
      c->sig = sig(c);
    } else {
      freed = n_fd;
    }

    next_chk->size = 0;
    next_chk->prev = 0;
    next_chk->sig = 0;
    next_meta->fd = 0;
    next_meta->bk = 0;
  }

  struct chunk *prev_chk = (struct chunk *)((char *)chk - chk->prev - sizeof(struct chunk));
  if (prev_chk && inheap(prev_chk) && isfree(prev_chk) && checksig(prev_chk)) {
    size_t prev_size = getsize(prev_chk);
    prev_size += chk_size;
    prev_chk->size = prev_size | FREE;

    chk->size = 0;
    chk->prev = 0;
    chk->sig = 0;
    meta->fd = 0;
    meta->bk = 0;

    chk = prev_chk;
    chk_size = prev_size;
  } else {
    meta->fd = freed;
    meta->bk = 0;

    if (freed) {
      freed->bk = meta;
      struct chunk *f_chk = ptr2chk(freed);
      f_chk->sig = sig(f_chk);
    }
    freed = meta;
  }

  struct chunk *following_chk = (struct chunk *)((char *)ptr + chk_size);
  if (inheap(following_chk)) {
    following_chk->prev = chk_size;
    following_chk->sig = sig(following_chk);
  }

  chk->sig = sig(chk);
}

void *realloc(void *ptr, size_t size) {
  void* ret = ptr;
  size = align(size);

  struct chunk *chk = ptr2chk(ptr);
  size_t chk_size = getsize(chk);

  if (size == chk_size) goto ret;

  if (size > chk_size) {
    if (!inheap(ptr+size)) {
      brk(top+(size-chk_size));
      chk->size = size;
      chk->sig = sig(chk);
      goto ret;
    }

    struct chunk *following_chk = (struct chunk *)((char *)chk + chk_size);
    int more = size-chk_size;
    
    if (isfree(following_chk) && inheap(following_chk) && following_chk->size > more) {
      chk->size = size;
      chk->sig = sig(chk);

      struct chunk *new_chk = (struct chunk*)((char*)ptr + size);
      struct meta *new_meta = chk2meta(new_chk);
      struct meta *following_meta = chk2meta(following_chk);
      int new_size = following_chk->size-more | FREE;
      struct meta *new_fd = following_meta->fd;
      struct meta *new_bk = following_meta->bk;

      following_chk->prev = 0;
      following_chk->size = 0;
      following_chk->sig = 0;
      following_meta->fd = 0;
      following_meta->bk = 0;

      new_chk->size = new_size;
      new_chk->prev = size;
      new_meta->fd = new_fd; 
      new_meta->bk = new_bk; 
      new_chk->sig = sig(new_chk);

      if (new_fd) {
        new_fd->bk = new_meta;
        struct chunk *c = ptr2chk(new_fd);
        c->sig = sig(c);
      }
      if (new_bk) {
        new_bk->fd = new_meta;
        struct chunk *c = ptr2chk(new_bk);
        c->sig = sig(c);
      } else {
        freed = new_meta;
      }
      
      goto ret;
    }

    ret = malloc(size);
    memcpy(ret, ptr, chk_size);
    free(ptr);
    goto ret;
  } else {
    int less = chk_size - size;
    chk->size = size;
    chk->sig = sig(chk);

    struct chunk *new_chk = (struct chunk *)((char *)ptr + size);
    struct meta *new_meta = chk2meta(new_chk);
    new_chk->size = less | FREE;
    new_chk->prev = size;
    new_meta->bk = freed;
    new_meta->fd = 0;

    if (freed) {
      freed->fd = new_chk;
      struct chunk *c = ptr2chk(freed);
      c->sig = sig(c);
    }

    freed = chk2meta(new_chk);
    new_chk->sig = sig(new_chk);
  }

ret:
  printf("[REALLOC] ret: 0x%p, size: 0x%p\n", ret, size);
  return ret;
}
