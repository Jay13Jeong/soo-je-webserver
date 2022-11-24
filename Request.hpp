#ifndef REQUEST_CLASS_HPP
# define REQUEST_CLASS_HPP
#include <vector>
#include <string>
#include <map>
#include "util.hpp"

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
        return this->method;
    }

public:
    void setMethod(std::string method)
    {
        this->method = method;
    }

public:
    std::string & getTarget()
    {
        return this->target;
    }

public:
    void setTarget(std::string target)
    {
        this->target = target;
    }

public:
    std::string getVersion()
    {
        return this->version;
    }

public:
    void setVersion(std::string version)
    {
        this->version = version;
    }

public:
    std::map<std::string, std::string> getHeaders()
    {
        return this->headers;
    }

public:
    void setHeaders(std::string key, std::string value)
    {
        this->headers.insert(std::make_pair(key, value));
    }

public:
    std::string getBody()
    {
        return this->body;
    }

public:
    void setBody(std::string body)
    {
        this->body = body;
    }

    //데이터를 받아서 파싱하는 메소드. 200,400,405,505......414에러는 길이 기준이 현재 없음
    bool parse(std::string & data, std::string & status_code)
    {
        std::vector<std::string> temp_data = util::ft_split_s(data, "\r\n");
        std::vector<std::string> temp_str;
        std::string temp = "";

        //스타트라인
        temp_str = util::ft_split_s(temp_data[0], " ");
        if (temp_str.size() != 3)
            return (status_code = "400", false);
        else if (temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST")
            setMethod(temp_str[0]);
        else if (!(temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST"))
            return (status_code = "405", false);

        if (temp_str[2] == "HTTP/1.1")
            setVersion(temp_str[2]);
        else if (!(temp_str[2] == "HTTP/1.1"))
            return (status_code = "505", false);
        else
            return (status_code = "400", false);
        setTarget(temp_str[1]);//414에러는 uri길이 기준이 현재 없음

        int i = 1;
        //헤더 부분
        while (i < temp_data.size())
        {
            if (temp_data[i].size() == 0)
                break ;
            temp_str = util::ft_split_s(temp_data[i], ":");
//라인폴딩은 아직 처리안함 , 실제 사례를 본 적이 없음
            if (temp_str.size() == 1)
                return (status_code = "400", false);
            else if (util::count_sp(temp_str[0]) != 0)
                return (status_code = "400", false);
            for (int j = 1; j < temp_str.size(); j++)
                temp = temp + ":" + temp_str[j];
            setHeaders(temp_str[0], temp);
            i++;
        }
        if (i == 1)
            return (status_code = "400", false);
        else if ((getMethod() == "GET" || getMethod() == "DELETE") && !(i == temp_data.size() || i == temp_data.size() - 1))
            return (status_code = "400", false);
        else if ((getMethod() == "POST") && (i == temp_data.size() || i == temp_data.size() - 1))
            return (status_code = "400", false);
        //바디부분
        temp = "";
        while (i < temp_data.size())
        {
            temp = temp + temp_data[i];
            //temp = temp + "\r\n";
            i++;
        }
        setBody(temp);
        return (status_code = "200", true); //문제없으면 true반환;
    }

public:
    Request(/* args */){};
    ~Request(){};
};

#endif