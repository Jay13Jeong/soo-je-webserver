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
#include "server.hpp"
#include "client.hpp"
#include <sys/socket.h> //socket
// #include <sys/un.h>
#include <sys/event.h> //kqueue
#include <stdexcept>
#include <sstream>
#include <map>
#include "util.hpp"

#define DETECT_SIZE 1024 //한 횟차에 처리할 최대 이벤트 갯수.
#define FAIL        -1 //실패를 의미하는 매크로.
#define RECV_ALL    1 //모두 수신 받음을 의미.
#define SEND_ALL    1 //모두 수신 받음을 의미.

class Webserv
{
private:
    std::vector<Server> _server_list;
    std::vector<Client> _client_list;
    std::vector<struct kevent> _ev_cmds; //kq 감지대상 벡터.
    std::map<int,Server> _server_map; //서버 맵.
    std::map<int,Client> _client_map; //클라이언트 맵.
    std::map<std::string, std::string> status_map;
 
public:
    Webserv(/* args */){};
    ~Webserv(){};
    std::vector<Server> get_server_list()
    {
        return this->_server_list;
    };
    void set_server_list(Server new_server)
    {
        this->_server_list.push_back(new_server);
    };
    std::vector<Client> get_client_list()
    {
        return this->_client_list;
    };
    void set_client_list(Client new_client)
    {
        this->_client_list.push_back(new_client);
    };

    /*
        - check bracket
        - 일단 정상적인 값이 들어온다는 전제
        - stack
        - 나올수 있는 예시들 listup하고서 find한다. (listen, root, index, location)
        - variable nameing rule check
            - start with _, alphabet
            - no start with digit
        - key값 중복 여부 확인 필요

    */
    bool    check_config_validation(std::ifstream &config_fd)
    {
        /*  체크사항
            노션 - 결정 사항 (11/16) 페이지에 정리해둠. 
        */
        std::string line;
        std::stack<std::string> bracket_stack;
        bool set = false;

        while (!config_fd.eof())
        {
            std::vector<std::string> split_result;
            getline(config_fd, line);
            int semicolon_cnt = util::count_semicolon(line);
            if (semicolon_cnt >= 2)
                return (false);
            // split_result.clear();
            split_result = util::ft_split(line, ";");
            if (split_result.size() == 0)
                continue;
            split_result = util::ft_split(split_result[0], "\t \n");
            if (split_result.size() == 0)
                continue;
            if (split_result[0] == "server")
            {
                set = true;
                // 가능한 유일한 입력 : server {
                if (semicolon_cnt != 0)
                    return (false);
                    
                if (split_result.size() != 2 || split_result[1] != "{")
                    return (false);
                bracket_stack.push("{"); // 여는 괄호를 넣어준다.
            }
            else if (split_result[0] == "listen")
            {
                // 가능한 입력 : listen 80; listen 70 ;
                if (set == false || semicolon_cnt != 1 || split_result.size() != 2) // 
                    return (false);
                if (util::is_numeric(split_result[1]) == false)
                    return (false);
            }
            else if (split_result[0] == "host")
            {
                // 가능한 입력 : host 127.0.0.1; host 127.0.0.1 ;
                if (set == false || semicolon_cnt != 1 || split_result.size() != 2) // 
                    return (false);
                if ((split_result[1] != "127.0.0.1"))
                    return (false);
            }
            else if (split_result[0] == "server_name")
            {
                // 가능한 입력 : server_name lalala; server_name lalala ;
                if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                    return (false);
            }
            else if (split_result[0] == "root")
            {
                // 가능한 입력 : root path; root path ;
                if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                    return (false);
            }
            else if (split_result[0] == "index")
            {
                if (set == false || semicolon_cnt != 1 || split_result.size() < 2)
                    return (false);
            }
            else if (split_result[0] == "default_error_pages")
            {
                if (set == false || semicolon_cnt != 1 || split_result.size() != 3)
                    return (false);
                if (util::is_numeric(split_result[1]) == false)
                    return (false);
            }
            else if (split_result[0] == "autoindex")
            {
                if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                    return (false);
                if (split_result[1] != "on" && split_result[1] != "off")
                    return (false);
            }
            else if (split_result[0] == "client_max_body_size")
            {
                if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                    return (false);
                if (util::is_numeric(split_result[1]) == false)
                    return (false);
            }
            else if (split_result[0] == "cgi")
            {
                if (set == false || semicolon_cnt != 1 || split_result.size() != 3)
                    return (false);
                if (split_result[1] != "none" && split_result[1] != ".py" && split_result[1] != ".php")
                    return (false);
            }
            else if (split_result[0] == "location")
            {
                if (set == false || semicolon_cnt != 0 || split_result.size() != 3)
                    return (false);
                if (split_result[2] != "{")
                    return (false);
                bracket_stack.push("{");
                // 계속 읽어준다.
                while (!config_fd.eof())
                {
                    getline(config_fd, line);
                    semicolon_cnt = util::count_semicolon(line);
                    if (semicolon_cnt > 1)
                        return (false);
                    split_result = util::ft_split(line, ";");
                    if (split_result.empty())
                        continue;
                    split_result = util::ft_split(split_result[0], "\t \n");
                    if (split_result[0] == "}")
                    {
                        if (semicolon_cnt != 0)
                            return (false);
                        if (bracket_stack.empty())
                            return (false);
                        bracket_stack.pop();
                        break;
                    }

                    // root, autoindex - key, value;
                    if (split_result[0] == "root" || split_result[0] == "autoindex")
                    {
                        if (split_result.size() != 2)
                            return (false);
                        if (split_result[0] == "autoindex" && split_result[1] != "on" && split_result[1] != "off")
                            return (false);
                    }
                    // return - key statuscode url
                    else if (split_result[0] == "return")
                    {
                        if (split_result.size() != 3)
                            return (false);
                        if (util::is_numeric(split_result[1]) == false)
                            return (false);
                    }
                    // index, accept_method - key value ...
                    else if (split_result[0] == "index" || split_result[0] == "accept_method")
                    {
                        if (split_result.size() == 1)
                            return (false); 
                    }
                }
            }
            else if (split_result[0] == "}")
            {
                if (set == false || semicolon_cnt != 0)
                    return (false);
                if (bracket_stack.empty() == true)
                    return (false);
                bracket_stack.pop();
            }
            else 
            {
                // error
                return (false);
            }
        }
        if (!bracket_stack.empty())
            return (false);
        return (true);
    }

    //conf파일을 인자를 받아 파싱 메소드.
    bool    parsing(std::string conf_file)
    {
        std::ifstream config_fd;
    	std::string line;

    	config_fd.open(conf_file);
        // ifstream 실패했을 경우 -> good() false
        if (config_fd.good() == false)
             return (false);
        if (check_config_validation(config_fd) == false)
        {
            config_fd.close();
             return (false);
        }
        config_fd.close();
        // 유효성 확인된 경우 파싱 시작
        config_fd.open(conf_file);
	    while (!config_fd.eof())
	    {
		    std::vector<std::string> split_result;
            getline(config_fd, line);
            split_result = util::ft_split(line, "\t \n");
            if (split_result.size() == 0)
                continue;
		    //함수 호출, 정규표현식은 없다.
		    //첫줄이 server 로 시작하면 server 리스트에 1개 추가
            if (split_result[0] == "server")
            {
                Server new_server;
                set_server_list(new_server);
            }
            else if (split_result[0] == "listen")
            {
                util::remove_last_semicolon(split_result[1]);
                int port = util::string_to_num<int>(split_result[1]);
                for(int i = 0; i < _server_list.size() - 1; i++)
                {
                    if (_server_list[i].port == port)
                    {
                        return (false);
                    }
                }
                 _server_list.back().port = port;
            }
            else if (split_result[0] == "server_name")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back().server_name = split_result[1];
            }
            else if (split_result[0] == "root")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back().root = split_result[1];
            }
            else if (split_result[0] == "index")
            {
                _server_list.back().get_index().clear();
                for (int i = 1; i < split_result.size(); i++)
                {
                    if (split_result[i] == ";")
                        break;
                    util::remove_last_semicolon(split_result[i]);
                    _server_list.back().index.push_back(split_result[i]);
                }
            }
            else if (split_result[0] == "autoindex")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back().autoindex = (split_result[1] == "on");
            }
            else if (split_result[0] == "client_max_body_size")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back().client_max_body_size = util::string_to_num<size_t>(split_result[1]);
            }
            else if (split_result[0] == "error_page")
            {
                util::remove_last_semicolon(split_result[1]);
                util::remove_last_semicolon(split_result[2]);
                _server_list.back().default_error_pages.insert(std::make_pair(split_result[1], split_result[2]));
            }
            else if (split_result[0] == "cgi")
            {
                if (split_result.size() >= 3 && (split_result[1] == ".py" || split_result[1] == ".php"  || split_result[1] == "none"))
                {
                    _server_list.back().cgi_map.insert(std::make_pair(split_result[1],split_result[2]));
                }
            }
            else if (split_result[0] == "location")
            {
                Location loc_temp(get_server_list().back().get_root(), \
                    get_server_list().back().get_index(), \
                    get_server_list().back().get_autoindex());
                //l의 기본값 넣는 부분 추가할 것
                loc_temp.path = split_result[1];
                //loc_temp.root = split_result[1];
                while (1)
                {
                    getline(config_fd, line);
                    util::remove_last_semicolon(line);
                    split_result = util::ft_split(line, "\t ");
                    if (split_result.empty())
                        continue;
                    if (split_result[0] == "}"){
                        _server_list.back().loc.push_back(loc_temp);
                        break ;
                    }
                    else if (split_result[0] == "return")
                    {
                        if (split_result.size() == 3)
                            loc_temp.redirection.insert(std::make_pair(split_result[1], split_result[2]));
                        else
                            ;//예외처리하기
                    }
                    else if (split_result[0] == "accept_method")
                    {
                        int i = 1;
                        loc_temp.accept_method.clear();
                        while (i < split_result.size())
                        {
                            if (split_result[i] == "GET" || split_result[i] == "POST" || split_result[i] == "DELETE")
                                loc_temp.accept_method.push_back(split_result[i]);
                            else
                                ;// 예외처리
                            i++;
                        }
                    }
                    else if (split_result[0] == "index")
                    {
                        loc_temp.index.clear();
                        int i = 1;
                        while (i < split_result.size())
                        {
                            loc_temp.index.push_back(split_result[i]);
                            i++;
                        }
                    }
                    else if (split_result[0] == "autoindex")
                    {
                        util::remove_last_semicolon(split_result[1]);
                        loc_temp.autoindex = (split_result[1] == "on");
                    }
                    else if (split_result[0] == "root")
                    {
                        util::remove_last_semicolon(split_result[1]);
                        loc_temp.root = split_result[1];
                    }
                }
            }
            else if (split_result[0] == "}" || split_result[0] == "host")
                continue;
            else
            {
                // error
            }

		//내용물 채우고 마지막 } 만나면 나오기
		//함수호출 끝
    	}
        config_fd.close();
        return (true);
    }

    //서버포트들을 개방하는 메소드.
    void open_ports()
    {
        for (int i(0);i < this->get_server_list().size();i++)
        {
            this->get_server_list()[i].open_port();
        }
    }

    //fd와 "감지할 행동", kevent지시문을 인자로 받아서 감지목록에 추가하는 메소드.
    void add_kq_event(uintptr_t ident, int16_t filter, uint16_t flags)
    {
        struct kevent new_event;

        EV_SET(&new_event, ident, filter, flags, 0, 0, NULL);
        this->_ev_cmds.push_back(new_event);
    }
    
    //서버들을 감지목록에 추가하는 메소드.
    void regist_servers_to_kq()
    {
        for (int i(0);i < this->get_server_list().size();i++)
        {
            add_kq_event(this->get_server_list()[i].fd, EVFILT_READ, EV_ADD | EV_ENABLE);
        }
    }

    void init_servers_map()
    {
        for (int i(0);i < this->get_server_list().size();i++)
        {
            this->_server_map[this->_server_list[i].fd] = this->_server_list[i];
            this->_server_list[i].init_location_map(); //로케이션 맵도 같이 초기화.
            this->_server_list[i].init_default_location(); //로케이션 초기화대상이없으면 하나 만들어주기.
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
        status_map.insert(std::make_pair("503","Service Unavailable"));
        status_map.insert(std::make_pair("504","Gateway Timeout"));
        status_map.insert(std::make_pair("505","HTTP Version Not Supported"));
    }

    //서버를 실행하는 메소드.
    void start()
    {
        int kq_fd; //커널큐 fd.
        int detected_count; //감지된 이벤트 갯수.
        std::vector<struct kevent> detecteds; //감지 된 이벤트벡터.
        detecteds.reserve(DETECT_SIZE); //오버헤드 방지.

        // struct kevent k_set;
        /**
        EV_SET(이벤트구조체 &k_set,
            감지할fd,
            감지되면 설정될 플래그 EVFILT_READ 또는 EVFILT_WRITE,
            "이벤트추가" 및 "이벤트반환"매크로 EV_ADD | EV_ENABLE,
            필터플레그값 0,
            필터 데이터값 0,
            사용자 정의 데이터 NULL);
        **/

        this->open_ports(); //서버포트 열기
        this->regist_servers_to_kq(); //감지목록에 서버들 추가.
        this->init_servers_map();
        this->init_status_map();
        kq_fd = kqueue();
        while ("soo-je-webserv")
        {
            try
            {
                detected_count = kevent(kq_fd, &_ev_cmds[0], _ev_cmds.size(), &detecteds[0], DETECT_SIZE, NULL);
                _ev_cmds.clear(); //사용한 이벤트명령은 비운다.
                for (int i(0); i < detected_count; i++)
                {
                    if (detecteds[i].flags & EVFILT_READ) //감지된 이벤트가 "읽기가능"일 때.
                    {
                        //감지된 fd가 정규파일인지 서버인지 클라이언트꺼인지 검사한다.
                        bool used = false; //찾았는지 여부.
                        for (int j(0); j < get_server_list().size(); j++)
                        {   //감지된 fd가 서버쪽 일 때.
                            if (detecteds[i].ident == get_server_list()[j].fd)
                            {
                                Client new_client = Client(&this->_ev_cmds);
                                new_client.setSocket_fd(get_server_list()[j].accept_client()); //브라우저의 연결을 수락.
                                new_client.set_status_msg(&(this->status_map));
                                new_client.set_myserver(&(this->_server_map[detecteds[i].ident])); //클라이언트클래스에서 서버클래스에 접근 할 수 있도록.
                                //감지목록에 등록.
                                add_kq_event(new_client.getSocket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE);
                                this->set_client_list(new_client); //클라이언트리스트에도 추가.
                                used = true;
                                break;
                            }
                        }
                        if (used == true)
                            continue;
                        for (std::vector<Client>::iterator it = _client_list.begin(); it != get_client_list().end(); it++)
                        {   //감지된 fd가 클라쪽 일 때.
                            if (detecteds[i].ident == (*it).getSocket_fd())
                            {
                                used = true;
                                int result = (*it).recv_data();
                                if (result == FAIL)
                                {
                                    perror("read client");
                                    this->_client_list.erase(it); //kq에서 읽기가능이라고 했는데도 데이터를 읽을 수 없다면 삭제한다.
                                }
                                else if (result == RECV_ALL) //모두수신받았을 때.
                                {
                                    add_kq_event((*it).getSocket_fd(), EVFILT_READ, EV_DELETE | EV_DISABLE); //"읽기가능"감지 끄기.
                                    if ((*it).parse_request() == false) //수신받은 request데이터 파싱. 실패시 에러응답준비.
                                    {
                                        (*it).ready_err_response_meta(); //에러응답 준비.
                                        break;
                                    }
                                    (*it).init_client_location(); //경로가 로케이션 경로중에 해당하면 그 경로로 정보를 변경한다.
                                    if ((*it).check_client_err() == true) //400번대 에러가 발생했는지 검사. 있다면 상태코드 설정.
                                    {
                                        (*it).ready_err_response_meta(); //에러응답 준비.
                                        break;
                                    }
                                    if ((*it).check_need_cgi() == false) //파싱된 데이터에 cgi요청이 없을 때.
                                    {
                                        if ((*it).ready_response_meta() == false) //요청에 필요한 데이터 IO하기.
                                            (*it).ready_err_response_meta();
                                    }
                                    else //cgi요청이 있을 때. (POST)
                                    {
                                        (*it).setCgi_mode(true); //cgi모드로 설정.
                                        (*it).excute_cgi(); //fork로 보내고 파생된result파일을 읽도록 kq에 등록.
                                    }
                                }
                                break;
                            }
                        }
                        if (used == true)
                            continue;
                        //감지된 fd가 파일쪽 일 때.
                        for (std::vector<Client>::iterator it = _client_list.begin();it != get_client_list().end();it++)
                        {   //먼저 어떤 클라이언트의 파일인지 찾는다.
                            if (detecteds[i].ident == (*it).getFile_fd())
                            {
                                int result = (*it).read_file(); //클라이언트객체는 파일을 읽는다.
                                if (result == FAIL || result == RECV_ALL)
                                    add_kq_event((*it).getSocket_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓에 response 쓸 준비.
                                if (result == FAIL) //파일 읽기 오류났을 때.
                                    perror("read file");
                                else if (result == RECV_ALL) //모두수신받았을 때.
                                    (*it).init_response(this->_server_map); //클라이언트는 응답 데이터를 제작한다.
                                break;
                            }
                        }
                    }
                    else if (detecteds[i].flags & EVFILT_WRITE) //감지된 이벤트가 "쓰기가능"일 때.
                    {
                        bool used = false; //찾았는지 여부.
                        for (std::vector<Client>::iterator it = _client_list.begin(); it != get_client_list().end(); it++)
                        {   //감지된 fd가 클라쪽 일 때.
                            if (detecteds[i].ident == (*it).getSocket_fd())
                            {
                                used = true;
                                if ((*it).getFile_fd() != -1) //아직 처리중인 파일이 있다면 송신하지 않는다.
                                    break;
                                int result = (*it).send_data(); //클라이언트 객체가 완성된 response데이터를 전송.
                                if (result == FAIL)
                                {
                                    _client_list.erase(it); //이 클라이언트 소켓 제거.
                                    perror("send client");
                                }
                                else if (result == SEND_ALL)
                                {
                                    add_kq_event((*it).getSocket_fd(), EVFILT_WRITE, EV_DELETE | EV_DISABLE); //"쓰기가능"감지목록에서 제외.
                                    add_kq_event((*it).getSocket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE); //다시 "읽기가능"감지목록에 등록.
                                    (*it).clear_client();//fd를 제외한 클라이언트 정보를 초기화한다.
                                }
                                break;
                            }
                        }
                        if (used == true)
                            continue;
                        //감지된 fd가 파일쪽 일 때. (PUT).
                        for (std::vector<Client>::iterator it = _client_list.begin();it != get_client_list().end();it++)
                        {   //먼저 어떤 클라이언트의 파일인지 찾는다.
                            if (detecteds[i].ident == (*it).getFile_fd())
                            {
                                int result = (*it).write_file(); //클라이언트객체는 파일을 작성한다.
                                if (result == FAIL || result == SEND_ALL)
                                    add_kq_event((*it).getSocket_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE); //소켓에 response 쓸 준비. (리스폰스제작과 연계)
                                if (result == FAIL) //파일 쓰기 오류났을 때.
                                    perror("write file");
                                else if (result == SEND_ALL) //모두작성했을 때.
                                    (*it).init_response(this->_server_map); //업로드 완료 후 처리?... (kq와 연계)
                                break;
                            }
                        }
                    } 
                    else { /* error... */ }
                }
            }
            catch(const std::exception& e)
            {
                close(kq_fd);
                kq_fd = kqueue();
                this->_client_list.clear();
                this->_client_map.clear();
                this->_ev_cmds.clear();
                this->regist_servers_to_kq();
                std::cerr << e.what() << '\n';
            }
        }
    }
};

#endif