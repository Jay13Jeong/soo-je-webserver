#include <signal.h>
//#include "Client.hpp"
//#include "location.hpp"
// #include ".hpp"
#include "Webserv.hpp"


std::vector<struct kevent> g_detects; //감지 할 이벤트벡터.

//인터럽트가 발생하면 fd를 모두 닫고 종료.
void my_sig(int signal)
{
    if (signal == SIGINT)
    {
        while (!g_detects.empty())
        {
            close(g_detects.back().ident);
            g_detects.pop_back();
        }
    }
    exit(0);
}

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
    // signal(SIGINT, &my_sig);
    // webserv.start();
    return 0;
}