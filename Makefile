NAME = pwman
LIB = libotman/libotman.a

CC = gcc
CPPFLAGS = -Iinclude -Ilibotman -MMD -MP
CFLAGS = -Wextra -ffreestanding -fno-builtin \
-fno-stack-protector -fno-pie

AS = nasm
ASFLAGS = -f elf64

ifdef DEBUG
CFLAGS += -g
AFLAGS += -g -F dwarf
endif

LD = ld
LDFLAGS = -e _start

SRC_DIR = src
SRC = main.c \
    cmd/close.c \
    cmd/create.c \
    cmd/delete.c \
    cmd/edit.c \
    cmd/list.c \
    cmd/new.c \
    cmd/open.c \
    cmd/save.c \
    cmd/show.c \
    ui/draw.c \
    utils/getter.c \
    utils/crypto.c

SRC_ASM = entry.asm

SRC := $(addprefix $(SRC_DIR)/, $(SRC))
SRC_ASM := $(addprefix $(SRC_DIR)/, $(SRC_ASM))

OBJ_DIR = build
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
OBJ_ASM = $(patsubst %.asm, $(OBJ_DIR)/%.o, $(SRC_ASM))

DEP = $(OBJ:.o=.d)

.PHONY: all
all: $(NAME)

$(NAME): $(LIB) $(OBJ) $(OBJ_ASM)
	$(LD) $(LDFLAGS) -o $@ $^

$(LIB):
	$(MAKE) -C libotman
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.asm
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OBJ_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: clean all

-include $(DEP)
