PROGRAM_NAME = ft_strace
NAME = $(BUILD_DIR)/$(PROGRAM_NAME)

MKDIR = mkdir -p
RM = rm -rf

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=gnu11

LDFLAGS =

BUILD_DIR := build
SRC_DIR := src
INC_DIR := includes

SRCS :=	src/args.c 		\
    	src/main.c 		\
     	src/signals.c 	\
     	src/syscalls.c 	\
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

# Docker commands
docker:
	@docker build -t $(PROGRAM_NAME) -f utils/Dockerfile .
	@docker run --name $(PROGRAM_NAME) -it -v './:/usr/strace' $(PROGRAM_NAME) /bin/bash
	@docker stop $(PROGRAM_NAME)
	@docker rm $(PROGRAM_NAME)

.PHONY: all sanitize clean fclean re docker
