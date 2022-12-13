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
    std::string & get_version()
    {
        return this->version;
    }
    void set_version(std::string version)
    {
        this->version = version;
    }
    std::string & get_status()
    {
        return this->status;
    }
    void set_status(std::string status)
    {
        this->status = status;
    }
    std::string & get_status_msg()
    {
        return this->status_msg;
    }
    void set_status_msg(const std::string & status_msg)
    {
        this->status_msg = status_msg;
    }
    std::map<std::string, std::string> & get_header_map()
    {
        return this->header_map;
    }
    void set_header_map(const std::string & key, const std::string & value)
    {
        this->header_map.insert(std::make_pair(key, value));
    }
    std::string & get_body()
    {
        return this->body;
    }
    void set_body(const std::string & body)
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
        if (this->get_header_map().find("server") == this->get_header_map().end())
            this->set_header_map("server", "soo-je-webserver");
        if (this->get_header_map().find("Date") == this->get_header_map().end())
            this->set_header_map("Date", util::get_date());
        if (this->get_header_map().find("Connection") == this->get_header_map().end())
            this->set_header_map("Connection", "keep-alive");
        if (this->get_header_map().find("Accept-Ranges") == this->get_header_map().end())
            this->set_header_map("Accept-Ranges", "bytes");
    }

    void find_mime_type(std::string &path)
    {
        std::string temp = "";
        //값"text/html",text/css, images/png, jpeg, gif
        //헤더파일형식 Content-Type: text/html;

        if (util::ft_split(path, "./").size() != 0)
            temp = util::ft_split_s(util::ft_split_s(path, "/").back(), ".").back();//파일 확장자만 반환하기

        if (temp == "css")
            this->set_header_map("Content-Type", "text/css");
        else if (temp == "png")
            this->set_header_map("Content-Type", "image/png");
        else if (temp == "jpeg")
            this->set_header_map("Content-Type", "image/jpeg");
        else if (temp == "gif")
            this->set_header_map("Content-Type", "image/gif");
        else
            this->set_header_map("Content-Type", "text/html");// ; charset=UTF-8");
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