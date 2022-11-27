#include <signal.h>
#include "webserv.hpp"

void ignore_sigpipe(int sig)
{
	(void)sig;
}

int main(int ac, char* av[])
{
    signal(SIGPIPE, ignore_sigpipe);
    Webserv webserv;

    if (ac == 1)
    {
        if (!webserv.parsing("default.conf")) //웹서브 클래스에 인자의 파일을 넣음
        {
            std::cerr << "config file error\n";
            return(-1);
        }
    }
    else if (ac == 2)
    {
        if (!webserv.parsing(av[1])) //웹서브 클래스에 인자의 파일을 넣음
        {
            std::cerr << "config file error\n";
            return(-1);
        }
        else
            std::cerr << "config file ok!\n";
    }
    else if (ac >= 3)
    {
        std::cerr << "ERROR : ARGS OVER" << std::endl;
        return(-1);
    }

    webserv.start();
    return 0;
}