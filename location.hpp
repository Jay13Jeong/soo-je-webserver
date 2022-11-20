// #ifdef LOCATION_HPP
// # define LOCATION_HPP
#include <string>
#include <vector>

class Location
{
public:
    Location()
    {
        accept_method.push_back("GET");
        accept_method.push_back("POST");
        accept_method.push_back("DELETE");
    }
    std::string root; //실행 되는 최상위 폴더
    std::map<std::string, std::string>redirection; // 301 302리다이렉션 정보.//맵으로 하기
    std::vector<std::string> index; //기본지정파일.벡터로 받게 하기
    bool        autoindex;
    std::vector<std::string> accept_method;//GET POST DELETE 만 받기
};
// #endif