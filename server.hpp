#ifndef SERVER_CLASS_HPP
# define SERVER_CLASS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h> //INADDR_ANY
#include <fcntl.h> //fcntl
#include "location.hpp"

class Server
{
public:
    int                                 port; //서버포트. (기본값 있음)
    std::string                         server_name; //서버이름. (기본값 있음)
    std::string                         root; //실행되는 최상위 폴더. (기본값 있음)
    std::vector<std::string>            index; //인덱스 파일목록. (기본값 있음 - index.html)
    std::vector<Location>               loc; //로케이션 구조체 배열
    bool                                autoindex; //오토인덱스. (기본값 있음 - off)
    size_t                              client_max_body_size; //서버가 수신가능한 최대 데이터 크기. (기본값 있음 ????????)
    std::map<std::string, std::string>  default_error_pages; // 키:status code 값:에러페이지
    int                                 fd; //linsten용 서버 fd.
    std::map<std::string,std::string>   cgi_map; // 키:확장자, 값:확장자 경로(python,java)
    std::map<std::string, Location>       loc_map; //로케이션 구조체 맵.

private:
    struct sockaddr_in          t_address; //포트개방용 변수. 소켓에 이식할 주소구조체.(초기화 안됨)

public:
    Server() : fd(-1) {
        port = 80;
        server_name = "soo-je";
        // root = "";
        index = {"index.html"};
        autoindex = false;
        client_max_body_size = 1000000;
    };
    ~Server()
    {
        if (this->fd != -1)
            close(this->fd);
    }

    std::string get_root()
    {
        return this->root;
    }
    std::string get_server_name()
    {
        return this->server_name;
    }
    int get_port()
    {
        return this->port;
    }
    std::vector<std::string> get_index()
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
    std::map<std::string, std::string>  get_default_error_page()
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

        int size = sizeof(t_address);
        if ((new_socket = accept(this->fd, (struct sockaddr*)&t_address, (socklen_t*)&size)) < 0) 
        {
            perror("accept");
            //**throw
        }
        fcntl(new_socket, F_SETFL, O_NONBLOCK); //NON-BLOCKING설정
        return new_socket;
    }

    //서버의 포토를 여는 메소드.
    void open_port( void )
    {
        //깡통 소켓 생성.
        if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket failed");
            //**throw
        }

        fcntl(this->fd, F_SETFL, O_NONBLOCK); //NON-BLOCKING설정 
        //** (이외에 파일FD에도 해준다.)
    
        int opt = 1;
        // 예약) 포트가 선점되어 있을 시 강제로 열도록 예약한다.
        if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            //**throw
        }
        
        // int addrlen = sizeof(t_address);
        t_address.sin_family = AF_INET; //주조체계를 ipv4로 초기화한다.
        t_address.sin_addr.s_addr = INADDR_ANY; //주조를 localhost로 초기화한다.
        t_address.sin_port = htons(this->port); //포트를 네트워크형식으로 전환해서 초기화.
    
        //초기화된 주소구조체로 소켓을 바인드.
        if (bind(this->fd, (struct sockaddr*)&t_address, sizeof(t_address)) < 0) {
            perror("bind failed");
            //**throw
        }
        //포트열기. 한 서버당 접속대기열을 1024개까지 받는다.
        if (listen(this->fd, 1024) < 0) {
            perror("listen");
            //**throw
        }
        // g_io_infos[this->fd] = IO_manager(this->fd, "server", 0);
    }

    //로케이션 구조체가 하나도 없으면 서버의 기본 필드로 '/'경로를 만드는 메소드.
    void init_default_location()
    {
        //if loc_map이 비었다면. (뭔가 존재하면 그냥 리턴하고 종료.)
        if (this->loc_map.empty())
            return;
        //default_loc초기화.....
        // Location default_loc(this->root, this->index, this->autoindex);
        Location default_loc = Location(this->root, this->index, this->autoindex);
        this->loc_map.insert(std::make_pair("/", default_loc));
        //this->loc_map["/"] = default_loc;
    }
};

#endif