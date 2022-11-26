NAME = webserv
# W3FLAG = -Wall -Wextra -Werror -std=c++98

.main.o : main.cpp
	c++ $(W3FLAG)  -c -o $@ $<

$(NAME) : .main.o *.hpp
	c++ $(W3FLAG) -o $@ $<

all : $(NAME)

clean :
	rm .main.o

fclean : clean
	rm $(NAME)

re : fclean all

.PHONY : all clean fclean re