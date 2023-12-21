CC = gcc

NAME = esgi_fight

SRC = main.c

OBJ = $(SRC:.c=.o)

CPPFLAGS = -I./includes -Wall -Wextra
LDFLAGS = -L./includes -lSDL -lSDL_gfx -lm
CFLAGS = -g3

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(CPPFLAGS) $(CFLAGS) -lSDL_main -lSDL -lSDL_gfx -lm

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
	clear

re: fclean all

.PHONY: all clean fclean re
