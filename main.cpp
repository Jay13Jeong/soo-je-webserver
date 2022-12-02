#include <signal.h>
#include "webserv.hpp"

// Colors
#define RED "\x1b[0;31m"
#define BLUE "\x1b[0;34m"
#define GREEN "\x1b[0;32m"
#define YELLOW "\x1b[0;33m"
#define MAGENTA "\x1b[0;35m"
#define RESET "\x1b[0m"

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
            return(-1);
        else
            std::cerr << GREEN << "[*] Config validation check pass!" << RESET << std::endl;
    }
    else if (ac == 2)
    {
        if (!webserv.parsing(av[1])) //웹서브 클래스에 인자의 파일을 넣음
            return(-1);
        else
            std::cerr << GREEN << "[*] Config validation check pass!" << RESET << std::endl;
    }
    else
    {
        std::cerr << RED << "[!] Error : ARGS OVER" << RESET << std::endl;
        return(-1);
    }

    webserv.start();
    return 0;
}