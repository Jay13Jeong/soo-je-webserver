// #include <signal.h>
#include "webserv.hpp"

int main(int ac, char* av[])
{
    Webserv webserv;

    if (ac == 1)
    {
        if (!webserv.parsing("error.conf"))
            std::cout << "config file error\n";
        else
            std::cout << "config file ok!\n";
    }
    else if (ac == 2)
    {
        if (!webserv.parsing(av[1])) //웹서브 클래스에 인자의 파일을 넣음
            std::cout << "config file error\n";
        else
            std::cout << "config file ok!\n";
    }
    else if (ac >= 3)
    {
        std::cout << "ERROR : ARGS OVER" << std::endl;
        return(-1);
    }
    // webserv.start();
    return 0;
}