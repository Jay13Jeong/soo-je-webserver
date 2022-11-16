#include <signal.h>
//#include "client.hpp"
//#include "location.hpp"
// #include ".hpp"
#include "webserv.hpp"



std::vector<struct kevent> g_detects; //감지 할 이벤트벡터.
// std::map<int, IO_manager> g_io_infos; //키:fd 값:fd에대한 정보구조체.

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
    // signal(SIGINT, &my_sig);
    // webserv.start();
    return 0;
}