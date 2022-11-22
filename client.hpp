#ifndef CLIENT_CLASS_HPP
# define CLIENT_CLASS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "location.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <sstream>

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
    Server * my_server; //현재 클라이언트의 서버.(conf 데이터 불러오기 가능).
    std::map<std::string, std::string> * status_msg;

public:
    Client(std::vector<struct kevent> * cmds) : socket_fd(-1), file_fd(-1), cgi_mode(false), _ev_cmds(cmds) {};
    ~Client()
    {
        if (this->socket_fd != -1)
            close(this->socket_fd);
    }
    Server * get_myserver()
    {
        return this->my_server;
    }
    void set_status_msg(std::map<std::string, std::string> * status_map)
    {
        this->status_msg = status_map;
    }
    void set_myserver(Server * server)
    {
        this->my_server = server;
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
        //5. kq에 소켓을 "쓰기가능"감지로 등록.
        return true; //문제없이 응답클래스를 초기화했으면 true반환
    }
    void find_mime_type(std::string path)
    {
        //값"text/html",text/css, images/png, jpeg, gif
        //헤더파일형식 Content-Type: text/html;
        std::string temp = util::ft_split(util::ft_split(path, "/").back(), ".").back();//파일 확장자만 반환하기

        if (temp == "css")
            this->response.setHeader_map("Content-Type", "text/css");
        else if (temp == "png")
            this->response.setHeader_map("Content-Type", "images/png");
        else if (temp == "jpeg")
            this->response.setHeader_map("Content-Type", "images/jpeg");
        else if (temp == "gif")
            this->response.setHeader_map("Content-Type", "images/gif");
        else
            this->response.setHeader_map("Content-Type", "text/html");
    }
    //에러 응답데이터를 만들기전에 필요한 준비를 지시하는 메소드.
    bool ready_err_response_meta()
    {
        Server s = *this->my_server;
        //std::string temp_body1 = "<html>/r/n<head>/r/n  <title> error page </title>/r/n  </head>/r/n  <body bgcolor='white'>/r/n    <center>/r/n      <h1>";
        //std::string temp_body2 = this->response.getStatus() + this->response.getStatus_msg();
        //std::string temp_body3 = "</h1>/r/n    </center>/r/n    <hr>/r/n    <center>soo-je-webserver</center>/r/n  </body>/r/n</html>";
        //if conf에 지정된 에러페이지가없으면.
        //  1.  바디를 하드코딩으로 만든다.       O
        //  2. kq에 소켓을 "쓰기가능"감지로 등록.  O
        //else
        //  1. stat으로 지정된 에러페이지(설정되어있다면)가 정규파일이면 바로 open, 에러시 500처리.   O
        //  2. 설정된 파일이 있고, 열리면 논블로킹 설정하고, 현 클라객체 file fd에 등록.      서버 88줄      o
        //  3. 열린파일fd를 "읽기 가능"감지에 등록.  288에서 read 로 변경하기                          O
//경로에서 확장자를 확인하고 해당하는 헤더를 반환하는
	    std::stringstream ss(this->response.getStatus());
        int status_num;
        ss >> status_num;
        if (s.get_default_error_page().count(status_num) == 0)
        {
            this->response.setVersion(this->request.getVersion());
            this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);
            //헤더도 넣기
            this->response.setHeader_map("server", "soo-je-webserver");
            this->response.setHeader_map("Date", util::get_date());
            this->response.setHeader_map("content-Type", "text/html");
            this->response.setHeader_map("Content-Length", "4");
            this->response.setHeader_map("Connection", "keep-alive");
            this->response.setHeader_map("Accept-Ranges", "bytes");
            this->response.setBody(this->response.getStatus());
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            add_kq_event(this->file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //파일을 쓰기감지에 예약.
        }
        else
        {
            struct stat sb;
            if (stat(s.get_default_error_page().find(status_num)->second.c_str(), &sb) != 0);
                return (this->response.setStatus("500"), this->ready_err_response_meta());
            if ((S_IFMT & file_info.sb.st_mode) != S_IFREG)//일반파일이 아닐 경우
                return (this->response.setStatus("500"), this->ready_err_response_meta());
            if ((this->file_fd = open(s.get_default_error_page().find(status_num)->second, O_RDONLY)) < 0)
                return (this->response.setStatus("500"), this->ready_err_response_meta());//터지면 경로 문제
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.

            this->response.setHeader_map("server", "soo-je-webserver");
            this->response.setHeader_map("Date", util::get_date());
            this->response.setHeader_map("content-Type", "text/html");
            this->response.setHeader_map("Content-Length", (sb.st_size).to_string());//논의 필요
            this->response.setHeader_map("Connection", "keep-alive");
            this->response.setHeader_map("Accept-Ranges", "bytes");

            add_kq_event(this->file_fd, EVFILT_READ, EV_ADD | EV_ENABLE); //파일을 쓰기감지에 예약.
        }
        return true; //정상수행 true반환.
    }

    //응답데이터를 만들기전에 필요한 read/write 또는 unlink하는 메소드.
    bool ready_response_meta()
    {
        Server s = *this->my_server;
        std::string uri = this->getRequest().getTarget().substr(0, this->getRequest().getTarget().find('?')); //'?'부터 쿼리스트링 제거

        if (this->request.getMethod() == "GET" || this->request.getMethod() == "POST")
        {
            if (uri[uri.length() - 1] != '/') //경로가 '/'로 끝나지 않으면 만들어준다.
			    uri += '/';
            std::string path = s.get_root();;
            if (this->request.getTarget() != "/")
            {
                //uri와 loc에 매칭되는 경로가 있다면 path는 loc의 root + 매치되지 않은 부분으로 재할당.
            }
            struct stat sb;
            if (stat(path.c_str(), &sb) == -1) //경로가 존재 하지 않을 때.
            {
                path.erase(--(path.end())); // '//'더블 슬레시 제거.
                if (stat(path.c_str(), &sb) == -1) // 그래도 없을 때.
                {
                    this->getResponse().setStatus("404"); //404처리.
                    return false; //바로 에러 페이지 제작 필요.
                }
            }
            //오토인덱스 확인 및 처리(미구현)...
            this->file_fd = open(path.c_str(),O_RDONLY);
            if (this->file_fd == -1) //열기 실패시.
            {
                this->getResponse().setStatus("500"); //500처리.
                return false; //바로 에러 페이지 제작 필요.
            }
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            add_kq_event(this->file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //파일을 쓰기감지에 예약.
        }
        else if (this->request.getMethod() == "DELETE")
        {
            if (uri[uri.length() - 1] != '/') //경로가 '/'로 끝나지 않으면 만들어준다.
			    uri += '/';
            //만약 uri가 location 목록에 있다면 경로를 재설정한다(미구현)....
        }
        else if (this->request.getMethod() == "PUT")
        {
            //미구현...
        }
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
    bool check_need_cgi()
    {
        Server s = *this->my_server;
        //taget의 확장자가 server의 map에 있는지 검사...
        std::map<std::string, std::string> & cgi_infos = s.get_cgi_map();

        size_t exe = this->getRequest().getTarget().find('.'); //확장자를 암시하는 부분을 찾는다.
        if (exe == std::string::npos) //없다면 검사종료.
            return (false);
        size_t curr = exe;
        while (curr != this->getRequest().getTarget().length()) //확장자의 문자열을 하나하나검사.
            if (this->getRequest().getTarget()[curr] != '/' && this->getRequest().getTarget()[curr] != '?') //문자열에 '/'또는'?'가 있다면 검사중단.
                curr++;
        std::string pure_exe = this->getRequest().getTarget().substr(exe, curr - exe); //순수 확장자만 파싱해서 뽑는다.
        std::map<std::string, std::string>::const_iterator match_cgi = cgi_infos.find(pure_exe); //지원하는 cgi가 있는지 검사.
        if (match_cgi == cgi_infos.end()) //지원하는 cgi가 없다면 false반환
            return (false);
        while (this->getRequest().getTarget()[exe] != '/') //////////////////
            exe--;
        pure_exe = this->getRequest().getTarget().substr(exe + 1, curr - exe - 1);
        //stat으로 cgi파일이 존재하는지 검사.....(미구현)
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
        this->read_buf.clear();
        this->write_buf.clear();
        // request
        // response
        this->file_fd = -1;
        this->file_buf.clear();
        this->write_size = 0;
        this->read_size = 0;
        this->server_fd = -1;
        this->cgi_mode = false; // ?
        this->_ev_cmds->clear();
        this->my_server = NULL;

        return true; //문제없으면 true리턴.
    }

    //400번대 에러가 발생했는지 검사하는 메소드.
    bool check_client_err()
    {
        // path 정의해줘야 함. (동작 확인 필요)
        std::string uri = this->getRequest().getTarget().substr(0, this->getRequest().getTarget().find('?'));
        std::string root = this->get_myserver()->get_root() + "/";
        std::string path;
        // location block에서 일치하는 항목 있는지 확인
        int loc_idx = -1;
        for(int idx = 0; idx < this->get_myserver()->loc.size(); idx++)
        {
            if (uri.find(this->get_myserver()->loc[idx].path) == 0)
            {
                if (loc_idx == -1 || this->get_myserver()->loc[idx].path > this->get_myserver()->loc[loc_idx].path)
                    loc_idx = idx;
            }
        }
        if (loc_idx != -1)
            path = uri.replace(0, this->get_myserver()->loc[loc_idx].path.length(), this->get_myserver()->loc[loc_idx].root + "/");
        else
            path = root + uri;
        size_t pos = path.find("//");
        while (1)
        {
            if (pos == std::string::npos)
                break;
            path.replace(pos, 2, "/");
            pos = path.find("//");
        }
        //권한오류, 메소드사용가능한지 유무, 최대 바디크기 유무 등등 검사....
        //400번대에 해당하는 오류있으면 response의 상태코드를 설정.
        // - 400 : request parse 에서 처리
        // - 401 : ? 인증 여부 확인
        // - 404 : access(path, F_OK), ready_response_meta 에서 처리
        if (access(path.c_str(), F_OK) == -1)
            return (this->getResponse().setStatus("404"), true);
        // - 403 : access(path, R_OK) (읽기권한)
        if (access(path.c_str(), R_OK) == -1)
            return (this->getResponse().setStatus("403"), true);
        // - 405 : accept_method 확인, request에서 확인 완.
        if (loc_idx != -1)
        {
           if (find(this->get_myserver()->loc[loc_idx].accept_method.begin(), \
            this->get_myserver()->loc[loc_idx].accept_method.end(), \
            this->getRequest().getMethod()) == this->get_myserver()->loc[loc_idx].accept_method.end())
            return (this->getResponse().setStatus("405"), true);
        }
        // - 408 : Timeout ?
        // - 410 : -> 404 ?
        // - 411 : Content-Length 필요한 경우 확인
        if (this->getRequest().getMethod() == "POST")
        {
            if (this->getRequest().getHeaders().find("Content-Length") == this->getRequest().getHeaders().end())
                return (this->getResponse().setStatus("411"), true);
        }
        // - 413 : client_max_body_size 확인
        if (this->getRequest().getBody().length() > this->get_myserver()->client_max_body_size)
            return (this->getResponse().setStatus("413"), true);
        // - 414 : URI 길이 ?
        return false; //이상 없으면 false반환.
    }

    void excute_cgi()
    {
        std::string file_name = "cgi_result_" + util::num_to_string(this->socket_fd);
        //1. cgi결과를 담을 result file open. (이름은 중복되지 않도록 뒤에 fd번호를 붙인다).
        //2. 클라의 file fd를 result file fd로 설정.
        this->file_fd = open(file_name.c_str(), O_WRONLY);
        if (this->file_fd == -1)
        {
            // error
        }
        //3. fork
        pid_t pid = fork();
        if (pid == 0)
        {
            //4. 자식프로세스의 stdout을 file fd로 변경(dup2). 그 후 execve로 실행. 마지막에 exit(0);
            int ret = dup2(this->file_fd, 1);
            if (ret == -1)
            {
                // error
            }
        }
        else
        {
            //5. 부모프로세스에서 file fd를 논블로킹으로 설정. kq에 "읽기가능"감지로 등록. return ;
        }
    }
};

#endif