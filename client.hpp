#ifndef CLIENT_CLASS_HPP
# define CLIENT_CLASS_HPP
#include "client_base.hpp"
#include "cgi_controller.hpp"

// Colors
#define RED "\x1b[0;31m"
#define BLUE "\x1b[0;34m"
#define GREEN "\x1b[0;32m"
#define YELLOW "\x1b[0;33m"
#define MAGENTA "\x1b[0;35m"
#define RESET "\x1b[0m"

class Client : public Client_base
{
private:
    Location * my_loc; //요청이 처리될 영역을 지정한 로케이션 구조체.
    std::vector<struct kevent> * _ev_cmds; //kq 감지대상 벡터.
    std::map<int, Client*> * _file_map; //파일 맵.
    Server * my_server; //현재 클라이언트의 서버.(conf 데이터 불러오기 가능).
    std::map<std::string, std::string> * status_msg;
    std::string  cgi_program; //cgi를 실행할 프로그램 경로 (예시 "/usr/bin/python")	// cgi_controller.hpp로 이동
    std::string  cgi_file; //cgi를 실행할 파일 경로 (예시 "hello.py")	// cgi_controller.hpp로 이동
    std::string  cgi_body_file; //cgi실행전 사용할 바디 파일 이름.	// cgi_controller.hpp로 이동
    int cgi_pid;	// cgi_controller.hpp로 이동
    int cgi_status;	// cgi_controller.hpp로 이동
	CgiController cgi_controller;

public:
    Client(std::vector<struct kevent> * cmds, std::map<int,Client*> * files) : my_loc(NULL), _ev_cmds(cmds) ,_file_map(files) \
    , my_server(NULL), status_msg(NULL), cgi_program(""), cgi_file(""),  cgi_body_file(""),  cgi_pid(-1), cgi_status(END)
    {
        socket_fd = -1;
        read_buf = "";
        write_buf = "";
        file_fd = -1;
        file_buf = "";
        write_size = 0;
        cgi_mode = false;
        cgi_file_name = "";
        is_done_chunk = false;
    }

    ~Client(){}
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

    //응답클래스를 제작하는 메소드.
    bool init_response()
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
        //1. 위와 같이 응답클래스를 초기화한다.
        //2. file_buf의 크기를 헤더필드 Content-Length에 추가한다. 그외에 필요한 정보가 있으면 추가.
        //3. (cgi냐 단순html파일이냐 에따라 다르게 file_buf 컨트롤 필요).
        //4. 맴버변수 write_buf에 하나의 데이터로 저장한다. (시작줄 + 헤더 + file_buf).
        //5. kq에 소켓을 "쓰기가능"감지로 등록.
        if (this->response.getHeader_map().find("Content-Length") != this->response.getHeader_map().end())
            this->response.getHeader_map().erase("Content-Length");
        if (this->cgi_mode == true && this->file_buf.size() != 0) //cgi일때 바디 사이즈 재측정.
        {
            size_t pos = this->file_buf.find("\r\n\r\n"); //캐리지리턴 기준으로 그 아래만 사이즈 측정한다.
            if (pos == std::string::npos)
                this->response.setHeader_map("Content-Length", "0");
            else
                this->response.setHeader_map("Content-Length", util::num_to_string(this->file_buf.size() - pos - 4));
        }
        else
            this->response.setHeader_map("Content-Length", util::num_to_string(this->file_buf.size()));
        if (this->response.getHeader_map().find("content-Type") == this->response.getHeader_map().end())
            this->find_mime_type(this->request.getTarget());
        this->response.setBody(this->file_buf);
        this->push_write_buf(this->file_buf);

        add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓을을 쓰기감지에 예약.
        return true; //문제없이 응답클래스를 초기화했으면 true반환
    }

    void push_write_buf(const std::string & response_body)
    {
        // if (this->response.getStatus() == "400")
        //     exit(9);
        //스타트라인
        std::cerr << YELLOW << this->response.getVersion() + " " + this->response.getStatus() + " " + this->response.getStatus_msg() << RESET << std::endl;
        this->write_buf = this->response.getVersion() + " " + this->response.getStatus() + " " + this->response.getStatus_msg() + "\r\n";
        //헤더 부분
        std::map<std::string, std::string> temp = this->response.getHeader_map();
    	std::map<std::string,std::string>::reverse_iterator iter;
	    for(iter = temp.rbegin() ; iter != temp.rend(); iter++)
		    this->write_buf = this->write_buf + iter->first + ": " + iter->second + "\r\n";
        //개행추가 부분, cgi의 경우 바디 윗부분에 개행이 추가되어있다.바디에 개행이 추가되는 것을 방지.
        if (this->cgi_mode == false)// || request.getMethod() != "HEAD")
            this->write_buf = this->write_buf + "\r\n";
        //바디 부분
        if (response_body.size() != 0 && request.getMethod() != "HEAD")
            this->write_buf += response_body;
    }

    //오토인데스 응답페이지를 만들고 송신준비를 하는 메소드.
    void init_autoindex_response(std::string & path)
    {
        (void)path;
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
        temp_body +=  this->request.getTarget() + "</title></head><body bg color='white'>  <h1> Index of " + this->request.getTarget() + "</h1>  <hr>  <pre>";
        while ((ent = readdir(dir)) != NULL)//경로의 파일들 바디에 넣기
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue ;
            temp_body = temp_body + "    <a href= " + ent->d_name + ">" + ent->d_name + "</a><br>";
        }
        temp_body += "</pre>  <hr></body></html>";
        this->response.setBody(temp_body);//바디 입력
        this->response.setHeader_map("Content-Length", util::num_to_string(this->response.getBody().length()));//바디 크기
        closedir(dir);
        push_write_buf(this->response.getBody());
        add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
    }

    void find_mime_type(std::string & path)
    {
        std::string temp = "";
        //값"text/html",text/css, images/png, jpeg, gif
        //헤더파일형식 Content-Type: text/html;

        if (util::ft_split(path, "./").size() != 0)
            temp = util::ft_split_s(util::ft_split_s(path, "/").back(), ".").back();//파일 확장자만 반환하기

        if (temp == "css")
            this->response.setHeader_map("Content-Type", "text/css");
        else if (temp == "png")
            this->response.setHeader_map("Content-Type", "image/png");
        else if (temp == "jpeg")
            this->response.setHeader_map("Content-Type", "image/jpeg");
        else if (temp == "gif")
            this->response.setHeader_map("Content-Type", "image/gif");
        else
            this->response.setHeader_map("Content-Type", "text/html");// ; charset=UTF-8");
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
        #ifdef TEST
        std::cerr << "errrrrr11" << std::endl;
        #endif
        if (this->response.getStatus() == "500" || s.get_default_error_page().find(this->response.getStatus()) == s.get_default_error_page().end())
        {
            #ifdef TEST
            std::cerr << "errrrrr22" << std::endl;
            std::cerr << "rrrr22 : " + response.getStatus()  << std::endl;
            #endif
            this->response.setVersion("HTTP/1.1");
            this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);
            //헤더도 넣기
            this->response.setHeader_map("server", "soo-je-webserver");
            this->response.setHeader_map("Date", util::get_date());
            this->response.setHeader_map("content-Type", "text/html");
            this->response.setHeader_map("Content-Length", "4");
            this->response.setHeader_map("Connection", "close");
            // this->response.setHeader_map("Accept-Ranges", "bytes");
            this->response.setBody(this->response.getStatus() + '\0');
            push_write_buf(this->response.getBody());
            add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
        }
        else
        {
            #ifdef TEST
            std::cerr << "errrrrr333" << std::endl;
            #endif
            std::string err_page = s.get_default_error_page().find(this->response.getStatus())->second;
            #ifdef TEST
            std::cerr << "rrr333 : " << err_page << std::endl;
            #endif
            struct stat sb;
            if (stat(err_page.c_str(), &sb) != 0)//루트경로 추가할 것
                return (this->response.setStatus("500"), this->ready_err_response_meta());
            if ((S_IFMT & sb.st_mode) != S_IFREG)//일반파일이 아닐 경우
                return (this->response.setStatus("500"), this->ready_err_response_meta());
            if ((this->file_fd = open(err_page.c_str(), O_RDONLY)) == -1)
                return (this->response.setStatus("500"), this->ready_err_response_meta());//터지면 경로 문제
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            //헤더 내용은 뒤에 다른 함수에서 추가
            add_kq_event(this->file_fd, EVFILT_READ, EV_ADD | EV_ENABLE); //파일을 읽기감지에 예약.
            this->_file_map->insert(std::make_pair(this->file_fd, this));//파일 맵에 추가.
        }
        #ifdef TEST
        std::cerr << "errrrrr444" << std::endl;
        #endif
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
        this->response.setStatus("204");
        this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);

        if (this->response.getHeader_map().find("server") == this->response.getHeader_map().end())
            this->response.setHeader_map("server", "soo-je-webserver");
        if (this->response.getHeader_map().find("Date") == this->response.getHeader_map().end())
            this->response.setHeader_map("Date", util::get_date());
        if (this->response.getHeader_map().find("Connection") == this->response.getHeader_map().end())
            this->response.setHeader_map("Connection", "keep-alive");
        if (this->response.getHeader_map().find("Accept-Ranges") == this->response.getHeader_map().end())
            this->response.setHeader_map("Accept-Ranges", "bytes");

        this->response.setBody("");
        push_write_buf(this->response.getBody());
        //DELETE용 응답데이터 (시작줄 + 헤더 + 바디)만들기....
    }

    //응답데이터를 만들기전에 필요한 read/write 또는 unlink하는 메소드.
    bool ready_response_meta()
    {
        std::string uri = this->getRequest().getTarget().substr(0, this->getRequest().getTarget().find('?')); //'?'부터 뒷부분 쿼리스트링 제거한 앞부분.

        if (this->request.getMethod() == "GET" || this->request.getMethod() == "POST" || this->request.getMethod() == "HEAD")
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
            stat(path.c_str(), &sb);
            if (sb.st_size == 0)
                return (this->init_response(), true);
            this->file_fd = open(path.c_str(),O_RDONLY, 0644);
            if (this->file_fd == -1) //열기 실패시.
            {
                this->getResponse().setStatus("500"); //500처리.
                return false; //바로 에러 페이지 제작 필요.
            }
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            add_kq_event(this->file_fd, EVFILT_READ, EV_ADD | EV_ENABLE); //파일을 읽기감지에 예약.
            this->_file_map->insert(std::make_pair(this->file_fd, this));//파일 맵에 추가.

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
            add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓을을 쓰기감지에 예약.
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
            this->file_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); //읽기전용, 없으면만듬, 덮어쓰기.
            if (this->file_fd == -1)
            {
                this->getResponse().setStatus("500"); //500처리.
                    return false; //바로 에러 페이지 제작 필요.
            }
            fcntl(this->file_fd, F_SETFL, O_NONBLOCK); //논블럭 설정.
            add_kq_event(this->file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE); //"쓰기감지"등록.
            this->_file_map->insert(std::make_pair(this->file_fd, this));//파일 맵에 추가.
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

    //비정제 data를 파싱해서 맴버변수"request"를 채우는 메소드.
    bool parse_request()
    {

        if (this->response.getStatus() == "800")//상태코드 800인지 확인하기
            return (this->request.ft_chunk_push_body(this->read_buf, this->response.getStatus()));
        else if ((this->request.parse(this->read_buf, this->response.getStatus())) == false) //read_buf 파싱.
            return false;
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
        this->cgi_mode = false;
        this->my_loc = NULL;
        this->cgi_program.clear();
        this->cgi_file.clear();
        this->response.clear_response();
        this->request.clear_request();
        this->cgi_file_name.clear();
        this->cgi_body_file.clear();
        this->is_done_chunk = false;
        this->cgi_pid = -1;
        this->cgi_status = END;
        return true;
    }

    //400번대 에러 중 파일을 사용하지 않는 에러가 발생했는지 검사하는 메소드.
    bool check_client_err()
    {
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
        if (this->getRequest().getMethod() != "PUT" && access(path.c_str(), F_OK) == -1)
            return (this->getResponse().setStatus("404"), true);
        // 403 : access(path, R_OK) (읽기권한)
        if (this->getRequest().getMethod() != "PUT" && access(path.c_str(), R_OK) == -1)
            return (this->getResponse().setStatus("403"), true);
        // 405 : 현재 경로에서 실행할 수 있는 method인지 확인
        if (find(this->my_loc->accept_method.begin(), this->my_loc->accept_method.end(), \
        this->getRequest().getMethod()) == this->my_loc->accept_method.end())
            return (this->getResponse().setStatus("405"), true);
        // 413 : client_max_body_size 확인
        if (this->getRequest().getBody().length() > this->my_loc->client_max_body_size)
            return (this->getResponse().setStatus("413"), true);
        return false;
    }

	// ANCHOR CGI

	bool check_need_cgi()
    {
        Server s = *this->my_server;
        std::map<std::string, std::string> & cgi_infos = this->my_loc->cgi_map;
        size_t offset = this->getRequest().getTarget().find('.'); //확장자를 암시하는 부분을 찾는다.
        if (offset == std::string::npos) //없다면 검사종료.
            return (false);
        size_t curr = offset;
        while (curr != this->getRequest().getTarget().length()) //확장자의 문자열을 하나하나검사.
            if (this->getRequest().getTarget()[curr] != '/' && this->getRequest().getTarget()[curr] != '?') //문자열에 '/'또는'?'가 있다면 검사중단.
                curr++;
            else
                break;
        std::string pure_exe = this->getRequest().getTarget().substr(offset, curr - offset); //순수 확장자만 파싱해서 뽑는다.
        std::map<std::string, std::string>::const_iterator match_cgi = cgi_infos.find(pure_exe); //지원하는 cgi가 있는지 검사.
        std::string program;
        if (match_cgi == cgi_infos.end()) //지원하는 cgi가 없다면 false반환
            return (false);
        else    //지원한다면 값을 맴버변수에 할당.
            program = match_cgi->second;
        offset = this->request.getTarget().find(this->my_loc->path);
        std::string script = this->request.getTarget().substr((offset + this->my_loc->path.length()), curr - offset - 1);
        this->cgi_mode = true;
        this->cgi_controller.initialize(program, script, this->request.getTarget(), this->my_loc->root, this->request.getHeaders(), this->request.getMethod(), this->socket_fd, this->my_server->get_host(), this->my_server->get_port(), this->request.getVersion(), this->response.get_sid());
        return true;
    }

	bool ready_body_file()
    {
        bool result = this->cgi_controller.ready_body_file();
        if (result == false)
            this->response.setStatus("500");
        else
        {
            fcntl(this->cgi_controller.get_file_fd(), F_SETFL, O_NONBLOCK);
            add_kq_event(this->cgi_controller.get_file_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE);
            this->_file_map->insert(std::make_pair(this->cgi_controller.get_file_fd(), this));
        }
        this->file_fd = this->cgi_controller.get_file_fd();
        return result;
    };

	int check_cgi_status()
    {
        int status = this->cgi_controller.get_status();
        if (status == RUNNING)
        {
            add_kq_event(this->cgi_controller.get_file_fd(), EVFILT_READ, EV_ADD | EV_ENABLE);
            this->_file_map->insert(std::make_pair(this->cgi_controller.get_file_fd(), this));
        }
        else if (status == ERROR)
        {
            this->response.setStatus("500");
            this->ready_err_response_meta();
        }
        return status;
    }

	bool excute_cgi()
    {
        bool result = this->cgi_controller.execute();
        if (result == false)
            this->response.setStatus("500");
        else
        {
            fcntl(this->cgi_controller.get_file_fd(), F_SETFL, O_NONBLOCK);
            add_kq_event(this->cgi_controller.get_file_fd(), EVFILT_READ, EV_ADD | EV_ENABLE);
            this->_file_map->insert(std::make_pair(this->cgi_controller.get_file_fd(), this));
            this->file_fd = this->cgi_controller.get_file_fd();
        }
        return result;
    }

    std::string check_chunked()
    {
        if (this->response.getStatus() == CHUNKED)
            return CHUNKED;
        if (this->request.getHeaders().find("Transfer-Encoding") != this->request.getHeaders().end())
        {
            std::string temp = util::ft_split(this->request.getHeaders().find("Transfer-Encoding")->second, " ")[0];
            if (temp.find("chunked") != std::string::npos)
            {
                const size_t body_size = this->request.getBody().length();
                const std::string & body_data = this->request.getBody();
                size_t curr = 0;
                std::string pase_body = "";
                while (body_size > 2)
                {
                    size_t rn = body_data.find("\r\n", curr);
                    if (rn == curr)
                    {
                        this->request.setBody("");
                        this->response.setStatus("400");
                        this->is_done_chunk = true;
                        perror("400 generate");
                        return "400";
                    }
                    if (rn == std::string::npos)
                    {
                        this->read_buf = body_data.substr(curr);
                        break;
                    }
                    int b = strtol(body_data.substr(curr, rn).c_str(), NULL, 16);
                    int a = body_size - (rn + 2);
                    if (a < (b + 2))
                    {
                        this->read_buf = body_data.substr(curr);
                        break;
                    }
                    if (b == 0)
                    {
                        this->is_done_chunk = true;
                        this->request.setBody(pase_body);
                        this->response.setStatus("200");
                        return "200";
                    }   
                    pase_body += body_data.substr(rn + 2, b);
                    curr = (rn + 2) + (b + 2);
                }
                this->request.setBody(pase_body);
                this->response.setStatus(CHUNKED);
                return CHUNKED;
            }
        }
        return ("200");      
    }

    std::string check_bodysize()
    {   
        if (this->response.getStatus() == LENGTHLESS)
        {
            std::string temp = util::ft_split(this->request.getHeaders().find("Content-Length")->second, " ")[0];
            int expect_size = atoi(temp.c_str());
            int real_size = this->request.getBody().length();
            if (real_size >= expect_size)
            {
                this->response.setStatus("200");
                return "200";
            }
            return LENGTHLESS;
        }
        if (this->request.getHeaders().find("Content-Length") != this->request.getHeaders().end())
        {
            std::string temp = util::ft_split(this->request.getHeaders().find("Content-Length")->second, " ")[0];
            int expect_size = atoi(temp.c_str());
            int real_size = this->request.getBody().length();
            if (real_size < expect_size)
            {
                this->response.setStatus(LENGTHLESS);
                return LENGTHLESS;
            }
        }
        return ("200");
    }

    //리다이렉트 응답을 만드는 메소드.
    void init_redirect_response()
    {
        this->response.setVersion("HTTP/1.1");
        this->response.setStatus("301");
        this->response.setStatus_msg((*(this->status_msg)).find(this->response.getStatus())->second);
        this->response.setBody("");
        push_write_buf(this->response.getBody());
    }

    bool check_redirect()
    {
        std::map<std::string,std::string> redirect = this->my_loc->redirection;
        if (redirect.find("301") != redirect.end())
        {
            this->response.setHeader_map("Location", util::ft_split(redirect.find("301")->second," ")[0]);
            this->init_redirect_response();
            add_kq_event(this->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
            return true;
        }
        return false;
    }

    bool chunk_done()
    {
        return this->is_done_chunk;
    }

    void manage_session()
    {
        if (this->response.get_sid() != 0)
            return ;
        long sid = 0;
        std::stringstream sstream;
        if (this->request.getHeaders().find("Cookie") != this->request.getHeaders().end())
        {
            std::vector<std::string> c_vec = util::ft_split(this->request.getHeaders().find("Cookie")->second, ";");
            for (std::vector<std::string>::iterator iter(c_vec.begin()); iter != c_vec.end(); iter++)
            {
                std::vector<std::string> key_val = util::ft_split(*iter, "=");
                if (key_val.size() < 2)
                    continue;
                std::string key = util::ft_split(key_val[0], " ")[0];
                std::string val = util::ft_split(key_val[1], " ")[0];
                if (key != "session_id")
                    continue;
                sid = atol(val.c_str());
                break;
            }
            //세션(sid) 있다면 서버 sid map에 접근
            if (sid != 0 && (this->my_server->get_sid_map().find(sid) != this->my_server->get_sid_map().end()))
            {
                this->response.set_sid(sid);
                if (this->my_server->get_sid_map().find(sid)->second == "old")
                    return;
                this->my_server->get_sid_map()[sid] = "old"; //4 재확인 후 정말 있다면 값을 old로 재설정.
                sstream << sid;
                std::string val = "status=" + this->my_server->get_sid_map().find(sid)->second + "; Path=/";
                this->response.setHeader_map("Set-Cookie", val);
                return ;
            }
        }   //없다면 new로 새로 만들어준다.
        sid = this->my_server->create_sid();
        this->response.set_sid(sid);
        sstream << sid;
        std::string val = "session_id=" + sstream.str() + "; Path=/\r\nSet-Cookie: status=" \
        + this->my_server->get_sid_map().find(sid)->second + "; Path=/";
        this->response.setHeader_map("Set-Cookie", val);
    }
};

#endif