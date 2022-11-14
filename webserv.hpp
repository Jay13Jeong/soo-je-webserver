#include <vector>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stack>
#include <unistd.h>
#include <algorithm>
#include "server.hpp"
#include "client.hpp"
#include <sys/socket.h> //socket
// #include <sys/un.h>
#include <sys/event.h> //kqueue

class Webserv
{
private:
    std::vector<Server*> _server_list;
    std::vector<Client*> _client_list;

public:
    webserv(/* args */);
    ~webserv();
    std::vector<Server*> get_server_list()
    {
        return this->_server_list;
    };
    void set_server_list(Server *new_server)
    {
        this->_server_list.push_back(new_server);
    };
    std::vector<Client*> get_client_list()
    {
        return this->_client_list;
    };
    void set_client_list(Client *new_client)
    {
        this->_client_list.push_back(new_client);
    };

    /*
        - check bracket
        - 일단 정상적인 값이 들어온다는 전제
        - stack
        - 나올수 있는 예시들 listup하고서 find한다. (listen, root, index, location)
        - variable nameing rule check
            - start with _, alphabet
            - no start with digit
        - key값 중복 여부 확인 필요

    */
    bool    check_config_validation(ifstream config_fd)
    {
        std::string line;
        stack<string> bracket_stack;

        while (!config_fd.eof())
        {
            getline(config_fd, line);
            // check 조건 추가 bracket, variable naming condition
        }
        return (true);
    }

    //conf파일을 인자를 받아 파싱 메소드.
    bool    parsing(std::string conf_file)
    {
        ifstream    config_fd;
    	std::string line;
        std::string options[5] = {"server", "listen", "root", "index", "location"};

    	config_fd = open(argv[2], O_RDONLY);
        // ifstream 실패했을 경우 -> good() false
        if (config_fd.good() == false)
            return (false);
        if (check_config_validation(config_fd) == false)
        {
            close(config_fd);
            return (false)
        }
        close(config_fd);
        // 유효성 확인된 경우 파싱 시작
        config_fd = open(argv[2], O_RDONLY);

	    while (!config_fd.eof())
	    {
		    getline(config_fd, line);
		    //함수 호출, 정규표현식은 없다.
		    //첫줄이 server 로 시작하면 server 리스트에 1개 추가
            if (std::find(line.begin(), line.end(), "server") == line.begin())
            {
                Server *new_server = new Server(); // 메모리 해제 신경써줘야함;;;
                set_server_list(new_server);
            }
			/*
                중괄호 숫자 +-세기, 0이면 함수 나오기
				한줄씩 읽어서 공백기준으로 나누기 -> ft_split 만들기.
                루트는 인자가 1개, but index의 인자가 여러개이면?,fastcgi는? -> 일단은 list로 처리
                각각의 부분에 대한 예외처리를 함수를 추가로 불러서 하도록 하자. 특히 location
			*/
		//내용물 채우고 마지막 } 만나면 나오기
		//함수호출 끝
    	}
        close(config_fd);
        return (true);
    }

    //서버를 실행하는 메소드.
    void start()
    {
        int kq_fd;

        kq_fd = kqueue();
    }
};
