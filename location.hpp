#ifndef LOCATION_CLASS_HPP
# define LOCATION_CLASS_HPP

#include <string>
#include <vector>
#include <utility>

class Location
{
public:
    //생성자..
    Location() { };
    Location(std::string root, std::vector<std::string> index, bool autoindex)
    {
        this->path = "/";
        this->root = root;
        // redirection.insert(std::make_pair("", ""));
        this->index = index;
        this->autoindex = autoindex;
        accept_method.push_back("GET");
        accept_method.push_back("HEAD");
        // accept_method.push_back("POST");
        // accept_method.push_back("DELETE");
        // accept_method.push_back("PUT");
        // accept_method = {"GET", "POST", "DELETE"};
    };
    ~Location(){}
    std::string path; // 이 패턴으로 시작하는 경우에 location block 적용.
    std::string root; // 실행 되는 최상위 폴더
    std::map<std::string, std::string>redirection; // key : status code, value : redirect target // 301 302 리다이렉션 정보. // 맵으로 하기
    std::vector<std::string> index; //기본지정파일.벡터로 받게 하기
    bool        autoindex;
    std::vector<std::string> accept_method;//GET POST DELETE 만 받기
    std::map<std::string,std::string>   cgi_map; // 키:확장자, 값:확장자 경로(python,java)
    std::map<std::string, Location>       loc_map; //로케이션 구조체 맵.
};

#endif