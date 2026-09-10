NAME = libotman.a

CC = gcc
CPPFLAGS = -Iinclude -MMD -MP
CFLAGS = -Wall -Wextra -Werror -ffreestanding -fno-builtin

AR = ar
ARFLAGS = rcs

ifdef DEBUG
	CFLAGS += -g
endif

SRC_DIR = src
SRC = crypto.c \
      io.c \
      malloc.c \
      string.c \
      utils.c

SRC := $(addprefix $(SRC_DIR)/, $(SRC))

OBJ_DIR = build
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

DEP = $(OBJ:.o=.d)

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ) $(OBJ_ASM)
	$(AR) $(ARFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OBJ_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: clean all

-include $(DEP)

