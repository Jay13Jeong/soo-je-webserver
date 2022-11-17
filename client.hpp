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
    std::string read_buf; //소켓에서 읽어온 비정제 데이터. (추후 파싱필요)
    std::string write_buf; //응답 클래스로 보낼 데이터.
    Request request; //비정제데이터를 파싱해서 만든 request클래스.
    Response response; //response를 제작하는 클래스.
    int file_fd; // cgi가 출력한 결과물을 담는 파일의 fd.
    std::string file_buf; //파일의 정보가 저장되는 변수.
    size_t write_size; //보낸 데이터 크기.

public
    std::string getFile_buf()
    {
        return this.file_buf;
    }

public
    void setFile_buf(std::string file_buf)
    {
        this.file_buf = file_buf;
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

    //클라이언트 소켓에서 데이터를 읽어서 본인의 read_buf버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int recv_data( void )
    {
        size_t read_size;
        char    buffer[BUFFER_SIZE];

        read_size = recv(this->getSocket_fd(), buffer, BUFFER_SIZE, 0);
        if (read_size == -1 || read_size == 0)
        {
            return -1;
        }
        else
        {
            //1.읽은 데이터 char[] -> string으로 변환해서 저장.
            this->read_buf += std::string(buffer, read_size);
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
        return 0;
    }

    //지정한 파일에서 데이터를 읽어서 본인의 file_buf버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int read_file( void )
    {
        size_t read_size;
        char    buffer[BUFFER_SIZE];

        read_size = recv(this->getFile_fd(), buffer, BUFFER_SIZE, 0);
        if (read_size == -1 || read_size == 0)
        {
            return -1;
        }
        else
        {
            this->file_buf += std::string(buffer, read_size);
            //**추가적으로 수신 완료여부 검사 필요할지도.

            if (read_size < BUFFER_SIZE)
                return 1;
        }
        return 0;
    }

    //응답데이터를 소켓에게 전송하는 메소드.
    int send_data()
    {
        size_t send_size;

        send_size = send(this->socket_fd, this->write_buf.c_str() + (this->write_size), this->write_buf.length(), 0);
        if (send_size == -1) //데이터전송 실패 했을 때.
            return -1; //호출한 부분에서 이 클라이언트 소켓 닫기.
        
        this->write_size += send_size;
        if (write_size >= write_buf.length())
        {
            //**추가적으로 송신 완료여부 검사 필요할지도.
            return 1; //호출한 부분에서 클라이언트 객체를 fd만 빼고 pop했다가 새로운 클라이언트 객체에 fd넣어서 클라이언트 백터에 넣기.
        }
        //전송중이면 (다 못보냈을 때)
        return 0;
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

    //소켓fd만 제외하고 모두 깡통으로만드는 메소드.
    bool clear_client()
    {

        return 1; //
    }
};

///cgi 처리후 결과파일을 읽어서 응답클래스를 만든다.
