#include "webserv.hpp"

int main(int ac, char* av[])
{
    Webserv webserv();

    if (ac == 1)
    {
        webserv.parsing("default.conf");
    }
    else if (ac == 2)
    {
        webserv.parsing(av[1]); //웹서브 클래스에 인자의 파일을 넣음
    }
    else if (ac >= 3)
    {
        std::cout << "ERROR : ARGS OVER" << std::endl;
        return(-1);
    }
    webserv.start();
    return 0;
}