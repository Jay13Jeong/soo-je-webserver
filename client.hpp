#ifndef CLIENT_CLASS_HPP
# define CLIENT_CLASS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/event.h>
#include "location.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFER_SIZE 5120

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
    size_t read_size; //파일의 읽은 데이터 크기.
    int server_fd; //파생해준 서버fd (conf정보 찾을 때 필요).
    bool cgi_mode; // cgi모드여부.
    std::vector<struct kevent> * _ev_cmds; //kq 감지대상 벡터.

public:
    Client(std::vector<struct kevent> * cmds) : socket_fd(-1), file_fd(-1), cgi_mode(false), _ev_cmds(cmds) {};
    ~Client()
    {
        if (this->socket_fd != -1)
            close(this->socket_fd);
    }

    bool isCgi_mode()
    {
        return this->cgi_mode;
    }
    void setCgi_mode(bool cgi_mode)
    {
        this->cgi_mode = cgi_mode;
    }
    std::string getFile_buf()
    {
        return this->file_buf;
    }
    void setFile_buf(std::string file_buf)
    {
        this->file_buf = file_buf;
    }
    int getFile_fd()
    {
        return this->file_fd;
    }
    void setFile_fd(int file_fd)
    {
        this->file_fd = file_fd;
    }
    int getSocket_fd()
    {
        return this->socket_fd;
    }
    void setSocket_fd(int socket_fd)
    {
        this->socket_fd = socket_fd;
    }
    Request getRequest()
    {
        return this->request;
    }
    void setRequest(Request request)
    {
        this->request = request;
    }
    Response getResponse()
    {
        return this->response;
    }
    void setResponse(Response response)
    {
        this->response = response;
    }

    //fd와 "감지할 행동", kevent지시문을 인자로 받아서 감지목록에 추가하는 메소드.
    void add_kq_event(uintptr_t ident, int16_t filter, uint16_t flags)
    {
        struct kevent new_event;

        EV_SET(&new_event, ident, filter, flags, 0, 0, NULL);
        this->_ev_cmds->push_back(new_event);
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

        read_size = read(this->getFile_fd(), buffer, BUFFER_SIZE);
        if (read_size == -1)
        {
            close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
            this->file_fd = -1;
            return -1;
        }
        else
        {
            this->file_buf += std::string(buffer, read_size);
            //**추가적으로 수신 완료여부 검사 필요할지도.

            if (read_size < BUFFER_SIZE)
            {
                close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
                this->file_fd = -1;
                return 1;
            }
        }
        return 0;
    }

    //지정한 파일에 file_buf를 write하는 메소드. 실패 -1 성공 0 모두보냄 1 반환.
    int write_file( void )
    {
        size_t size;

        size = write(this->getFile_fd(), file_buf.c_str() + (this->write_size), file_buf.length() - (this->write_size));
        if (size == -1)
        {
            close(this->file_fd);
            this->file_fd = -1;
            this->write_size = 0;
            return -1;
        }
        this->write_size += size;
        if (this->write_size >= this->file_buf.length())
        {
            close(this->file_fd);
            this->file_fd = -1;
            this->write_size = 0;
            return 1;
        }
        return 0;
    }

    //응답데이터를 소켓에게 전송하는 메소드.
    int send_data()
    {
        size_t send_size;

        send_size = send(this->socket_fd, this->write_buf.c_str() + (this->write_size), this->write_buf.length() - (this->write_size), 0);
        if (send_size == -1) //데이터전송 실패 했을 때.
            return -1; //호출한 부분에서 이 클라이언트 제거.
        
        this->write_size += send_size;
        if (this->write_size >= this->write_buf.length())
        {
            //**추가적으로 송신 완료여부 검사 필요할지도.
            return 1; //호출한 부분에서 클라이언트 객체를 초기화하는 함수 실행.
        }
        //전송중이면 (다 못보냈을 때)
        return 0;
    }

    //응답클래스를 제작하는 메소드.
    bool init_response(std::map<int,Server> & server_map)
    {
        Server s = server_map[this->server_fd];
        this->response.setVersion(this->request.getVersion());
        //1. 위와 같이 응답클래스를 초기화한다.
        //2. file_buf의 크기를 헤더필드 Content-Length에 추가한다. 그외에 필요한 정보가 있으면 추가.
        //3. (cgi냐 단순html파일이냐 에따라 다르게 file_buf 컨트롤 필요).
        //4. 맴버변수 write_buf에 하나의 데이터로 저장한다. (시작줄 + 헤더 + file_buf).
        return true; //문제없이 응답클래스를 초기화했으면 true반환 
    }

    //에러 응답데이터를 만들기전에 필요한 준비를 지시하는 메소드.
    bool ready_err_response_meta(std::map<int,Server> & server_map)
    {
        Server s = server_map[this->server_fd];

        //if conf에 지정된 에러페이지가없으면.
        //  1.  바디를 하드코딩으로 만든다.
        //  2. kq에 소켓을 "쓰기가능"감지로 등록.
        //else
        //  1. stat으로 지정된 에러페이지(설정되어있다면)가 정규파일이면 바로 open, 에러시 500처리.
        //  2. 설정된 파일이 있고, 열리면 논블로킹 설정하고, 현 클라객체 file fd에 등록.
        //  3. 열린파일fd를 "읽기 가능"감지에 등록.

        return true; //정상수행 true반환.
    }

    //응답데이터를 만들기전에 필요한 read/write 또는 unlink하는 메소드.
    bool ready_response_meta(std::map<int,Server> & server_map)
    {
        Server s = server_map[this->server_fd];
        //if GET or POST (read)
        //  0. (아래 진행하다가 문제 생기면 400, 404 등등 처리) --나중에 여기서 세션처리.
        //  1. stat으로 target_path가 정규파일이면 바로 open, 에러시 500처리.
        //  2. 폴더면 conf의 index로 open. 없으면 autoindex유무에 따라 처리. 없으면 400에러
        //  3. 열린파일fd 논블로킹 설정하고, 현 클라객체 file fd에 등록.
        //  4. 열린파일fd를 "읽기 가능"감지에 등록.
        //if DELETE (no body)
        //  1.폴더면 rmdir, 정규파일이면 unlink
        //  2.바디가 없는 응답클래스를 제작.
        //  3.kq에 소켓을 "쓰기가능"감지로 등록.
        //if PUT (write)
        //  1. stat으로 target_path가 정규파일이면 바로 open, 에러시 500처리.
        //  2. 폴더면 conf의 index로 open.
        //  3. 열린파일fd 논블로킹 설정하고, 현 클라객체 file fd에 등록.
        //  4. 열린파일fd를 "쓰기 가능"감지에 등록.
        //else 지원되는 메소드가 아니면 501

        return true; //정상수행 true반환.
    }

    //cgi실행이 필요한지 여부를 반환하는 메소드. 
    bool check_need_cgi(std::map<int,Server> & server_map)
    {
        Server s = server_map[this->server_fd];
        //taget의 확장자가 server의 map에 있는지 검사...

        return false; //cgi가 필요없으면 false반환.
    }

    //비정제 data를 파싱해서 맴버변수"request"를 채우는 메소드. 
    bool parse_request()
    {
        if ((this->request.parse(this->read_buf, this->response.getStatus())) == false) //read_buf 파싱.
            return false;
        return true; //문제없이 파싱이 끝나면 true반환.
    }

    //소켓fd만 제외하고 모두 깡통으로만드는 메소드.
    bool clear_client()
    {
        //fd빼고 모두 초기상태로 초기화한다...

        return true; //문제없으면 true리턴.
    }

    //400번대 에러가 발생했는지 검사하는 메소드.
    bool check_client_err()
    {
        //권한오류, 메소드사용가능하지 유무, 최대 바디크기 유무 등등 검사....
        //400번대에 해당하는 오류있으면 response의 상태코드를 설정.

        return false; //이상 없으면 false반환.
    }

    void excute_cgi()
    {
        //1. cgi결과를 담을 result file open. (이름은 중복되지 안도록 뒤에 fd번호를 붙인다).
        //2. 클라의 file fd를 result file fd로 설정. 
        //3. fork
        //4. 자식프로세스의 stdout을 file fd로 변경(dup2). 그 후 execve로 실행. 마지막에 exit(0);
        //5. 부모프로세스에서 file fd를 논블로킹으로 설정. kq에 "읽기가능"감지로 등록. return ;
    }
};

#endif