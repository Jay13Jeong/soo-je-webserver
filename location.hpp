// #ifdef LOCATION_HPP
// # define LOCATION_HPP
#include <string>

class Location
{
public:
    std::string root; //실행 되는 최상위 폴더
    std::string redirection; // 301 302리다이렉션 정보.
    std::string index; //기본지정파일.
    bool        autoindex;
};
// #endif