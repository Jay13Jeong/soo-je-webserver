#ifndef WEBSERV_CLASS_HPP
# define WEBSERV_CLASS_HPP
#include <vector>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stack>
#include <unistd.h>
#include <algorithm>
#include "util.hpp"
#include "server.hpp"
#include "client.hpp"
#include <sys/socket.h> //socket
#include <sys/event.h> //kqueue
#include <stdexcept>
#include <sstream>
#include <map>
#include <exception>
#include <cctype>

#define DETECT_SIZE 44497 //한 횟차에 처리할 최대 이벤트 갯수.
#define FAIL        -1 //실패를 의미하는 매크로.
#define RECV_ALL    1 //모두 수신 받음을 의미.
#define SEND_ALL    1 //모두 수신 받음을 의미.
#define LENGTHLESS  "700" //사이즈 부족.

// Colors
#define RED "\x1b[0;31m"
#define BLUE "\x1b[0;34m"
#define GREEN "\x1b[0;32m"
#define YELLOW "\x1b[0;33m"
#define MAGENTA "\x1b[0;35m"
#define RESET "\x1b[0m"

class Webserv
{
public:
    Webserv(/* args */){};
    ~Webserv(){};
    std::vector<Server> &get_server_list()
    {
        return this->server_list;
    };
    void set_server_list(Server & new_server)
    {
        this->server_list.push_back(new_server);
    };

    //서버포트들을 개방하는 메소드.
    void open_ports()
    {
        for (size_t i(0);i < this->get_server_list().size();i++)
        {
            this->get_server_list()[i].open_port();
        }
    }

    //fd와 "감지할 행동", kevent지시문을 인자로 받아서 감지목록에 추가하는 메소드.
    void add_kq_event(uintptr_t ident, int16_t filter, uint16_t flags)
    {
        struct kevent new_event;

        EV_SET(&new_event, ident, filter, flags, 0, 0, NULL);
        this->ev_cmds.push_back(new_event);
    }

    //서버들을 감지목록에 추가하는 메소드.
    void regist_servers_to_kq()
    {
        for (size_t i(0);i < this->get_server_list().size();i++)
            add_kq_event(this->get_server_list()[i].get_fd(), EVFILT_READ, EV_ADD | EV_ENABLE);
    }

    void init_servers_map()
    {
        for (size_t i(0);i < this->get_server_list().size();i++)
        {
            this->server_map[this->server_list[i].get_fd()] = this->server_list[i];
            this->server_map[this->server_list[i].get_fd()].init_location_map(); //로케이션 맵도 같이 초기화.
            this->server_map[this->server_list[i].get_fd()].init_default_location(); //로케이션 초기화대상이없으면 하나 만들어주기.
        }

    }

    void init_status_map()
    {
        status_map.insert(std::make_pair("200","OK"));
        status_map.insert(std::make_pair("201","Created"));
        status_map.insert(std::make_pair("204","No Content"));
        status_map.insert(std::make_pair("205","Reset Content"));
        status_map.insert(std::make_pair("301","Moved Permanently"));
        status_map.insert(std::make_pair("303","See Other"));
        status_map.insert(std::make_pair("307","Temporary Redirect"));
        status_map.insert(std::make_pair("400","Bad Request"));
        status_map.insert(std::make_pair("401","Unauthorized"));
        status_map.insert(std::make_pair("403","Forbidden"));
        status_map.insert(std::make_pair("404","Not Found"));
        status_map.insert(std::make_pair("405","Method Not Allowed"));
        status_map.insert(std::make_pair("408","Request Timeout"));
        status_map.insert(std::make_pair("410","Gone"));
        status_map.insert(std::make_pair("411","Length Required"));
        status_map.insert(std::make_pair("413","Payload Too Large"));
        status_map.insert(std::make_pair("414","URI Too Long"));
        status_map.insert(std::make_pair("500","Internal Server Error"));
        status_map.insert(std::make_pair("501","Not Implemented"));
        status_map.insert(std::make_pair("503","Service Unavailable"));
        status_map.insert(std::make_pair("504","Gateway Timeout"));
        status_map.insert(std::make_pair("505","HTTP Version Not Supported"));
    }

    //서버를 실행하는 메소드.
    void start()
    {   
        std::cerr << ":::: SOO-JE-WEBSERVER start ::::" << std::endl;
        int kq_fd; //커널큐 fd.
        int detected_count = 0; //감지된 이벤트 갯수.
        struct kevent detecteds[DETECT_SIZE]; //감지 된 이벤트벡터.
        struct kevent* curr_det; //현재처리중인 이벤트.

        this->open_ports(); //서버포트 열기
        this->regist_servers_to_kq(); //감지목록에 서버들 추가.
        this->init_servers_map();
        this->init_status_map();
        kq_fd = kqueue();
        struct timespec timeout;
        timeout.tv_sec = 7;
        timeout.tv_nsec = 0;
        while ("soo-je-webserv")
        {
            #ifdef TEST
            std::cerr << "================ while start ===================== " << std::endl;
            #endif
            detected_count = kevent(kq_fd, &ev_cmds[0], ev_cmds.size(), detecteds, DETECT_SIZE, &timeout);

            #ifdef TEST
            std::cerr << "detect : " << detected_count << std::endl;
            #endif
            ev_cmds.clear(); //사용한 이벤트명령은 비운다.
            for (int i(0); i < detected_count; i++)
            {
                curr_det = &detecteds[i];
                if (curr_det->flags & EV_ERROR || curr_det->flags & EV_EOF)
                {
                    std::cerr << MAGENTA << "close : " << curr_det->ident << RESET << std::endl;
                    if (this->server_map.find(curr_det->ident) != this->server_map.end())
                    {
                        this->server_map.erase(this->server_map.find(curr_det->ident));
                        close(curr_det->ident);
                        continue;
                    }
                    if (this->client_map.find(curr_det->ident) != this->client_map.end())
                    {
                        this->client_map.erase(this->client_map.find(curr_det->ident));
                        close(curr_det->ident);
                        continue;
                    }
                    this->file_map.erase(this->file_map.find(curr_det->ident));
                    close(curr_det->ident);
                    continue;
                }
                if (curr_det->filter == EVFILT_READ) //감지된 이벤트가 "읽기가능"일 때.
                {
                    //감지된 fd가 서버일 때.
                    if (this->server_map.find(curr_det->ident) != this->server_map.end())
                    {
                        Server & s = this->server_map.find(curr_det->ident)->second;
                        #ifdef TEST
                        perror("read server");
                        #endif
                        int client_fd = s.accept_client();
                        if (client_fd == -1)
                            continue;
                        Client new_client(&(this->ev_cmds), &(this->file_map));
                        new_client.set_socket_fd(client_fd); //브라우저의 연결을 수락.
                        std::cerr << GREEN << "listen : " << new_client.get_socket_fd() << RESET << std::endl;
                        #ifdef TEST
                        // std::cerr << "listen : " << new_client.get_socket_fd() << std::endl;
                        #endif
                        new_client.set_status_msg(&(this->status_map));
                        #ifdef TEST
                        // std::cerr << "111 " << std::endl;
                        #endif
                        new_client.set_myserver(&(this->server_map[curr_det->ident])); //클라이언트클래스에서 서버클래스에 접근 할 수 있도록.
                        #ifdef TEST
                        // std::cerr << "222 " << std::endl;
                        #endif
                        add_kq_event(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE); //감지목록에 등록.
                        #ifdef TEST
                        // std::cerr << "333 " << std::endl;
                        #endif
                        this->client_map.insert(std::make_pair(client_fd, new_client));
                        #ifdef TEST
                        // std::cerr << "444 " << std::endl;
                        #endif
                        continue;
                    }
                    if (this->client_map.find(curr_det->ident) != this->client_map.end())
                    {
                        Client & c = this->client_map.find(curr_det->ident)->second;
                        #ifdef TEST
                        perror("read client");
                        #endif
                        int result = c.recv_data();
                        if (result == FAIL)
                        {
                            std::cerr << MAGENTA << "close : " << c.get_socket_fd() << RESET << std::endl;
                            close(c.get_socket_fd());
                            this->client_map.erase(this->client_map.find(curr_det->ident)); //kq에서 읽기가능이라고 했는데도 데이터를 읽을 수 없다면 삭제한다.
                        }
                        else if (result == RECV_ALL) //모두수신받았을 때.
                        {
                            #ifdef TEST
                            std::cerr << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
                            std::cerr << c.get_read_buf() << std::endl;
                            std::cerr << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
                            std::cerr << "aaaa" << std::endl;
                            #endif
                            if (c.get_response().get_status() == "" && c.parse_request() == false) //수신받은 request데이터 파싱. 실패시 에러응답준비.
                            {
                                add_kq_event(c.get_socket_fd(), EVFILT_READ, EV_DELETE | EV_DISABLE); //"읽기가능"감지 끄기.
                                #ifdef TEST
                                std::cerr << "bbbbbb" << std::endl;
                                #endif
                                c.ready_err_response_meta(); //에러응답 준비.
                                #ifdef TEST
                                std::cerr << "ccccccc" << std::endl;
                                #endif
                                continue;
                            }
                            if (c.chunk_done() == false && c.check_chunked() == CHUNKED)
                            {
                                #ifdef TEST
                                std::cerr << "#$#" << std::endl;
                                #endif
                                continue;
                            }
                            if (c.chunk_done() == false && c.check_bodysize() == LENGTHLESS)
                            {
                                #ifdef TEST
                                std::cerr << "@" << std::endl;
                                #endif
                                continue;
                            }
                            add_kq_event(c.get_socket_fd(), EVFILT_READ, EV_DELETE | EV_DISABLE); //"읽기가능"감지 끄기.
                            #ifdef TEST
                            std::cerr << "abababab" << std::endl;
                            #endif
                            #ifdef COOKIE
                            c.manage_session();
                            #endif
                            #ifdef TEST
                            std::cerr << "ddddddd" << std::endl;
                            #endif
                            if (c.get_response().get_status() == "400")
                            {
                                c.ready_err_response_meta(); //에러응답 준비.
                                continue;
                            }
                            c.init_client_location(); //경로가 로케이션 경로중에 해당하면 그 경로로 정보를 변경한다.
                            #ifdef TEST
                            std::cerr << "eeeeeee" << std::endl;
                            #endif
                            if (c.check_client_err() == true) //400번대 에러가 발생했는지 검사. 있다면 상태코드 설정.
                            {
                                #ifdef TEST
                                std::cerr << "fffffff" << std::endl;
                                #endif
                                c.ready_err_response_meta();
                                #ifdef TEST
                                std::cerr << "gggggg" << std::endl;
                                #endif
                                continue;
                            }
                            #ifdef TEST
                            std::cerr << "hhhhh" << std::endl;
                            #endif
                            if (c.check_redirect() == true) //리다이렉트 대상이라면
                                continue;
                            if (c.check_need_cgi() == false) //파싱된 데이터에 cgi요청이 없을 때.
                            {
                                #ifdef TEST
                                std::cerr << "iiiiii" << std::endl;
                                #endif
                                if (c.ready_response_meta() == false) //요청에 필요한 데이터 IO하기.
                                    c.ready_err_response_meta();
                                #ifdef TEST
                                std::cerr << "jjjjjjj" << std::endl;
                                #endif
                            }
                            else //cgi요청이 있을 때. (POST)
                            {
                                #ifdef TEST
                                std::cerr << "[!] CGI Mode Set" << std::endl;
                                #endif
                                c.set_cgi_mode(true); //cgi모드로 설정.
                                if (c.ready_body_file() == false)
                                    c.ready_err_response_meta();
                                #ifdef TEST
                                std::cerr << "[!] CGI finished" << std::endl;
                                #endif
                            }
                        }
                        continue;
                    }
                    //감지된 fd가 파일쪽 일 때.
                    if (this->file_map.find(curr_det->ident) != this->file_map.end())
                    {
                        Client & c = *this->file_map.find(curr_det->ident)->second;
                        #ifdef TEST
                        perror("read file");
                        #endif
                        if (c.is_cgi_mode() == true)
                        {
                            int cgi_status = c.check_cgi_status();
                            if (cgi_status == RUNNING || cgi_status == ERROR)
                                continue;
                        }
                        int result = c.read_file(); //클라이언트객체는 파일을 읽는다.
                        #ifdef TEST
                        perror("z1111111");
                        #endif
                        if (result == FAIL || result == RECV_ALL)
                        {
                            add_kq_event(c.get_socket_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓에 response 쓸 준비.
                            this->file_map.erase(this->file_map.find(c.get_file_fd())); 
                            c.set_file_fd(-1);
                        }
                        #ifdef TEST
                        perror("z222222");
                        #endif
                        if (result == FAIL) //파일 읽기 오류났을 때.
                            std::cerr << "close r file : " << c.get_file_fd() << ", own client : " << c.get_socket_fd() << std::endl;
                        else if (result == RECV_ALL) //모두수신받았을 때.
                            c.init_response(); //클라이언트는 응답 데이터를 제작한다.
                        #ifdef TEST
                        perror("z333333");
                        #endif
                        continue;
                    }
                }
                else if (curr_det->filter == EVFILT_WRITE) //감지된 이벤트가 "쓰기가능"일 때.
                {
                    #ifdef TEST
                    perror("write something");
                    #endif
                    if (this->client_map.find(curr_det->ident) != this->client_map.end())
                    {
                        Client & c = this->client_map.find(curr_det->ident)->second;
                        #ifdef TEST
                        perror("send socket 1");
                        #endif
                        if (c.get_file_fd() != -1) //아직 처리중인 파일이 있다면 송신하지 않는다.
                            continue;
                        int result = c.send_data(); //클라이언트 객체가 완성된 response데이터를 전송.
                        #ifdef TEST
                        perror("send socket 2");
                        #endif
                        if (result == SEND_ALL || result == FAIL)
                        {
                            unlink((".payload/cgi_ready_" + util::num_to_string(c.get_socket_fd())).c_str());
                            unlink((".payload/cgi_result_" + util::num_to_string(c.get_socket_fd())).c_str());
                        }
                        if (result == FAIL)
                        {
                            std::cerr << MAGENTA << "close : " << c.get_socket_fd() << RESET << std::endl;
                            close(c.get_socket_fd());
                            this->client_map.erase(this->client_map.find(curr_det->ident)); //이 클라이언트 소켓 제거.
                            #ifdef TEST
                            perror("send client fail");
                            #endif
                        }
                        else if (result == SEND_ALL)
                        {
                            add_kq_event(c.get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE); //다시 "읽기가능"감지목록에 등록.
                            add_kq_event(c.get_socket_fd(), EVFILT_WRITE,  EV_DELETE | EV_DISABLE); //"쓰기가능"감지목록에서 제외.
                            c.clear_client();//fd를 제외한 클라이언트 정보를 초기화한다.
                        }
                        continue;
                    }
                    //감지된 fd가 파일쪽 일 때. (PUT, POST(stdin)).
                    if (this->file_map.find(curr_det->ident) != this->file_map.end())
                    {
                        Client & c = *this->file_map.find(curr_det->ident)->second;
                        int result = c.write_file(); //클라이언트객체는 파일을 작성한다.
                        if (result == FAIL || result == SEND_ALL)
                        {
                            add_kq_event(c.get_socket_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓에 response 쓸 준비. (리스폰스제작과 연계)
                            this->file_map.erase(this->file_map.find(c.get_file_fd())); 
                            c.set_file_fd(-1);
                        }
                        if (result == FAIL) //파일 쓰기 오류났을 때.
                        {
                            std::cerr << "close w file : " << c.get_file_fd() << ", own client : " << c.get_socket_fd() << std::endl;
                            #ifdef TEST
                            perror("write file");
                            #endif
                        }
                        else if (result == SEND_ALL) //모두작성했을 때.
                        {
                            #ifdef TEST
                            perror("write all 1");
                            #endif
                            if (c.is_cgi_mode() == true)
                            {
                                #ifdef TEST
                                perror("write all 2");
                                #endif
                                add_kq_event(c.get_socket_fd(), EVFILT_WRITE,  EV_DELETE | EV_DISABLE);
                                if (c.excute_cgi() == false)
                                    c.ready_err_response_meta();
                                #ifdef TEST
                                perror("write all 3");
                                #endif
                                continue;
                            }
                            c.get_response().set_status("201");
                            c.init_response(); //업로드 완료 후 처리?... (kq와 연계)
                        }
                        continue;
                    }
                }
            }
            #ifdef TEST
            std::cerr << "================ while end ===================== " << std::endl;
            #endif
        }
    }
private:
    std::vector<Server> server_list;
    std::vector<struct kevent> ev_cmds; //kq 감지대상 벡터.
    std::map<int, Server> server_map; //서버 맵.
    std::map<int, Client> client_map; //클라이언트 맵.
    std::map<int, Client*> file_map; //파일 맵.
    std::map<std::string, std::string> status_map;
};

#endif