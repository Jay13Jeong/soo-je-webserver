gitNAME = webserv
W3FLAG = -Wall -Wextra -Werror -std=c++98

.main.o : main.cpp *.hpp
	c++ $(W3FLAG)  -c -o $@ $< -O2

$(NAME) : .main.o 
	c++ $(W3FLAG) -o $@ $< -O2

all : $(NAME)

clean :
	rm -rf .main.o
	rm -rf .main_cookie.o
	rm -rf .main_parrot.o

fclean : clean
	rm -rf $(NAME)
	rm -rf cookie
	rm -rf parrot

re : fclean all

test : main.cpp *.hpp
	c++ $(W3FLAG) -o $@ $< -DTEST=1 -O2

.main_cookie.o : main.cpp *.hpp
	c++ $(W3FLAG)  -c -o $@ $< -O2 -DCOOKIE=1 -O2

cookie : .main_cookie.o
	c++ $(W3FLAG) -o $@ $< -O2 -DCOOKIE=1 -O2

.main_parrot.o : main.cpp *.hpp
	c++ $(W3FLAG)  -c -o $@ $< -DPARROT=1 -O2

parrot : .main_parrot.o
	c++ $(W3FLAG) -o $@ $< -DPARROT=1 -O2

.PHONY : all clean fclean re
