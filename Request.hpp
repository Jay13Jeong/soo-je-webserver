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
    std::string getMethod()
    {
        return this.method;
    }

public:
    void setMethod(std::string method)
    {
        this.method = method;
    }

public:
    std::string getTarget()
    {
        return this.target;
    }

public:
    void setTarget(std::string target)
    {
        this.target = target;
    }

public:
    std::string getVersion()
    {
        return this.version;
    }

public:
    void setVersion(std::string version)
    {
        this.version = version;
    }

public:
    std::map<std::string, std::string> getHeaders()
    {
        return this.headers;
    }

public:
    void setHeaders(std::map<std::string, std::string> headers)
    {
        this.headers = headers;
    }

public:
    std::string getBody()
    {
        return this.body;
    }

public:
    void setBody(std::string body)
    {
        this.body = body;
    }

    //데이터를 받아서 파싱하는 메소드.
    bool parse(std::string & data, std::string & status_code)
    {
        //
        return true; //문제없으면 true반환;
    }

public:
    Request(/* args */);
    ~Request();
};

