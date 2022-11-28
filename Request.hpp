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
    bool ft_chunk_push_body(std::string temp_data, std::string &status_code)
    {
        std::string temp = "";
        size_t count;
        for (int i = 0; i < temp_data.size();)//바디부터 시작
        {
            count = strtol(temp_data.substr(i).c_str(), NULL, 16);//16진수로 읽을 수 있는 곳까지 숫자를 읽음

            i = i + temp_data.substr(i).find("\r\n");//청크 길이 뒤에 처음오는 \r\n 시작위치
            if (i == std::string::npos)
                return (status_code = "400", false);
            i += 2;//\r\n건너띄기

            temp = temp + temp_data.substr(i, count);//바디 추가

            if (i + count != i + temp_data.substr(i).find("\r\n"))//청크 바디 뒤에 \r\n 확인
                return (status_code = "400", false);
            i = i + count + 2;//다음
        }
        setBody(temp);
        return (status_code = "200", true);
    }
private:
    size_t ft_find_header_end(std::string data)
    {
        size_t num = data.find("\r\n\r\n");//헤더와 바디 사이의 공백 찾기, 헤더의 마지막 부분.

        if (num != std::string::npos)
            return num;
        num = data.rfind("\r\n");//\r\n이 중간에 없으면 끝에서 부터 찾기 시작, 바디가 없다고 추측, 헤더가 \r\n으로 끝날때
        if (num != data.size() - 2)//끝에 \r\n 이 없는 경우
            return data.size();
        return num;
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
        size_t data_header_end_point = ft_find_header_end(data);//헤더 마지막 부분 찾기
        if (data_header_end_point == 0)
            return (status_code = "400", false);

        std::vector<std::string> temp_data = util::ft_split_s(data.substr(0, data_header_end_point), "\r\n");
        std::vector<std::string> temp_str;
        std::string temp = "";
        status_code = "";//초기화

        //스타트라인
        temp_str = util::ft_split_s(temp_data[0], " ");
        if (temp_str.size() != 3)//스타트라인 규격 체크
            return (status_code = "400", false);
        else if (temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST" || temp_str[0] == "PUT" || temp_str[0] == "HEAD")
            setMethod(temp_str[0]);
        else if (!(temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST" || temp_str[0] == "PUT" || temp_str[0] == "HEAD"))
            return (status_code = "405", false);

        if (temp_str[2] == "HTTP/1.1")
            setVersion(temp_str[2]);
        else if (!(temp_str[2] == "HTTP/1.1"))//HTTP/1.1만 지원
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
        if (i == 1)//헤더가 없는 경우,temp_data.size()==i아니면 헤더 파싱 다 된 것 아님.
            return (status_code = "400", false);
        else if ((getMethod() == "GET" || getMethod() == "DELETE" || getMethod() == "HEAD") && !(data_header_end_point >= (data.size() - 4)))//-4는 올수 있는CRLF의 최대값
            return (status_code = "400", false);
        // else if ((getMethod() == "POST") && (i == temp_data.size() || i == temp_data.size() - 1))// Post에서 바디가 없을 수도 있다
        //     return (status_code = "411", false);

        //바디부분
        if (status_code == "800")
            return (ft_chunk_push_body(data.substr(data_header_end_point + 4), status_code));//바디 처음부터 시작

        temp = "";
        //while (i < temp_data.size())
        //{
        //    temp = temp + temp_data[i];
        //    //temp = temp + "\r\n";
        //    i++;
        //}

        if (data.size() > data_header_end_point + 4)//바디 시작부분이 요청값 전체 사이즈 보다 작아야 한다.
            temp = temp + data.substr(data_header_end_point + 4);//바디 부분 시작지점.+4 "\r\n\r\n"이후 바디 첫번쨰 값.

        if (this->getHeaders().find("Content-Length") == this->getHeaders().end())//"Content-Length"없음
            setBody(temp);
        else if (strtol(this->getHeaders()["Content-Length"].c_str(), NULL, 10) >= temp.size())//"Content-Length"길이가 더 큼
            setBody(temp);
        else if (strtol(this->getHeaders()["Content-Length"].c_str(), NULL, 10) < temp.size())//바디 크기가 더크면 잘라서 넣기
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