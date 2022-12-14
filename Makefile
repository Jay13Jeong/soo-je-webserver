NAME = webserv
W3FLAG = -Wall -Wextra -Werror -std=c++98

all : $(NAME)

.obj/main.o : srcs/main.cpp srcs/*.hpp
	mkdir -p .obj
	c++ $(W3FLAG)  -c -o $@ $< -O2

$(NAME) : .obj/main.o
	c++ $(W3FLAG) -o $@ $< -O2

clean :
	rm -rf .obj
	rm -rf .payload

fclean : clean
	rm -rf $(NAME)
	rm -rf cookie
	rm -rf test
	rm -rf YoupiBanane

re : fclean all

test : srcs/main.cpp srcs/*.hpp YoupiBanane
	c++ $(W3FLAG) -o $@ $< -DTEST=1 -O2

.obj/main_cookie.o : srcs/main.cpp srcs/*.hpp
	mkdir -p .obj
	c++ $(W3FLAG)  -c -o $@ $< -O2 -DCOOKIE=1 -O2

cookie : .obj/main_cookie.o
	c++ $(W3FLAG) -o $@ $< -O2 -DCOOKIE=1 -O2

YoupiBanane :
	mkdir -p YoupiBanane/nop
	touch YoupiBanane/nop/other.pouic
	touch YoupiBanane/nop/youpi.bad_extension
	mkdir -p YoupiBanane/Yeah
	touch YoupiBanane/Yeah/not_happy.bad_extension
	touch YoupiBanane/youpi.bad_extension
	touch YoupiBanane/youpi.bla
	touch YoupiBanane/youpla.bla

.PHONY : all clean fclean re
