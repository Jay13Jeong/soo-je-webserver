NAME = webserv
W3FLAG = -Wall -Wextra -Werror -std=c++98

.main.o : main.cpp *.hpp
	c++ $(W3FLAG)  -c -o $@ $< -O2

$(NAME) : .main.o 
	c++ $(W3FLAG) -o $@ $< -O2

all : $(NAME)

clean :
	rm -rf .main.o
	rm -rf .main_cookie.o

fclean : clean
	rm -rf $(NAME)
	rm -rf cookie
	rm -rf test

re : fclean all

test : main.cpp *.hpp
	c++ $(W3FLAG) -o $@ $< -DTEST=1 -O2

.main_cookie.o : main.cpp *.hpp
	c++ $(W3FLAG)  -c -o $@ $< -O2 -DCOOKIE=1 -O2

cookie : .main_cookie.o
	c++ $(W3FLAG) -o $@ $< -O2 -DCOOKIE=1 -O2

.PHONY : all clean fclean re
