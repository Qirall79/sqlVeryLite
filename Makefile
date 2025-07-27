CFLAGS = -std=c++23 -fsanitize=address -g

SRC = $(wildcard src/*.cpp)
	
OBJ = $(patsubst src/%.cpp, obj/%.o, $(SRC))
NAME = sqlite
HEADERS = $(wildcard src/*.hpp)
CC = c++
RM = rm -f

all: $(NAME)

obj/%.o: src/%.cpp $(HEADERS) | obj
	$(CC) -c $(CFLAGS) $< -o $@ -g

obj:
	mkdir -p obj

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) -g

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re