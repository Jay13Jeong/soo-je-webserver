#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "location.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFER_SIZE 5120

class Client
{
private:
    int socket_fd; //클라이언트 소켓 fd.
    std::string data; //소켓에서 읽어온 비정제 데이터. (추후 파싱필요)
    Request request; //비정제데이터를 파싱해서 만든 request클래스.
    Response response; //response를 제작하는 클래스.
    

public:
    Client(/* args */) : socket_fd(-1) {};
    ~Client()
    {
        if (this->socket_fd != -1)
            close(this->socket_fd);
    }
    int getSocket_fd()
    {
        return this.socket_fd;
    }
    void setSocket_fd(int socket_fd)
    {
        this.socket_fd = socket_fd;
    }
    Request getRequest()
    {
        return this.request;
    }
    void setRequest(Request request)
    {
        this.request = request;
    }
    Response getResponse()
    {
        return this.response;
    }
    void setResponse(Response response)
    {
        this.response = response;
    }

    //감지목록에 클라이언트의 kevent를 초기화해서 추가하는 메소드. 
    void regist_fd_to_detects( void )
    {
        EV_SET(g_detects, this->getSocket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        EV_SET(g_detects, this->getSocket_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        
        //**struct kevent k_set;
        /**
        EV_SET(이벤트구조체 &k_set,
            감지할fd,
            감지되면 설정될 플래그 EVFILT_READ 또는 EVFILT_WRITE,
            "이벤트추가" 및 "이벤트반환"매크로 EV_ADD | EV_ENABLE,
            필터플레그값 0,
            필터 데이터값 0,
            사용자 정의 데이터 NULL);
        **/
    }

    //클라이언트 소켓에서 데이터를 읽어서 본인의 data버퍼에 저장하는 메소드. 실패 -1 성공 0 반환.
    int recv_data( void )
    {
        unsigned int read_size;
        char    buffer[BUFFER_SIZE];

        read_size = read(new_socket, buffer, BUFFER_SIZE);
        if (read_size <= 0) //kq에서 읽기가능이라고 했는데도 데이터를 읽을 수 없다면 삭제한다.
        {
            return (-1);
        }
        else //읽은 데이터 char[] -> string으로 변환해서 저장.
        {
            this->data += std::string(buffer, read_size);
        }
        return 0;
    }
};
