#include <vector>
#include <string>
#include <map>

class Request
{
private:
    std::string method; //메소드.
    std::string target; //대상파일(또는 폴더)의 경로.
    std::string version; //프로토콜버전. http1.1고정이기 때문에 float보다 string.
    std::map<std::string,std::string> headers; //콜론을 기준으로 나눈 헤더 키:값 들. 
    std::string body; //바디부분 데이터.
public:
    Request(/* args */);
    ~Request();
};

Request::Request(/* args */)
{
}

Request::~Request()
{
}
