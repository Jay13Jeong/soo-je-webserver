#ifndef REQUEST_CLASS_HPP
# define REQUEST_CLASS_HPP
#include <vector>
#include <string>
#include <map>
#include "util.hpp"
#include <cstdlib>

// Colors
#define RED "\x1b[0;31m"
#define BLUE "\x1b[0;34m"
#define GREEN "\x1b[0;32m"
#define YELLOW "\x1b[0;33m"
#define MAGENTA "\x1b[0;35m"
#define RESET "\x1b[0m"

class Request
{
private:
    std::string method; //메소드.
    std::string target; //대상파일(또는 폴더)의 경로.
    std::string version; //프로토콜버전. http1.1고정이기 때문에 float보다 string.
    std::map<std::string,std::string> headers; //콜론을 기준으로 나눈 헤더 키:값 들.
    std::string body; //바디부분 데이터.

private:
    void ft_chunk_check(std::string & temp_str, std::string &status_code)
    {
        std::vector<std::string> temp = util::ft_split(temp_str, ", ");

        for (size_t i = 0; i < temp.size(); i++)
        {
            if (temp[i] == "chunked")
            {
                status_code = "800";
                break ;
            }
        }
        if (status_code != "800")// 청크가 아닐 경우
            status_code = "200";
    }
public:
    bool ft_chunk_push_body(std::string &data, std::string &status_code)
    {
        std::string temp = "";
        size_t i = 0;
        size_t count;
        size_t cr;
        size_t chunk_count = 0;

        while ((chunk_count < 10) && (i < data.size()))//바디부터 시작
        {
            cr = data.find("\r\n", i) - i;//16진수 길이

            if (cr == std::string::npos)
                return (status_code = "400", false);

            count = strtol(data.substr(i, cr).c_str(), NULL, 16);//16진수로 읽을 수 있는 곳까지 숫자를 읽음

            i += cr;//청크 길이 뒤에 처음오는 \r\n 시작위치
            i += 2;//\r\n건너띄기

            if (chunk_count != 0 && (i + count != data.find("\r\n", i)))//추가로 받는 청크데이터가 아직 덜 받은 상태이라면..
                return (set_body(get_body() + temp), data = data.substr((i - cr - 2)), status_code = "800", false);

            temp += data.substr(i, count);//바디 추가
            i += count + 2;//다음 청크데이터 시작 위치
            chunk_count++;
        }
        set_body(get_body() + temp);
        if (count == 0)//뒤에 \r\n\r\n오는 건 확인을 해야하긴 하는데....
            return (data = "", status_code = "200", true);

        data = data.substr(i);//다음 청크 위치, 여기가 문제인가?
        return (status_code = "800", false);
    }
private:
    size_t ft_find_header_end(std::string & data)
    {
        size_t num = data.find("\r\n\r\n");//헤더와 바디 사이의 공백 찾기, 헤더의 마지막 부분.

        if (num != std::string::npos)
            return num;
        num = data.rfind("\r\n");//\r\n이 중간에 없으면 끝에서 부터 찾기 시작, 바디가 없다고 추측, 헤더가 \r\n으로 끝날때
        if (num != data.size() - 2)//끝에 \r\n 이 없는 경우
            return data.size();
        return num;
    }

private:
    bool ft_header_key_check(std::string &status_code)
    {
        if (this->headers.find("host") == this->headers.end())
            return (status_code = "400", false);

        return (true);
    }

public:
    std::string & get_method()
    {
        return this->method;
    }

public:
    void set_method(std::string method)
    {
        this->method = method;
    }

public:
    std::string &get_target()
    {
        return this->target;
    }

public:
    void set_target(std::string target)
    {
        this->target = target;
    }

public:
    std::string & get_version()
    {
        return this->version;
    }

public:
    void set_version(std::string version)
    {
        this->version = version;
    }

public:
    std::map<std::string, std::string> & get_headers()
    {
        return this->headers;
    }

public:
    void set_headers(const std::string & key,const std::string & value)
    {
        this->headers.insert(std::make_pair(key, value));
    }

public:
    std::string &get_body()
    {
        return this->body;
    }

public:
    void set_body(const std::string & body)
    {
        this->body = body;
    }
private:
    bool find_header_end(std::string & data, size_t &data_header_end_point)
    {
        data_header_end_point = ft_find_header_end(data);//헤더 마지막 부분 찾기
        if (data_header_end_point == 0)
            return (false);
        return (true);
    }

    bool push_start_line(std::string & temp_data, std::string &status_code)
    {
        std::cerr << BLUE << temp_data << RESET << std::endl;
        std::vector<std::string> temp_str = util::ft_split_s(temp_data, " ");
        if (temp_str.size() != 3)//스타트라인 규격 체크
            return (status_code = "400", false);
        else if (temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST" || temp_str[0] == "PUT" || temp_str[0] == "HEAD")
            set_method(temp_str[0]);
        else if (!(temp_str[0] == "GET" || temp_str[0] == "DELETE" || temp_str[0] == "POST" || temp_str[0] == "PUT" || temp_str[0] == "HEAD"))
            return (status_code = "405", false);
        if (temp_str[2] == "HTTP/1.1")
            set_version(temp_str[2]);
        else if (!(temp_str[2] == "HTTP/1.1"))//HTTP/1.1만 지원
            return (status_code = "505", false);
        else
            return (status_code = "400", false);
        set_target(temp_str[1]);//414에러는 uri길이 기준이 현재 없음
        return (true);
    }

    bool push_headers(std::string & data, std::vector<std::string> & temp_data, std::string &status_code, size_t & header_end_point)
    {
        std::vector<std::string> temp_str;
        std::string temp = "";
        size_t i = 1;
        //헤더 부분
        while (i < temp_data.size())
        {
            if (temp_data[i].size() == 0)
                break ;
            temp_str = util::ft_split(temp_data[i], ":");
//라인폴딩은 아직 처리안함 , 실제 사례를 본 적이 없음, 역사적 의미만 있음
            if (temp_str.size() == 1)
                return (status_code = "400", false);
            if (util::count_sp(temp_str[0]) != 0)
                return (status_code = "400", false);
            temp = temp_str[1];
            for (size_t j = 2; j < temp_str.size(); j++)
                temp = temp + ":" + temp_str[j];
            set_headers(temp_str[0], temp);
            i++;
        }
        if (i == 1)//헤더가 없는 경우,temp_data.size()==i아니면 헤더 파싱 다 된 것 아님.
            return (status_code = "400", false);
        else if ((get_method() == "GET" || get_method() == "DELETE" || get_method() == "HEAD") && !(header_end_point >= (data.size() - 4)))//-4는 올수 있는CRLF의 최대값
            return (status_code = "400", false);
        return (true);
    }

    bool push_body(std::string &data, std::string &status_code, size_t & data_header_end_point)
    {
        std::string temp = "";
        if (data.size() >= data_header_end_point + 4)
            data = data.substr(data_header_end_point + 4);//바디 부분 시작지점.+4 "\r\n\r\n"이후 바디 첫번쨰 값.
        //바디부분
        if (status_code == "800")
            return (ft_chunk_push_body(data, status_code));//바디 처음부터 시작

        if (data.size() > 0)//바디 시작부분이 요청값 전체 사이즈 보다 작아야 한다.
            temp = data;

        if (this->get_headers().find("Content-Length") == this->get_headers().end())//"Content-Length"없음
            set_body(temp);
        else if (strtol(this->get_headers()["Content-Length"].c_str(), NULL, 10) >= (long)temp.size())//"Content-Length"길이가 더 큼
            set_body(temp);
        else if (strtol(this->get_headers()["Content-Length"].c_str(), NULL, 10) < (long)temp.size())//바디 크기가 더크면 잘라서 넣기
            set_body(temp.substr(0, strtol(this->get_headers()["Content-Length"].c_str(), NULL, 10)));

        return (true);
    }

    void data_set(std::string &data, size_t & end_point)
    {//end_point에서 CRLF없을 수 있음, 최대 2번 올 수 있음
        data = data.substr(end_point);

        if (data[0] == '\r'){
            if (data.find("\r\n") == 0)
                data = data.substr(2);
        }
        if (data[0] == '\r'){
            if (data.find("\r\n") == 0)
                data = data.substr(2);
        }
    }

public:
    //데이터를 받아서 파싱하는 메소드. 200,400,405,505......414에러는 길이 기준이 현재 없음
    bool parse(std::string &data, std::string &status_code)
    {
        size_t data_header_end_point;

        if (!find_header_end(data, data_header_end_point))
            return (status_code = "400", false);

        std::vector<std::string> temp_data = util::ft_split_s(data.substr(0, data_header_end_point), "\r\n");
        std::vector<std::string> temp_str;
        std::string temp = "";

        if (!push_start_line(temp_data[0], status_code))//스타트라인
            return (false);

        if (!push_headers(data, temp_data, status_code, data_header_end_point))//헤더 부분
            return (false);

        if (status_code == "800")
            data_set(data, data_header_end_point);

        if (status_code == "800")
            return (ft_chunk_push_body(data, status_code));

        if (!push_body(data, status_code, data_header_end_point))
            return (false);

        return (status_code = "200", true);
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