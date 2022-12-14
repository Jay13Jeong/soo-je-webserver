#ifndef CLIENT_BASE_HPP
# define CLIENT_BASE_HPP
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <dirent.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.hpp"
#include "server.hpp"
#include "location.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFER_SIZE 10000

class Client_base
{
public:
    Client_base(){}
    ~Client_base(){}
    std::string & get_read_buf()
    {
        return this->read_buf;
    }
    bool is_cgi_mode()
    {
        return this->cgi_mode;
    }
    void set_cgi_mode(bool cgi_mode)
    {
        this->cgi_mode = cgi_mode;
    }
    std::string & get_file_buf()
    {
        return this->file_buf;
    }
    void set_file_buf(std::string & file_buf)
    {
        this->file_buf = file_buf;
    }
    int & get_file_fd()
    {
        return this->file_fd;
    }
    void set_file_fd(int file_fd)
    {
        this->file_fd = file_fd;
    }
    int get_socket_fd()
    {
        return this->socket_fd;
    }
    void set_socket_fd(int socket_fd)
    {
        this->socket_fd = socket_fd;
    }
    Request & get_request()
    {
        return this->request;
    }
    void set_request(Request & request)
    {
        this->request = request;
    }
    Response & get_response()
    {
        return this->response;
    }
    void set_response(Response & response)
    {
        this->response = response;
    }

    //클라이언트 소켓에서 데이터를 읽어서 본인의 read_buf버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int recv_data( void )
    {
        size_t size;
        char    buffer[BUFFER_SIZE];

        size = recv(this->socket_fd, buffer, BUFFER_SIZE, 0);
        if (size == (size_t)-1)
        {
            perror("recv client err");
            return -1;
        }
        else if (size == 0)
        {
            return -1;
        }
        else
        {
            if (this->response.get_status() == LENGTHLESS)
            {
                this->request.get_body() += std::string(buffer, size);
                return 1;
            }
            
            this->read_buf += std::string(buffer, size); //1.읽은 데이터 char[] -> string으로 변환해서 저장.
            if (this->response.get_status() == CHUNKED)
            {
                const size_t body_size = this->read_buf.length();
                const std::string & body_data = this->read_buf;
                size_t curr = 0;
                while (body_size > 2)
                {
                    size_t rn = body_data.find("\r\n", curr);
                    if (rn == curr)
                    {
                        this->read_buf.clear();
                        this->response.set_status("400");
                        this->is_done_chunk = true;
                        return 1;
                    }
                    if (rn == std::string::npos)
                    {
                        if (curr != 0)
                        {
                            if (curr != body_size)
                                this->read_buf = this->read_buf.substr(curr);
                            else
                                this->read_buf.clear();
                        }
                        return 1;
                    }
                    size_t b = strtol(body_data.substr(curr, rn - curr).c_str(), NULL, 16);
                    size_t a = body_size - (rn + 2);
                    if (a < (b + 2))
                    {
                        if (curr != 0)
                        {
                            if (curr != body_size)
                                this->read_buf = this->read_buf.substr(curr);
                            else
                                this->read_buf.clear();
                        }
                        return 1;
                    }
                    if (b == 0)
                    {
                        this->read_buf.clear();
                        this->response.set_status("200");
                        this->is_done_chunk = true;
                        return 1;
                    }   
                    this->request.get_body() += body_data.substr(rn + 2, b);
                    if (b != 0)
                    {
                    }
                    curr = (rn + 2) + (b + 2);
                }
                return 1;
            }
            if (size == BUFFER_SIZE)
                return 0;
            if (this->read_buf.find("\r\n\r\n") != std::string::npos) //모두 읽었다면..
                return 1;
            if (this->read_buf.find("\n\n") != std::string::npos) //모두 읽었다면..
                return 1;

        }
        return 0;
    }

    //지정한 파일에서 데이터를 읽어서 본인의 file_buf버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int read_file( void )
    {
        size_t size;
        char    buffer[BUFFER_SIZE];

        size = read(this->file_fd, buffer, BUFFER_SIZE);
        if (size == (size_t)-1)
        {
            close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
            if (this->cgi_mode == true)
                unlink(this->cgi_file_name.c_str());
            return -1;
        }
        this->file_buf += std::string(buffer, size);
        if (size < BUFFER_SIZE)
        {
            close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
            if (this->cgi_mode == true)
                unlink(this->cgi_file_name.c_str());
            return 1;
        }
        return 0;
    }

    //지정한 파일에 바디데이터를 write하는 메소드. 실패 -1 성공 0 모두보냄 1 반환.
    int write_file( void )
    {
        size_t size;

        size = write(this->get_file_fd(), request.get_body().c_str() + (this->write_size), request.get_body().length() - (this->write_size));
        if (size == (size_t)-1)
        {
            close(this->file_fd);
            this->write_size = 0;
            return -1;
        }
        this->write_size += size;
        if (this->write_size >= request.get_body().length())
        {
            close(this->file_fd);
            this->write_size = 0;
            return 1;
        }
        return 0;
    }

    //응답데이터를 소켓에게 전송하는 메소드.
    int send_data()
    {
        size_t size;

        size = send(this->socket_fd, this->write_buf.c_str() + (this->write_size), this->write_buf.length() - (this->write_size), 0);
        if (size == (size_t)-1) //데이터전송 실패 했을 때.
            return -1; //호출한 부분에서 이 클라이언트 제거.

        this->write_size += size;
        if (this->write_size >= this->write_buf.length())
        {
            #ifdef TEST
            std::cerr << "0000000000000000000000000000000000000000000" << std::endl;
            std::cerr << this->write_buf << std::endl;
            std::cerr << "0000000000000000000000000000000000000000000" << std::endl;
            #endif
            return 1; //호출한 부분에서 클라이언트 객체를 초기화하는 함수 실행.
        }
        //전송중이면 (다 못보냈을 때)
        return 0;
    }
protected:
    int socket_fd; //클라이언트 소켓 fd.
    std::string read_buf; //소켓에서 읽어온 비정제 데이터. (추후 파싱필요)
    std::string write_buf; //응답 클래스로 보낼 데이터.
    Request request; //비정제데이터를 파싱해서 만든 request클래스.
    Response response; //response를 제작하는 클래스.
    int file_fd; // cgi가 출력한 결과물을 담는 파일의 fd.
    std::string file_buf; //파일의 정보가 저장되는 변수.
    size_t write_size; //보낸 데이터 크기.
    bool cgi_mode; // cgi모드여부.
    std::string  cgi_file_name; //cgi 결과물을 담은 파일 이름.
    bool is_done_chunk;
};

#endif