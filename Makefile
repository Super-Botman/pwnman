.PHONY: all build link clean fclean

CFLAGS = -I./lib -Wno-builtin-declaration-mismatch -c -fno-stack-protector -nostdlib -MMD -MP

LIB_SRCS = $(wildcard ./lib/*.c)
LIB_OBJS = $(patsubst ./lib/%.c,./build/%.o,$(LIB_SRCS))

all: link

builddir:
	mkdir -p ./build

build: builddir ./src/entry.asm ./src/main.c ./build/libotman.a
	nasm -f elf64 ./src/entry.asm -o ./build/entry.o
	gcc $(CFLAGS) ./src/main.c -o ./build/main.o

./build/libotman.a: $(LIB_OBJS)
	ar rcs $@ $^

./build/%.o: ./lib/%.c | builddir
	gcc $(CFLAGS) $< -o $@

link: build
	ld -e _start -o ./build/main ./build/entry.o ./build/main.o -L./build -lotman

clean: 
	rm -rf ./build/*.o

fclean:
	rm -rf ./build/*
