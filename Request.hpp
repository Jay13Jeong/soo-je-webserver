#ifndef REQUEST_CLASS_HPP
# define REQUEST_CLASS_HPP
#include <vector>
#include <string>
#include <map>
#include "util.hpp"
#include <cstdlib>

class Request
{
private:
    std::string method; //메소드.
    std::string target; //대상파일(또는 폴더)의 경로.
    std::string version; //프로토콜버전. http1.1고정이기 때문에 float보다 string.
    std::map<std::string,std::string> headers; //콜론을 기준으로 나눈 헤더 키:값 들.
    std::string body; //바디부분 데이터.

private:
    bool ft_chunk_fin_check(std::string data, std::string temp_str, std::string &status_code)
    {
        std::vector<std::string> temp = util::ft_split(temp_str, ", ");

        for (int i = 0; i < temp.size(); i++)
        {
            if (temp[i] == "chunked")
            {
                status_code = "800";
                break ;
            }
        }
        if (status_code != "800")// 청크가 아닐 경우
            return (status_code = "200", true);
        if ((data.size() - 5) == data.rfind("0\r\n\r\n"))//종료 위지 같으면
            return (status_code = "800", true);
        return (status_code = "800", false);
    }
private:
    bool ft_chunk_push_body(std::vector<std::string> temp_data, int i, std::string &status_code)
    {
        std::string temp = "";
        for (int j = i + 1; j < temp_data.size(); j+=2)//바디부터 시작
        {
            if (j + 1 >= temp_data.size())
                return (status_code = "400", false);
            if (strtol(temp_data[j].c_str(), NULL, 16) != temp_data[j + 1].size())
                return (status_code = "400", false);
            temp = temp + temp_data[j + 1];
        }
        setBody(temp);
        return (status_code = "200", true);
    }

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
    std::string &getTarget()
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
    std::map<std::string, std::string> & getHeaders()
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
        // // std::cerr << "request.parse() 함수에 들어온 데이터"<< std::endl;
        // // std::cerr << data << std::endl;
        // // std::cerr << "여기까지"<< std::endl;
        std::vector<std::string> temp_data = util::ft_split_s(data, "\r\n");
        std::vector<std::string> temp_str;
        std::string temp = "";
        status_code = "";//초기화

        //스타트라인
        temp_str = util::ft_split_s(temp_data[0], " ");
        if (temp_str.size() != 3)
            return (status_code = "400", false);
        else if (temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST" || temp_str[0] == "PUT" || temp_str[0] == "HEAD")
            setMethod(temp_str[0]);
        else if (!(temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST" || temp_str[0] == "PUT" || temp_str[0] == "HEAD"))
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

            if (temp_str[0] == "Transfer-Encoding")//청크부분 처리
            {
                if (!ft_chunk_fin_check(data, temp_str[1], status_code))
                    return (false);
            }
            temp = temp_str[1];
            for (int j = 2; j < temp_str.size(); j++)
                temp = temp + ":" + temp_str[j];
            setHeaders(temp_str[0], temp);
            i++;
        }
        if (i == 1)
            return (status_code = "400", false);
        else if ((getMethod() == "GET" || getMethod() == "DELETE" || temp_str[0] == "HEAD") && !(i == temp_data.size() || i == temp_data.size() - 1))
            return (status_code = "400", false);
        // else if ((getMethod() == "POST") && (i == temp_data.size() || i == temp_data.size() - 1))// Post에서 바디가 없을 수도 있다
        //     return (status_code = "411", false);

        //바디부분
        if (status_code == "800")
            return (ft_chunk_push_body(temp_data, i, status_code));
        temp = "";
        while (i < temp_data.size())
        {
            temp = temp + temp_data[i];
            //temp = temp + "\r\n";
            i++;
        }

        if (this->getHeaders().find("Content-Length") == this->getHeaders().end())
            setBody(temp);
        else if (strtol(this->getHeaders()["Content-Length"].c_str(), NULL, 10) >= temp.size())
            setBody(temp);
        else if (strtol(this->getHeaders()["Content-Length"].c_str(), NULL, 10) < temp.size())
            setBody(temp.substr(0, strtol(this->getHeaders()["Content-Length"].c_str(), NULL, 10)));

        return (status_code = "200", true); //문제없으면 true반환;
    }

    void clear_request()
    {
        this->body.clear();
        this->headers.clear();
        this->method.clear();
        this->target.clear();
        this->version.clear();
    }

public:
    Request(/* args */){};
    ~Request(){};
};

#endif