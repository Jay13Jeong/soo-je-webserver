#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/event.h>
#include "location.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFER_SIZE 5120
// #define 

class Client
{
private:
    int socket_fd; //클라이언트 소켓 fd.
    std::string data; //소켓에서 읽어온 비정제 데이터. (추후 파싱필요)
    Request request; //비정제데이터를 파싱해서 만든 request클래스.
    Response response; //response를 제작하는 클래스.
    int file_fd; // cgi가 출력한 결과물을 담는 파일의 fd.
    std::string file_data; //파일의 정보가 저장되는 변수.

public
    std::string getFile_data()
    {
        return this.file_data;
    }

public
    void setFile_data(std::string file_data)
    {
        this.file_data = file_data;
    }

public
    int getFile_fd()
    {
        return this.file_fd;
    }

public
    void setFile_fd(int file_fd)
    {
        this.file_fd = file_fd;
    }

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

    //클라이언트 소켓에서 데이터를 읽어서 본인의 data버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int recv_data( void )
    {
        unsigned int read_size;
        char    buffer[BUFFER_SIZE];

        read_size = recv(this->getSocket_fd(), buffer, BUFFER_SIZE, 0);
        if (read_size >= 1)
        {
            //1.읽은 데이터 char[] -> string으로 변환해서 저장.
            this->data += std::string(buffer, read_size);
            //**클라객체에서 추가적으로 수신 완료여부 검사 필요.

            //2.전부 송신이 되었는지 (또는 멤버bool변수로 체크).
            if (read_size < BUFFER_SIZE)
            {
                //3.송신된 데이터 파싱.
                return 1;
            }
            //4. 파싱이 끝났는지. 
            //5.파싱된 요청클래스로 응답클래스 제작 (cgi 또는 파일업로드 필요시 중간에 실행) (완료되면 준비됨으로 바꿈).
        }
        else
        {
            return -1;
        }
        return 0;
    }

    //지정한 파일에서 데이터를 읽어서 본인의 file_data버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int read_file( void )
    {
        unsigned int read_size;
        char    buffer[BUFFER_SIZE];

        read_size = recv(this->getFile_fd(), buffer, BUFFER_SIZE, 0);
        if (read_size >= 1)
        {
            this->file_data += std::string(buffer, read_size);
            //**클라객체에서 추가적으로 수신 완료여부 검사 필요.

            if (read_size < BUFFER_SIZE)
                return 1;
        }
        else
        {
            return -1;
        }
        return 0;
    }

    //응답데이터를 소켓에게 전송하는 메소드.
    int send_data()
    {
        int send_size;
        
        this->response
    }

    //파싱이 완료된 요청클래스로 
    bool init_response()
    {
        this->response.setVersion(this->request.getVersion);
        //**이런식으로 응답클래스를 초기화한다.

        return true; //문제없이 응답클래스를 초기화했으면 true반환 
    }

    //cgi실행이 필요한지 여부를 반환하는 메소드. 
    bool check_need_cgi()
    {
        /*code*/

        return false;
    }

    //비정제 data를 파싱해서 맴버변수"request"를 채우는 메소드. 
    bool parse_request()
    {

        return true; //문제없이 파싱이 끝나면 true반환.
    }
};
