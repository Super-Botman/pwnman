.PHONY: all build link clean fclean

all: link

build: src/entry.asm ./src/main.c
	nasm -f elf64 ./src/entry.asm -o ./build/entry.o
	gcc -I./include -Wno-builtin-declaration-mismatch -c -fno-stack-protector ./src/main.c -o ./build/main.o

link: build
	ld -e _start -o ./build/main ./build/entry.o ./build/main.o

clean:
	rm -rf ./build/*.o

fclean:
	rm -rf ./build/*
