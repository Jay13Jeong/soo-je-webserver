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
#include <dirent.h>
#include "util.hpp"
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
    int write_size; //보낸 데이터 크기.
    int read_size; //파일의 읽은 데이터 크기.
    Location * my_loc; //요청이 처리될 영역을 지정한 로케이션 구조체.
    int server_fd; //파생해준 서버fd (conf정보 찾을 때 필요).
    bool cgi_mode; // cgi모드여부.
    std::vector<struct kevent> * _ev_cmds; //kq 감지대상 벡터.
    Server * my_server; //현재 클라이언트의 서버.(conf 데이터 불러오기 가능).
    std::map<std::string, std::string> * status_msg;
    std::string  cgi_program; //cgi를 실행할 프로그램 경로 (예시 "/usr/bin/python")
    std::string  cgi_file; //cgi를 실행할 파일 경로 (예시 "hello.py")

public:
    Client(std::vector<struct kevent> * cmds) : socket_fd(-1), file_fd(-1), cgi_mode(false), _ev_cmds(cmds) \
    ,read_buf(""), write_buf(""), file_buf(""), write_size(0), read_size(0), my_loc(NULL), server_fd(-1) \
    , my_server(NULL), status_msg(NULL), cgi_program(""), cgi_file("") {};
    ~Client()
    {
        perror("close Client!");
        // if (this->socket_fd != -1)
        //     close(this->socket_fd);
    }
    // void get_client_status()
    // {
    //     std::cerr << this->socket_fd << " //클라이언트 소켓 fd." << std::endl;
    //     std::cerr << this->read_buf<< "//소켓에서 읽어온 비정제 데이터. (추후 파싱필요)" <<std::endl;
    //     std::cerr << this->write_buf; //응답 클래스로 보낼 데이터.
    //     std::cerr << this->file_fd; // cgi가 출력한 결과물을 담는 파일의 fd.
    //     std::cerr << this->file_buf; //파일의 정보가 저장되는 변수.
    //     std::cerr << this->write_size; //보낸 데이터 크기.
    //     std::cerr << this->read_size; //파일의 읽은 데이터 크기.
    //     std::cerr << this->server_fd; //파생해준 서버fd (conf정보 찾을 때 필요).
    //     std::cerr << this->cgi_mode; // cgi모드여부.
    //     std::cerr << this->cgi_program; //cgi를 실행할 프로그램 경로 (예시 "/usr/bin/python")
    //     std::cerr << this->cgi_file; //cgi를 실행할 파일 경로 (예시 "hello.py")
    // }
    std::string get_read_buf()
    {
        return this->read_buf;
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
    int & getFile_fd()
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
    Request & getRequest()
    {
        return this->request;
    }
    void setRequest(Request request)
    {
        this->request = request;
    }
    Response & getResponse()
    {
        return this->response;
    }
    void setResponse(Response response)
    {
        this->response = response;
    }
    std::string &get_cgi_program()
    {
        return this->cgi_program;
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
        int size;
        char    buffer[BUFFER_SIZE];

        // if ((this->read_buf.size() + BUFFER_SIZE) < this->read_buf.size()) //string 용량 초과시 예외처리.
        //     return -1;
        size = recv(this->socket_fd, buffer, BUFFER_SIZE, 0);
        if (size == -1)
        {
            perror("recv client err");
            return -1;
        }
        if (size == 0)
            return -1;
        else
        {
            this->read_buf += std::string(buffer, size); //1.읽은 데이터 char[] -> string으로 변환해서 저장.
            std::cerr << this->read_buf << " : buff" << std::endl;
            std::cerr << this->socket_fd << " : sock" << std::endl;
            if (size < BUFFER_SIZE) //모두 읽었다면..
                return 1;
        }
        return 0;
    }

    //지정한 파일에서 데이터를 읽어서 본인의 file_buf버퍼에 저장하는 메소드. 실패 -1 성공 0 모두받음 1 반환.
    int read_file( void )
    {
        int size;
        char    buffer[BUFFER_SIZE];

        // if ((this->file_buf.size() + BUFFER_SIZE) < this->file_buf.size()) //string 용량 초과시 예외처리.
        // {
        //     close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
        //     this->file_fd = -1;
        //     return -1;
        // }
        size = read(this->file_fd, buffer, BUFFER_SIZE);
        if (size == -1)
        {
            close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
            this->file_fd = -1;
            perror("read file fail...");
            return -1;
        }
        if (size == 0)
        {
            close(this->file_fd); //파일을 닫는다. (자동으로 감지목록에서 사라짐).
            this->file_fd = -1;
            return -1;
        }
        else
        {
            this->file_buf += std::string(buffer, size);
            if (size < BUFFER_SIZE)
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
        int size;

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
        int size;

        size = send(this->socket_fd, this->write_buf.c_str() + (this->write_size), this->write_buf.length() - (this->write_size), 0);
        if (size == -1) //데이터전송 실패 했을 때.
            return -1; //호출한 부분에서 이 클라이언트 제거.

        this->write_size += size;
        if (this->write_size >= this->write_buf.length())
        {
            //**추가적으로 송신 완료여부 검사 필요할지도.
            return 1; //호출한 부분에서 클라이언트 객체를 초기화하는 함수 실행.
        }
        //전송중이면 (다 못보냈을 때)
        return 0;
    }

    //응답클래스를 제작하는 메소드.
    bool init_response()
    {
        Server & s = *this->my_server;
        this->response.setVersion(this->request.getVersion());
        this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);

        if (this->response.getHeader_map().find("server") == this->response.getHeader_map().end())
            this->response.setHeader_map("server", "soo-je-webserver");
        if (this->response.getHeader_map().find("Date") == this->response.getHeader_map().end())
            this->response.setHeader_map("Date", util::get_date());
        if (this->response.getHeader_map().find("Connection") == this->response.getHeader_map().end())
            this->response.setHeader_map("Connection", "keep-alive");
        if (this->response.getHeader_map().find("Accept-Ranges") == this->response.getHeader_map().end())
            this->response.setHeader_map("Accept-Ranges", "bytes");


        //1. 위와 같이 응답클래스를 초기화한다.
        //2. file_buf의 크기를 헤더필드 Content-Length에 추가한다. 그외에 필요한 정보가 있으면 추가.
        //3. (cgi냐 단순html파일이냐 에따라 다르게 file_buf 컨트롤 필요).
        //4. 맴버변수 write_buf에 하나의 데이터로 저장한다. (시작줄 + 헤더 + file_buf).
        //5. kq에 소켓을 "쓰기가능"감지로 등록.

        //2번내용
        if (this->response.getHeader_map().find("Content-Length") != this->response.getHeader_map().end())
            this->response.getHeader_map().erase("Content-Length");
        this->response.setHeader_map("Content-Length", util::num_to_string(this->file_buf.size()));
        //1번 내용, cgi 경우로 인해 못 한 것 추가
        if (this->response.getHeader_map().find("content-Type") == this->response.getHeader_map().end())
            this->find_mime_type(this->request.getTarget());
        this->response.setBody(this->file_buf);

        //4번.
        std::cerr << "----init_response()->push_write_bud()" << std::endl;
        this->push_write_buf(this->file_buf);

        // add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓을을 쓰기감지에 예약.
        return true; //문제없이 응답클래스를 초기화했으면 true반환
    }

    void push_write_buf(std::string response_body)
    {
        //스타트라인
        this->write_buf = this->response.getVersion() + " " + this->response.getStatus() + " " + this->response.getStatus_msg() + "\r\n";
        //헤더 부분
        std::map<std::string, std::string> temp = this->request.getHeaders();
    	std::map<std::string,std::string>::iterator iter;
	    for(iter = temp.begin() ; iter != temp.end(); iter++)
		    this->write_buf = this->write_buf + iter->first + ": " + iter->second + "\r\n";
        //개행추가 부분, cgi의 경우 바디 윗부분에 개행이 추가되어있다.바디에 개행이 추가되는 것을 방지.
        if (this->cgi_mode == false)
            this->write_buf = this->write_buf + "\r\n";
        //바디 부분
        if (response_body.size() != 0)
            this->write_buf = this->write_buf + response_body;

        std::cerr << "----push_write_bud()에서 실행, write_buf 출력--------------------" << std::endl;
        std::cerr << write_buf << std::endl;
        std::cerr << "--------------------------------------------------------------" << std::endl;
    }

    //오토인데스 응답페이지를 만들고 송신준비를 하는 메소드.
    void init_autoindex_response(std::string path)
    {
        std::string temp_body;
        //1.바로 소켓송신이 가능하도록 헤더+바디를 제작한다. -> "this->write_buf에 바로 담는다."
        //2.kq에 "쓰기가능"감지 등록한다.

        this->response.setVersion(this->request.getVersion());
        this->response.setStatus("200");
        this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);
        //헤더도 넣기
        this->response.setHeader_map("server", "soo-je-webserver");
        this->response.setHeader_map("Date", util::get_date());
        this->response.setHeader_map("content-Type", "text/html");
        this->response.setHeader_map("Connection", "keep-alive");
        this->response.setHeader_map("Accept-Ranges", "bytes");

        DIR *dir;
        struct dirent *ent;
        dir = opendir((my_loc->root + this->request.getTarget()).c_str());//해당 경로의 파일및 디렉터리 를 받기 위한 오픈
        if (dir == NULL)
        {
            this->response.setStatus("500");//서버 에러
            this->ready_err_response_meta();//기본 세팅 보내기
            return ;
        }
        //오토인덱스 html 초기세팅
        temp_body = "<html><head>    <title>Index of ";
        temp_body = temp_body + this->request.getTarget() + "</title></head><body bg color='white'>  <h1> Index of " + this->request.getTarget() + "</h1>  <hr>  <pre>";

        while ((ent = readdir(dir)) != NULL)//경로의 파일들 바디에 넣기
        {
            // .  .. 이건 빼자, 숨긴 파일은 출력을 해보자
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue ;
            temp_body = temp_body + "    <a href= " + ent->d_name + ">" + ent->d_name + "</a><br>";
        }
        temp_body = temp_body + "</pre>  <hr></body></html>";
        this->response.setHeader_map("Content-Length", util::num_to_string(this->response.getBody().length()));//바디 크기
        this->response.setBody(temp_body);//바디 입력
        closedir(dir);
        std::cerr << "----init_autoindex_response()->push_write_bud()에서 실행--------------------" << std::endl;
        push_write_buf(this->response.getBody());
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
        //if conf에 지정된 에러페이지가없으면.
        //  1.  바디를 하드코딩으로 만든다. -> "this->write_buf 제작"
        //  2. kq에 소켓을 "쓰기가능"감지로 등록.
        //else
        //  1. stat으로 지정된 에러페이지(설정되어있다면)가 정규파일이면 바로 open, 에러시 500처리.
        //  2. 설정된 파일이 있고, 열리면 논블로킹 설정하고, 현 클라객체 file fd에 등록.
        //  3. 열린파일fd를 "읽기 가능"감지에 등록.  288에서 read 로 변경하기

        Server & s = *this->my_server;
        //경로에서 확장자를 확인하고 해당하는 헤더를 반환하는
        if (s.get_default_error_page().count(this->response.getStatus()) == 0)
        {
            perror("no default err page");
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
             std::cerr << "----ready_err_response_meta()->if()->push_write_bud()" << std::endl;
            push_write_buf(this->response.getBody());
            // add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); 
        }
        else
        {
            perror("default err page");
            struct stat sb;
            if (stat(s.get_default_error_page().find(this->response.getStatus())->second.c_str(), &sb) != 0)//루트경로 추가할 것
                return (this->response.setStatus("500"), this->ready_err_response_meta());
            if ((S_IFMT & sb.st_mode) != S_IFREG)//일반파일이 아닐 경우
                return (this->response.setStatus("500"), this->ready_err_response_meta());
            if ((this->file_fd = open(s.get_default_error_page().find(this->response.getStatus())->second.c_str(), O_RDONLY)) < 0)
                return (this->response.setStatus("500"), this->ready_err_response_meta());//터지면 경로 문제
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.

            //헤더 내용은 뒤에 다른 함수에서 추가

            add_kq_event(this->file_fd, EVFILT_READ, EV_ADD | EV_ENABLE); //파일을 쓰기감지에 예약.
        }
        return true; //정상수행 true반환.
    }

    //index 목록에 실존하는 파일이 있는지 확인 하고 없으면 빈경로를 반환하는 메소드.
    std::string found_index_abs_path(std::string & path)
    {
        std::string tmp_path = path;
        struct stat sb;

        for (std::vector<std::string>::const_iterator it = this->my_loc->index.begin(); it != this->my_loc->index.end(); it++)
        {
            tmp_path = path + *it;
            if (stat(tmp_path.c_str(), &sb) == 0)
                return (tmp_path);
        }
        return ("");
    }

    //송신할 DELETE 응답정보를 만드는 메소드.
    void init_delete_response()
    {
        this->response.setVersion(this->request.getVersion());
        this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);

        if (this->response.getHeader_map().find("server") == this->response.getHeader_map().end())
            this->response.setHeader_map("server", "soo-je-webserver");
        if (this->response.getHeader_map().find("Date") == this->response.getHeader_map().end())
            this->response.setHeader_map("Date", util::get_date());
        if (this->response.getHeader_map().find("Connection") == this->response.getHeader_map().end())
            this->response.setHeader_map("Connection", "keep-alive");
        if (this->response.getHeader_map().find("Accept-Ranges") == this->response.getHeader_map().end())
            this->response.setHeader_map("Accept-Ranges", "bytes");
        std::cerr << "----init_delete_response()->push_write_bud()" << std::endl;
        push_write_buf("\0");
        //DELETE용 응답데이터 (시작줄 + 헤더 + 바디)만들기....
    }

    //응답데이터를 만들기전에 필요한 read/write 또는 unlink하는 메소드.
    bool ready_response_meta()
    {
        Server s = *this->my_server;
        std::string uri = this->getRequest().getTarget().substr(0, this->getRequest().getTarget().find('?')); //'?'부터 뒷부분 쿼리스트링 제거한 앞부분.

        if (this->request.getMethod() == "GET" || this->request.getMethod() == "POST")
        {
            if (uri[uri.length() - 1] != '/') //경로가 '/'로 끝나지 않으면 만들어준다.
			    uri += '/';
            std::string path = this->my_loc->root; //실제 서버경로.
            if (this->request.getTarget() != "/") //uri에 loc에 매칭된 경로외에 정보가 있다면 실제 서버의 경로에 이어 붙인다.
                path += uri.substr(this->my_loc->path.length());
            struct stat sb;
            if (stat(path.c_str(), &sb) == -1) //서버경로가 존재 하지 않을 때.
            {
                path.erase(--(path.end())); // '//'더블 슬레시 제거.
                if (stat(path.c_str(), &sb) == -1) // 그래도 없을 때.
                {
                    this->getResponse().setStatus("404"); //404처리.
                    return false; //바로 에러 페이지 제작 필요.
                }
            }
            if (path[path.length() - 1] == '/') //서버경로가 정규파일이 아닌 폴더일 때.
            {
                std::string abs_path = this->found_index_abs_path(path); //conf의 index 목록중에 실존하는 파일의 절대경로찾기.
                if (abs_path == "" && this->my_loc->autoindex == false) //오토인덱스도 꺼져있고, index목록에도 없을 때
                {
                    this->getResponse().setStatus("404"); //404처리.
                    return false; //바로 에러 페이지 제작 필요.
                }
                else if (abs_path == "" && this->my_loc->autoindex == true) //index는 없지만 오토인덱스가 켜져있을 때.
                {
                    this->init_autoindex_response(path); //오토인덱스 페이지 만들어서 kq통해 바로 리스폰.
                    return true ; //에러페이지는 만들지 않게한다.
                }
                else //conf에서의 index목록중에 있다면 해당 경로로 대치.
                    path = abs_path;
            }
            this->file_fd = open(path.c_str(),O_RDONLY, 0755);
            if (this->file_fd == -1) //열기 실패시.
            {
                this->getResponse().setStatus("500"); //500처리.
                return false; //바로 에러 페이지 제작 필요.
            }
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            add_kq_event(this->file_fd, EVFILT_READ, EV_ADD | EV_ENABLE); //파일을 읽기감지에 예약.
            std::cerr << "get(post) done : " << this->file_fd << std::endl;
        }
        else if (this->request.getMethod() == "DELETE")
        {
            if (uri[uri.length() - 1] != '/') //경로가 '/'로 끝나지 않으면 만들어준다.
			    uri += '/';
            std::string path = this->my_loc->root; //지울 디렉토리 지정 준비.
            if (uri.length() > this->my_loc->path.length()) //uri에 추가 경로가 있다면...
                path += uri.substr(this->my_loc->path.length()); //추가경로를 타겟의 path에 이어붙인다.
            //1.경로가 폴더면 위와 동일하게 폴더 내부를 비운다.
            //2.파일이면 unlink.
            struct stat sb;
            if (stat(path.c_str(), &sb) == 0) // 디렉토리일 때.(맨뒤에 슬래시달린상태로 stat되면 디렉토리).
            {
                path.erase(--(path.end())); //맨뒤 슬래시 제거.
                if (util::rm_sub_files(path.c_str()) == false) //재귀로 하위폴더 비우기.
                    return (this->getResponse().setStatus("500"), false); //500처리.
            }
            else //디렉토리가 아닐 때.
            {
                path.erase(--(path.end()));
                unlink(path.c_str()); //있든없든 지우고본다. (RFC: 서버는 삭제를 보장하지않음.)
            }
            this->init_delete_response();
            //3.kq에 "쓰기가능"감지으로 등록한다.
            //구현중...
        }
        else if (this->request.getMethod() == "PUT")
        {
            std::string path = this->my_loc->root; //실제 서버경로.
            if (this->request.getTarget() != "/") //uri에 loc에 매칭된 경로외에 정보가 있다면 실제 서버의 경로에 이어 붙인다.
                path += uri.substr(this->my_loc->path.length());
            struct stat sb;
            stat(path.c_str(), &sb);

            if (path[path.length() - 1] == '/' || S_ISDIR(sb.st_mode)) //put대상이 폴더일 때.
            {
                this->getResponse().setStatus("400"); //400처리.
                    return false; //바로 에러 페이지 제작 필요.
            }
            if (util::make_middle_pathes(path) == false) //경로의 중간 경로들이 없다면 만든다.
            {
                this->getResponse().setStatus("500"); //500처리.
                    return false; //바로 에러 페이지 제작 필요.
            }
            this->file_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755); //읽기전용, 없으면만듬, 덮어쓰기.
            if (this->file_fd == -1)
            {
                this->getResponse().setStatus("500"); //500처리.
                    return false; //바로 에러 페이지 제작 필요.
            }
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            add_kq_event(this->file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //"쓰기감지"등록.
        }
        else
            return (this->getResponse().setStatus("501"), false); //501처리. 지원하지않는 메소드.
        return true; //정상수행 true반환.
    }

    //요청경로가 로케이션 구조체중에 일치하면 그 경로로 로케이션구조체를 설정하는 메소드.
    void init_client_location()
    {
        size_t pos;
        std::string uri_loc = "";
        std::string & uri = this->request.getTarget();

        pos = uri.find('.'); //확장자 암시를 찾는다.
        if (pos != std::string::npos) //확장자 암시가 없다면.
        {
            while (uri[pos] != '/') //경로를 한 폴더 올라가서 uri_loc변수에 백업.
                pos--;
            uri_loc = uri.substr(0, pos + 1);
        }
        else //확장자 암시가 있다면.
        {
            pos = uri.find('?'); //쿼리스트링 암시를 찾아서,
            if (pos != std::string::npos)
                uri_loc = uri.substr(0, pos); //쿼리스트링이 있다면 '?'부터 뒷부분 지운다.
            else
                uri_loc = uri; //없으면 말고.
        }
        //**conf파일에서 location의 경로는 무조건 슬래쉬로 끝나야 적용가능. (conf파싱부분에서 loc경로를 슬래시로 닫아주는 처리가 필요)
        if (uri_loc[uri_loc.length() - 1] != '/')  //경로는 무조건 슬래시로 끝나야한다.(로케이션 구조체와 구분하기 위해)
            uri_loc += "/";	//슬래시가 없다면 만들어주자.

        this->my_loc = NULL;
        std::map<std::string, Location> &loc_map = this->my_server->get_loc_map();
        std::string key = "";
        for (std::string::const_iterator iter = uri_loc.begin(); iter != uri_loc.end(); iter++)
        { // 슬래시를 만나서 단계별로 경로가 만들어질 때마다 매칭되는 로케이션 유무를 확인한다.
            key += *iter;
            if (*iter == '/') //경로가 한단계 만들어질 때.
            {
                if (loc_map.find(key) == loc_map.end()) //구조체 중 찾아보고 없으면 종료.
                    break;
                else
                    this->my_loc = &loc_map[key]; //있다면 해당 구조체로 설정. 이어서 다음단계 찾기.
            }
        }
        if (this->my_loc == NULL)
            this->my_loc = &loc_map["/"]; //기본값으로 루트경로 구조체로 초기화.
    }

    //cgi실행이 필요한지 여부를 반환하는 메소드. cgi가 필요없으면 false반환. 있으면 cgi 정보를 설정하고 true반환.
    bool check_need_cgi()
    {
        Server s = *this->my_server;
        std::map<std::string, std::string> & cgi_infos = s.get_cgi_map();

        size_t offset = this->getRequest().getTarget().find('.'); //확장자를 암시하는 부분을 찾는다.
        if (offset == std::string::npos) //없다면 검사종료.
            return (false);
        size_t curr = offset;
        while (curr != this->getRequest().getTarget().length()) //확장자의 문자열을 하나하나검사.
            if (this->getRequest().getTarget()[curr] != '/' && this->getRequest().getTarget()[curr] != '?') //문자열에 '/'또는'?'가 있다면 검사중단.
                curr++;
        std::string pure_exe = this->getRequest().getTarget().substr(offset, curr - offset); //순수 확장자만 파싱해서 뽑는다.
        std::map<std::string, std::string>::const_iterator match_cgi = cgi_infos.find(pure_exe); //지원하는 cgi가 있는지 검사.
        if (match_cgi == cgi_infos.end()) //지원하는 cgi가 없다면 false반환
            return (false);
        else    //지원한다면 값을 맴버변수에 할당.
            this->cgi_program = match_cgi->second;
        while (this->getRequest().getTarget()[offset] != '/') //현재 확장자의 파일이름이 있는 곳으로 포인터를 옮긴다.
            offset--;
        this->cgi_file = this->getRequest().getTarget().substr(offset + 1, curr - offset - 1); //파일이름 + 확장자 형식의 순수 파일명을 뽑기.
        return true;
    }

    //비정제 data를 파싱해서 맴버변수"request"를 채우는 메소드.
    bool parse_request()
    {
        if ((this->request.parse(this->read_buf, this->response.getStatus())) == false) //read_buf 파싱.
            return false;
        std::cerr << "~~parse_request()에서 실행 파서 값 출력~~~~~~~~~~~~~~~~" << std::endl;
        std::cerr << "Method : " << this->request.getMethod() << std::endl;
	    std::cerr << "Target : " << this->request.getTarget() << std::endl;
	    std::cerr << "Version : " << this->request.getVersion() << std::endl;
	    std::cerr << "Headers: " << std::endl;
	    std::map<std::string, std::string> temp = this->request.getHeaders();
	    std::map<std::string,std::string>::iterator iter;
	    for(iter = temp.begin() ; iter != temp.end(); iter++){
		    std::cerr << iter->first << ":"<< iter->second << std::endl;
	    }
	    std::cerr << "Body : " << this->request.getBody() << std::endl;
        std::cerr << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        return true; //문제없이 파싱이 끝나면 true반환.
    }

    //소켓fd만 제외하고 모두 깡통으로만드는 메소드.
    bool clear_client()
    {
        //socket_fd,server_fd,_ev_cmds,my_server,status_msg빼고 모두 초기상태로 초기화한다...
        this->read_buf.clear();
        this->write_buf.clear();
        this->file_fd = -1;
        this->file_buf.clear();
        this->write_size = 0;
        this->read_size = 0;
        this->cgi_mode = false; // ?
        this->my_loc = NULL;
        this->cgi_program.clear();
        this->cgi_file.clear();

        return true; //문제없으면 true리턴.
    }

    //400번대 에러가 발생했는지 검사하는 메소드.
    bool check_client_err()
    {
        // // path 정의해줘야 함. (동작 확인 필요)
        std::string uri = this->getRequest().getTarget().substr(0, this->getRequest().getTarget().find('?'));
        std::string root = this->my_loc->root+ "/";
        std::string path;
        path = uri.replace(0, this->my_loc->path.length(), root);
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
        if (find(this->my_loc->accept_method.begin(), this->my_loc->accept_method.end(), \
        this->getRequest().getMethod()) == this->my_loc->accept_method.end())
        return (this->getResponse().setStatus("405"), true);
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

    //cgi를 실행하는 메소드.
    bool excute_cgi()
    {
        std::string file_name = "cgi_result_" + util::num_to_string(this->socket_fd);
        if ((this->file_fd = open(file_name.c_str(), O_RDWR | O_CREAT | O_APPEND, 0755)) == -1)//읽쓰기, 없으면만듬, 이어쓰기가능.
        {
            perror("open cgi_result err");
            this->getResponse().setStatus("500"); //500처리.
            return false; //바로 에러 페이지 제작 필요.
        }
        int pid = -1;
        if ((pid = fork()) < 0)
        {
            perror("fork err");
            this->getResponse().setStatus("500"); //500처리.
            return false; //바로 에러 페이지 제작 필요.
        }
        if (pid == 0) //자식프로세스 일 때.
        {
            std::string file_path = this->request.getTarget(); //실행 할 상대경로 제작.
            file_path = file_path.substr(this->my_loc->path.length());
            file_path = this->my_loc->root + file_path;
            char *buf = realpath(file_path.c_str(), NULL); //상대경로를 절대경로로 변경.
            if (buf != NULL) //변환성공 했을 때.
                file_path = std::string(buf); //실행할 경로를 절대경로로 재지정.
            char **env = this->init_cgi_env(file_path); //환경변수 준비.
            dup2(this->file_fd, 1); //출력 리다이렉트.
            if (file_path.substr(file_path.rfind('.')) == ".bla") //인트라 cgi테스터용 특별처리.
            {   //(인트라 cgi프로그램은 인자를 직접 받지않고 환경변수로 받는다.)
                char **arg = (char **)malloc(sizeof(char *) * 2);
                arg[0] = strdup("./cgi-bin/cgi_tester");
                arg[1] = NULL;
                if (execve(arg[0], arg, env) == -1)
                {
                    perror("execve err bla");
                    exit(1);
                }
            } else {
                char **arg = (char **)malloc(sizeof(char *) * 3);
                arg[0] = strdup(this->cgi_program.c_str()); //예시 "/usr/bin/python"
                arg[1] = strdup(file_path.c_str()); //실행할 파일의 절대경로.
                arg[2] = NULL;
                if (execve(arg[0], arg, env) == -1) //cgi 실행.
                {
                    perror("execve err normal");
                    exit(1);
                }
            }
            exit(0);
        }
        else //부모프로세스는 논블럭설정하고 "읽기가능"감지에 등록한다.
        {
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭으로 설정.
            add_kq_event(this->file_fd, EVFILT_READ, EV_ADD | EV_ENABLE);
            return true;
        }
    }

    char *get_client_ip(void)
    {
        struct sockaddr_in client_sockaddr;
        socklen_t client_sockaddr_len = sizeof(client_sockaddr);
        getsockname(this->socket_fd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_len);
        return (inet_ntoa(client_sockaddr.sin_addr));
    }

    // CGI 환경변수 (PATH_INFO, SCRIPT_NAME) 설정을 위한 메소드
    void set_cgi_env_path(std::map<std::string, std::string> &cgi_env_map)
    {
        // target 중에서 (상대경로) CGI 프로그램까지의 문자열 : SCRIPT_NAME
        // 1. program name 찾아오기
        size_t pos = this->get_cgi_program().rfind("/");
        std::string program_name;
        if (pos != std::string::npos)
            program_name = this->get_cgi_program().substr(pos + 1);
        else
            program_name = this->get_cgi_program();
        // 2. target에서 program_name 찾아서 앞부분 SCRIPT_NAME 으로 넣어주기.
        size_t pos1 = this->getRequest().getTarget().find(program_name);
        pos1 += program_name.length();
        cgi_env_map["SCRIPT_NAME"] = this->getRequest().getTarget().substr(pos1);
        // 2. 시작지점 옮겨주기 -> PATH_INFO(상대경로 그대로)
        if (pos1 == this->getRequest().getTarget().length() - 1)
        {
            // error
        }
        size_t pos2 = pos1 + 1;
        while (pos2 < this->getRequest().getTarget().length() && this->getRequest().getTarget()[pos2] != '?')
            pos2++;
        if (pos2 == pos1 + 1)
        {
            // error
        }
        cgi_env_map["PATH_INFO"] = this->getRequest().getTarget().substr(pos1 + 1, pos2 - pos1);
    }

    //cgi자식프로세스가 사용할 환경변수 목록을 2차원포인터로 제작하는 메소드.
    char **init_cgi_env(std::string & file_path)
    {
        // 0. file_path : 서버 상 절대 경로
        // 1. 일단 필요한 정보들 가공해서 map 에 넣기
        std::map<std::string, std::string> cgi_env_map;
        cgi_env_map["AUTH_TYPE"] = ""; // 인증과정 없으므로 NULL
        cgi_env_map["CONTENT_LENGTH"] = this->getResponse().getBody().length(); // 길이 모른다면 -1
        cgi_env_map["CONTENT_TYPE"] = this->getResponse().getHeader_map()["Content-Type"];  // 빈 경우 혹은 모르는 경우가 있는지 확인해야 함. (그 경우 NULL)
        cgi_env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
        cgi_env_map["REQUEST_METHOD"] = this->getRequest().getMethod();
        cgi_env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
        cgi_env_map["PATH_TRANSLATED"] = std::string(file_path);
        size_t pos = this->getRequest().getTarget().find('?');
        if (pos != std::string::npos || pos < this->getRequest().getTarget().length() - 1)
            cgi_env_map["QUERY_STRING"] = this->getRequest().getTarget().substr(this->getRequest().getTarget().find('?'));
        cgi_env_map["REMOTE_ADDR"] = std::string(get_client_ip());
        cgi_env_map["REMOTE_USER"] = ""; // 인증과정 없으므로 NULL
        cgi_env_map["SERVER_NAME"] = this->get_myserver()->get_host() + ":" + util::num_to_string(this->get_myserver()->get_port());
        cgi_env_map["SERVER_PORT"] = this->get_myserver()->get_port();
        cgi_env_map["SERVER_PROTOCOL"] = this->getRequest().getVersion();
        cgi_env_map["SERVER_SOFTWARE"] = "soo-je-webserv/1.0";
        this->set_cgi_env_path(cgi_env_map);
        char **cgi_env = new char *[sizeof(char *) * cgi_env_map.size() + 1]; // 환경변수의 개수 + 1 만큼 할당
        // 2. 맵의 내용들 2차원 배열로 저장하기
        int i = 0;
        for(std::map<std::string, std::string>::iterator iter = cgi_env_map.begin(); iter != cgi_env_map.end(); iter++)
        {
            cgi_env[i] = strdup(((*iter).first + "=", (*iter).second).c_str());
            i++;
        }
        cgi_env[cgi_env_map.size()] = NULL;
        return (cgi_env);
        //단톡DM방 책갈피의 cgi IBM문서 참조...., https://www.oreilly.com/openbook/cgi/ch02_02.html
        /*
        - AUTH_TYPE : 사용자 인증이 사용되는 경우, 이 사용자가 인증된 사용자라면 그 사용자를 인증하기 위해 사용한 메소드. 우리는 인증 과정이 없으므로 NULL
        - CONTENT_LENGTH : 요청 본문의 길이, 모르면 -1
        - CONTENT_TYPE : this->response.getHeader_map()["Content-Type"], 모르는 경우가 있다면? null
        - GATEWAY_INTERFACE : "CGI/1.1" CGI 스펙의 버전
        - REQUEST_METHOD : 이 요청을 작성할 때 사용된 메소드. this->request.getMethod()
        - SERVER_PROTOCOL : "HTTP/1.1"
        - PATH_INFO : CGI 프로그램명 이후의 uri
        - PATH_TRANSLATED : 서버상의 절대경로
        - QUERY_STRING : after ?
        - REMOTE_ADDR : client IP address
        - REMOTE_HOST : The remote hostname (만약에 서버가 hostname을 모르고, REMOTE_ADDR이 설정되어 있다면 이 변수는 unset 되어도 괜찮다.)
        - REMOTE_USER : 사용자가 인증된경우 로그인을 넣고, 인증되지 않은 경우 NULL. 우리는 인증 과정이 없으므로 무조건 NULL
        - SCRIPT_NAME : 프로토콜 이름에서 HTTP 요청의 첫 번째 라인에 있는 조회 문자열까지, URL의 부분을 리턴. this->getRequest().getTarget()
        - SERVER_NAME : The server's hostname or IP address. this->get_myserver()->get_host + ":" + this->get_myserver()->get_port()
        - SERVER_PORT : 요청이 수신된 포트번호. this->get_myserver()->get_port();
        - SERVER_PROTOCOL : 요청이 사용하는 프로토콜 이름과 버전. this->getRequest().getVersion()
        - SERVER_SOFTWARE : 컨테이너의 이름과 버전을 리턴합니다. format : name/version ? 임의로 적어도 되는듯?
        - HTTP_COOKIE : ? 필요하다면 추가
        - WEBTOP_USER : ? 필요하다면 추가
        - NCHOME : ? 필요하다면 추가
        */
        /*
            * from : https://www.oreilly.com/openbook/cgi/ch02_04.html
            * http://some.machine/cgi-bin/display.pl/cgi/cgi_doc.txt
            * Since the server knows that display.pl is the name of the program,
            * the string "/cgi/cgi_doc.txt" is stored in the environment variable PATH_INFO.
            * Meanwhile, the variable PATH_TRANSLATED is also set, which maps the information stored in PATH_INFO to the document root directory
            * (e.g., /usr/local/etc/httpd/ public/cgi/cgi-doc.txt).
        */
    }
};

#endif