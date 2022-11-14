#include <string>

struct Location
{
    std::string root; //실행 되는 최상위 폴더
    int         redirection; // 301 302리다이렉션 정보.
    std::string defaultFile; //기본지정파일.
    std::string uploadFolder; //업로드파일을 저장할 폴더 경로.
    bool        autoindex;
};
