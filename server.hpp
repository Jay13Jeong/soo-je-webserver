#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "location.hpp"

class Server
{
private:
    int                         port; //서버포트.
    std::string                 server_name; //서버이름.
    std::string                 root; //실행되는 최상위 폴더.
    std::vector<std::string>    index; //인덱스 파일목록.
    Location                    loc; //로케이션 구조체
    bool                        autoindex; //오토인덱스.
    size_t                      client_max_body_size; //서버가 수신가능한 최대 데이터 크기.
    std::string                 default_error_page; //기본 에러페이지.
    int                         fd; //linsten용 서버 fd.
    std::map<std::string,std::string>   cgi_map; // 키:확장자, 값:확장자 경로(python,java)

public:
    server(/* args */);
    ~server();
};

server::server(/* args */)
{
}

server::~server()
{
}
