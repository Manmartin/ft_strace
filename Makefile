NAME = $(BUILD_DIR)/ft_strace

MKDIR = mkdir -p
RM = rm -rf

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu11

LDFLAGS =

BUILD_DIR := build
SRC_DIR := src
INC_DIR := includes

SRCS :=	src/args.c 		\
    	src/main.c 		\
     	src/syscall.c 	\
      	src/tracer.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

CFLAGS += -I./$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJS) $(LDLIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

debug:: CFLAGS += -g3 -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=bounds -fsanitize=null
debug:: fclean $(NAME)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)
fclean: clean
	$(RM) $(BUILD_DIR)

re: fclean all

.PHONY: all sanitize clean fclean re
