//cgi를 실행하는 메소드.
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <dirent.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>

char** init_cgi_env()
{
    char **env = (char **)malloc(sizeof(char *) * 4);
    env[0] = strdup("REQUEST_METHOD=GET");
    env[1] = strdup("SERVER_PROTOCOL=HTTP/1.1");
    env[2] = strdup("PATH_INFO=anyway@@@");
    env[3] = NULL;
    return env;
}

int main()
{
    std::string cgi_file_name = "cgi_result_test";
    int chiled_file;
    std::cerr << "hello" << std::endl;

    if ((chiled_file = open(cgi_file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1)//쓰기, 없으면만듬, 덮어쓰기.
    {
        std::cerr << "can not create" << std::endl;
        return -1; //바로 에러 페이지 제작 필요.
    }
    int post_body_fd;
    if ((post_body_fd = open("./socket.cpp", O_RDONLY, 0755)) == -1)//읽기전용.
    {
        std::cerr << "can not open" << std::endl;
        return -1; //바로 에러 페이지 제작 필요.
    }
    int pid = -1;
    if ((pid = fork()) < 0)
    {
        return -1; //바로 에러 페이지 제작 필요.
    }
    if (pid == 0) //자식프로세스 일 때.
    {
        char **env = init_cgi_env(); //환경변수 준비.
        dup2(post_body_fd, 0);
        dup2(chiled_file, 1); //출력 리다이렉트.
        char **arg = (char **)malloc(sizeof(char *) * 2);
        arg[0] = strdup("../cgi-bin/cgi_tester");
        arg[1] = NULL;
        if (execve(arg[0], arg, env) == -1)
        {
            perror("execve err bla cgi");
            exit(1);
        }
        exit(0);
    }
    else
    {
        close(chiled_file);
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
            return (-1);
        close(post_body_fd);
        return 0;
    }
}

