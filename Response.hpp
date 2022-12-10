#ifndef RESPONSE_CLASS_HPP
# define RESPONSE_CLASS_HPP
#include <string>
#include <vector>
#include <map>
#include "util.hpp"

#define LENGTHLESS "700"
#define CHUNKED "800"

class Response
{
public:
    Response(/* args */) : status(""), body(""), sid(0) {};
    ~Response(){};

    void set_sid(long id)
    {
        this->sid = id;
    }
    long get_sid()
    {
        return this->sid;
    }
    std::string & getVersion()
    {
        return this->version;
    }
    void setVersion(std::string version)
    {
        this->version = version;
    }
    std::string & getStatus()
    {
        return this->status;
    }
    void setStatus(std::string status)
    {
        this->status = status;
    }
    std::string & getStatus_msg()
    {
        return this->status_msg;
    }
    void setStatus_msg(const std::string & status_msg)
    {
        this->status_msg = status_msg;
    }
    std::map<std::string, std::string> & getHeader_map()
    {
        return this->header_map;
    }
    void setHeader_map(const std::string & key, const std::string & value)
    {
        this->header_map.insert(std::make_pair(key, value));
    }
    std::string & getBody()
    {
        return this->body;
    }
    void setBody(const std::string & body)
    {
        this->body = body;
    }

    void clear_response()
    {
        this->body.clear();
        this->header_map.clear();
        this->version.clear();
        this->status_msg.clear();
        this->status.clear();
        this->sid = 0;
    }

    void init_headers()
    {
        if (this->getHeader_map().find("server") == this->getHeader_map().end())
            this->setHeader_map("server", "soo-je-webserver");
        if (this->getHeader_map().find("Date") == this->getHeader_map().end())
            this->setHeader_map("Date", util::get_date());
        if (this->getHeader_map().find("Connection") == this->getHeader_map().end())
            this->setHeader_map("Connection", "keep-alive");
        if (this->getHeader_map().find("Accept-Ranges") == this->getHeader_map().end())
            this->setHeader_map("Accept-Ranges", "bytes");
    }

    void init_delete_response(std::map<std::string, std::string> *status_msg, std::string &write_buf, bool cgi_mode)
    {
        this->setVersion("HTTP/1.1");
        this->setStatus("204");
        this->setStatus_msg((*(status_msg)).find(getStatus())->second);

        if (this->getHeader_map().find("server") == this->getHeader_map().end())
            this->setHeader_map("server", "soo-je-webserver");
        if (this->getHeader_map().find("Date") == this->getHeader_map().end())
            this->setHeader_map("Date", util::get_date());
        if (this->getHeader_map().find("Connection") == this->getHeader_map().end())
            this->setHeader_map("Connection", "keep-alive");
        if (this->getHeader_map().find("Accept-Ranges") == this->getHeader_map().end())
            this->setHeader_map("Accept-Ranges", "bytes");

        this->setBody("");
        push_write_buf(this->getBody(), write_buf, cgi_mode);
        //DELETE용 응답데이터 (시작줄 + 헤더 + 바디)만들기....
    }

    void redirect_response(std::map<std::string, std::string> *status_msg, std::string &write_buf, bool cgi_mode)
    {
        this->setVersion("HTTP/1.1");
        this->setStatus("301");
        this->setStatus_msg((*(status_msg)).find(this->getStatus())->second);
        this->setBody("");
        this->push_write_buf(this->getBody(), write_buf, cgi_mode);
    }

    void push_write_buf(const std::string &response_body, std::string &write_buf, bool cgi_mode)
    {
        //스타트라인
        std::cerr << YELLOW << this->getVersion() + " " + this->getStatus() + " " + this->getStatus_msg() << RESET << std::endl;
        write_buf = this->getVersion() + " " + this->getStatus() + " " + this->getStatus_msg() + "\r\n";
        //헤더 부분
        std::map<std::string, std::string> temp = this->getHeader_map();
    	std::map<std::string,std::string>::reverse_iterator iter;
	    for(iter = temp.rbegin() ; iter != temp.rend(); iter++)
		    write_buf = write_buf + iter->first + ": " + iter->second + "\r\n";
        //개행추가 부분, cgi의 경우 바디 윗부분에 개행이 추가되어있다.바디에 개행이 추가되는 것을 방지.
        if (cgi_mode == false)// || request.getMethod() != "HEAD")
            write_buf = write_buf + "\r\n";
        //바디 부분
        if (response_body.size() != 0)// && request.getMethod() != "HEAD")
            write_buf += response_body;
    }

    void find_mime_type(std::string & path)
    {
        std::string temp = "";
        //값"text/html",text/css, images/png, jpeg, gif
        //헤더파일형식 Content-Type: text/html;

        if (util::ft_split(path, "./").size() != 0)
            temp = util::ft_split_s(util::ft_split_s(path, "/").back(), ".").back();//파일 확장자만 반환하기
        if (temp == "css")
            this->setHeader_map("Content-Type", "text/css");
        else if (temp == "png")
            this->setHeader_map("Content-Type", "image/png");
        else if (temp == "jpeg")
            this->setHeader_map("Content-Type", "image/jpeg");
        else if (temp == "gif")
            this->setHeader_map("Content-Type", "image/gif");
        else
            this->setHeader_map("Content-Type", "text/html");// ; charset=UTF-8");
    }

private:
    std::string version; //http버전
    std::string status; //상태코드 (ex "505").
    std::string status_msg; //상태코드 해석 메세지.
    std::map<std::string,std::string> header_map; //헤더필드의 키=값.
    std::string body; //바디.
    long sid; //세션 id

};

#endif