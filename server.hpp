#ifndef SERVER_CLASS_HPP
# define SERVER_CLASS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h> //INADDR_ANY
#include <fcntl.h> //fcntl
#include "util.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "location.hpp"

// Colors
#define RED "\x1b[0;31m"
#define BLUE "\x1b[0;34m"
#define GREEN "\x1b[0;32m"
#define YELLOW "\x1b[0;33m"
#define MAGENTA "\x1b[0;35m"
#define RESET "\x1b[0m"

class Server
{
public:
    int                                 port; //서버포트. (기본값 있음)
    std::string                         host; // host (기본적으로 127.0.0.1);
    std::string                         server_name; //서버이름. (기본값 있음)
    std::string                         root; //실행되는 최상위 폴더. (기본값 있음)
    std::vector<std::string>            index; //인덱스 파일목록. (기본값 있음 - index.html)
    std::vector<Location>               loc; //로케이션 구조체 배열
    bool                                autoindex; //오토인덱스. (기본값 있음 - off)
    size_t                              client_max_body_size; //서버가 수신가능한 최대 데이터 크기.
    std::map<std::string, std::string>  default_error_pages; // 키:status code 값:에러페이지
    int                                 fd; //linsten용 서버 fd.
    std::map<std::string,std::string>   cgi_map; // 키:확장자, 값:확장자 경로(python,java)
    std::map<std::string, Location>       loc_map; //로케이션 구조체 맵.
    std::map<long, std::string> sid_map; //세션키=값으로 구성된 맵.

public:
    Server() : fd(-1) {
        port = 80;
        host = "127.0.0.1";
        server_name = "soo-je";
        root = "/";
        index.push_back("index.html");
        autoindex = false;
        client_max_body_size = 100000000;
    };
    ~Server()
    {
    }
    std::map<long, std::string> & get_sid_map()
    {
        return this->sid_map;
    }
    void    set_host(std::string host)
    {
        this->host = host;
    }
    std::string get_root()
    {
        return this->root;
    }
    std::string get_host()
    {
        return (this->host);
    }
    std::string get_server_name()
    {
        return this->server_name;
    }
    int get_port()
    {
        return this->port;
    }
    std::vector<std::string> & get_index()
    {
        return this->index;
    }
    std::map<std::string, Location> & get_loc_map()
    {
        return this->loc_map;
    }
    std::vector<Location> & get_loc()
    {
        return this->loc;
    }
    bool get_autoindex()
    {
        return this->autoindex;
    }
    size_t get_max_body_size()
    {
        return this->client_max_body_size;
    }
    std::map<std::string, std::string> & get_default_error_page()
    {
        return this->default_error_pages;
    }
    int get_fd()
    {
        return this->fd;
    }
    std::map<std::string,std::string> & get_cgi_map()
    {
        return this->cgi_map;
    }

    //로케이션 맵을 초기화하는 메소드.
    void init_location_map()
    {
        for (std::vector<Location>::iterator it = this->loc.begin();it != this->loc.end(); it++)
            this->loc_map[(*it).path] = *it;
    }

    //서버가 connet를 수락하고 클라이언트fd를 생성해서 반환하는 메소드.
    int accept_client( void )
    {
        int new_socket(-1);
        struct sockaddr_in  t_address;
        int size = sizeof(t_address);

        if ((new_socket = accept(this->fd, (struct sockaddr*)&t_address, (socklen_t*)&size)) < 0)
        {
            perror("accept_fail...");
            return -1;
        }
        struct linger _linger;
        _linger.l_onoff = 1;
        _linger.l_linger = 0;
        if (setsockopt(new_socket, SOL_SOCKET, SO_LINGER , &_linger, sizeof(_linger)) == -1) {
            perror("set_sockopt fail...");
        }
        fcntl(new_socket, F_SETFL, O_NONBLOCK); //NON-BLOCKING설정
        return new_socket;
    }

    //서버의 포토를 여는 메소드.
    void open_port( void )
    {
        //깡통 소켓 생성.
        if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket create failed...");
            exit(1);
            //**throw
        }

        int opt = 1;
        // time wait포트를 재사용하도록 설정한다.
        if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) == -1) {
            perror("set_sockopt fail...");
            exit(1);
        }

        /////// getaddrinfo
        struct addrinfo hint; // getaddrinfo의 인자로 들어갈 힌트
        struct addrinfo *t_info; // 값을 저장할 구조체
        memset(&hint, 0, sizeof(hint));
        hint.ai_family = AF_INET; // IPv4
        hint.ai_socktype = SOCK_STREAM; // TCP Socket
        int ret = getaddrinfo(this->host.c_str(), util::num_to_string(this->port).c_str(), &hint, &t_info);
        if (ret != 0)
        {
            freeaddrinfo(t_info); // addrinfo 할당 해제
            std::cerr << gai_strerror(ret) << std::endl;
            exit(1);
        }
        if (bind(this->fd, t_info->ai_addr, sizeof(*(t_info->ai_addr))) == -1) {
            perror("bind failed...");
            freeaddrinfo(t_info);
            exit(1);
        }

        freeaddrinfo(t_info);
        //포트열기. 한 서버당 접속대기열을 접속대기열을 설정.
        if (listen(this->fd, 128) == -1) {
            perror("listen fail...");
            exit(1);
        }
        std::cerr << GREEN << "Listen " << this->host << ":" << this->port << RESET << std::endl;
        fcntl(this->fd, F_SETFL, O_NONBLOCK); //NON-BLOCKING설정
    }

    //로케이션 구조체가 하나도 없으면 서버의 기본 필드로 '/'경로를 만드는 메소드.
    void init_default_location()
    {
        //if loc_map이 비었다면. (뭔가 존재하면 그냥 리턴하고 종료.)
        if (this->loc_map.empty() == false)
            return;
        //default_loc초기화.....
        Location default_loc(this->root, this->index, this->autoindex, this->client_max_body_size, this->cgi_map);
        this->loc_map.insert(std::make_pair("/", default_loc));
    }
    //sid를 새로 생성하는 메소드.
    long create_sid()
    {
        static int new_id = 0;
        this->sid_map[++new_id] = "new";
        return new_id;
    }
};

#endif