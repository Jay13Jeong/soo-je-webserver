NAME = webserv
W3FLAG = -Wall -Wextra -Werror -std=c++98

.main.o : main.cpp *.hpp
	c++ $(W3FLAG)  -c -o $@ $< -O2

$(NAME) : .main.o 
	c++ $(W3FLAG) -o $@ $< -O2

all : $(NAME)

clean :
	rm .main.o

fclean : clean
	rm $(NAME)

re : fclean all

test : main.cpp *.hpp
	c++ $(W3FLAG) -o $@ $< -DTEST=1 -O2

.PHONY : all clean fclean re
