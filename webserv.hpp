#include <vector>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stack>
#include <unistd.h>
#include <algorithm>
#include "Server.hpp"
// #include "Client.hpp"
#include <sys/socket.h> //socket
// #include <sys/un.h>
#include <sys/event.h> //kqueue
#include <stdexcept>
#include <sstream>
#include "util.hpp"

#define DETECT_SIZE 1024; //한 횟차에 처리할 최대 이벤트 갯수.
#define FAIL        -1; //실패를 의미하는 매크로.

class Webserv
{
private:
    std::vector<Server*> _server_list;
    // std::vector<Client*> _client_list;

public:
    Webserv(/* args */);
    ~Webserv();
    std::vector<Server*> get_server_list()
    {
        return this->_server_list;
    };
    void set_server_list(Server *new_server)
    {
        this->_server_list.push_back(new_server);
    };
    // std::vector<Client*> get_client_list()
    // {
    //     return this->_client_list;
    // };
    // void set_client_list(Client *new_client)
    // {
    //     this->_client_list.push_back(new_client);
    // };

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
    // bool    check_config_validation(ifstream config_fd)
    // {
    //     std::string line;
    //     stack<string> bracket_stack;

    //     while (!config_fd.eof())
    //     {
    //         getline(config_fd, line);
    //         // check 조건 추가 bracket, variable naming condition
    //     }
    //     return (true);
    // }

    //conf파일을 인자를 받아 파싱 메소드.
    bool    parsing(std::string conf_file)
    {
        std::ifstream config_fd;
    	std::string line;

    	// config_fd.open(conf_file);
        // // ifstream 실패했을 경우 -> good() false
        // if (config_fd.good() == false)
        //     return (false);
        // if (check_config_validation(config_fd) == false)
        // {
        //     close(config_fd);
        //     return (false)
        // }
        // config_fd.close();
        // 유효성 확인된 경우 파싱 시작
        config_fd.open(conf_file);

	    while (!config_fd.eof())
	    {
		    std::vector<std::string> split_result;
            std::stringstream ss;
            getline(config_fd, line);
            split_result = util::ft_split(line, "\t \n");
            if (split_result.size() == 0)
                continue;
		    //함수 호출, 정규표현식은 없다.
		    //첫줄이 server 로 시작하면 server 리스트에 1개 추가
            if (split_result[0] == "server")
            {
                Server *new_server = new Server(); // 메모리 해제 신경써줘야함;;;
                set_server_list(new_server);
            }
            else if (split_result[0] == "listen")
            {
                util::remove_last_semicolon(split_result[1]);
                ss.str(split_result[1]);
                int port;
                ss >> port; // 이 경우 a80 은 0으로, 80a는 80으로 파싱됨 -> 유효성 검사 부분에서 처리 필요함.
                for(int i = 0; i < _server_list.size(); i++)
                {
                    if (_server_list[i]->port == port)
                    {
                        //error
                    }
                }                
                 _server_list.back()->port = port;
            }
            else if (split_result[0] == "server_name")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back()->server_name = split_result[1];
            }
            else if (split_result[0] == "root")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back()->root = split_result[1];
            }
            else if (split_result[0] == "index")
            {
                for (int i = 1; i < split_result.size(); i++)
                {
                    if (split_result[i] == ";")
                        break;
                    util::remove_last_semicolon(split_result[i]);
                    _server_list.back()->index.push_back(split_result[i]);
                }
            }
            else if (split_result[0] == "autoindex")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back()->autoindex = (split_result[1] == "on");
            }
            else if (split_result[0] == "client_max_body_size")
            {
                util::remove_last_semicolon(split_result[1]);
                ss.str(split_result[1]);
                ss >> _server_list.back()->client_max_body_size;
            }
            else if (split_result[0] == "error_page")
            {
                util::remove_last_semicolon(split_result[1]);
                _server_list.back()->default_error_page = split_result[1];
            }
            else if (split_result[0] == "cgi")
            {
                if (split_result[1] == "py" || split_result[1] == "php")
                {
                    if (split_result[1] == "py")
                        _server_list.back()->cgi_map.insert(std::make_pair("py","python"));
                    else if (split_result[1] == "php")
                        _server_list.back()->cgi_map.insert(std::make_pair("php","php"));
                }
                if (split_result.size() >= 3 && (split_result[2] == "python" || split_result[2] == "php"))
                {
                    if (split_result[2] == "py")
                        _server_list.back()->cgi_map.insert(std::make_pair("py","python"));
                    else if (split_result[2] == "php")
                        _server_list.back()->cgi_map.insert(std::make_pair("php","php"));
                }
            }
            else if (split_result[0] == "location")
            {
                Location loc_temp;
                //l의 기본값 넣는 부분 추가할 것
                loc_temp.root = split_result[1];
                while (1)
                {
                    getline(config_fd, line);
                    split_result = util::ft_split(line, "\t ");
                    if (split_result[0] == "}"){
                        _server_list.back()->loc.push_back(loc_temp);
                        break ;
                    }
                    else if (split_result[0] == "return")
                    {//배열 크기가 3이 아닌 2일때 예외처리 할 것
                        loc_temp.redirection = split_result[1];
                        loc_temp.index = split_result[2];
                    }
                    else if (split_result[0] == "autoindex")
                    {
                        util::remove_last_semicolon(split_result[1]);
                        loc_temp.autoindex = (split_result[1] == "on");
                    }
                }
            }
            else if (split_result[0] == "{" || split_result[0] == "}")
                continue; // 논의 필요...
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

    //서버를 실행하는 메소드.
    // void start()
    // {
    //     int kq_fd; //커널큐 fd.
    //     int detected_count; //감지된 이벤트 갯수.
    //     std::vector<struct kevent> detecteds; //감지 된 이벤트벡터.
    //     detecteds.resize(DETECT_SIZE); //오버헤드 방지.

    //     // struct kevent k_set;
    //     /**
    //     EV_SET(이벤트구조체 &k_set,
    //         감지할fd,
    //         감지되면 설정될 플래그 EVFILT_READ 또는 EVFILT_WRITE,
    //         "이벤트추가" 및 "이벤트반환"매크로 EV_ADD | EV_ENABLE,
    //         필터플레그값 0,
    //         필터 데이터값 0,
    //         사용자 정의 데이터 NULL);
    //     **/

    //     kq_fd = kqueue();
    //     while ("soo-je-webserv")
    //     {
    //         try
    //         {
    //             detected_count = kevent(kq_fd, g_detects, g_detects.size(), detecteds, DETECT_SIZE, NULL);
    //             for (int i(0); i < detected_count; i++)
    //             {
    //                 if (detecteds[i].flags & EVFILT_READ) //감지된 이벤트가 "읽기가능"일 때.
    //                 {
    //                     //감지된 fd가 정규파일인지 서버인지 클라이언트꺼인지 검사한다.
    //                     bool used = false; //찾았는지 여부.
    //                     for (int j(0); j < get_server_list().size(); j++)
    //                     {   //감지된 fd가 서버쪽 일 때.
    //                         if (detecteds[i].ident == get_server_list()[j]->fd)
    //                         {
    //                             Client *new_client = new Client();
    //                             new_client->setSocket_fd(get_server_list()[j]->accept_client()); //브라우저의 연결을 수락.
    //                             new_client->regist_fd_to_detects(); //감지목록에 등록.
    //                             this->set_client_list(new_client);
    //                             used = true;
    //                             break;
    //                         }
    //                     }
    //                     if (used == true)
    //                         continue;
    //                     for (std get_client_list().begin())
    //                     for (int j(0); j < get_client_list().size(); j++) 
    //                     {   //감지된 fd가 클라쪽 일 때.(map활용예정)
    //                         if (detecteds[i].ident == get_client_list()[j]->fd)
    //                         {
    //                             ////////////////////////
    //                             /*클라이언트 객체로 이관예정*/
    //                             if (get_client_list()[j]->recv_data() == FAIL)
    //                             {
    //                                 this->get_client_list()
    //                             }
    //                             ////////////
    //                             char buf[1024];
    //                             int n = recv(detecteds[i].ident, buf, sizeof(buf)); //소켓데이터를 버퍼만큼 읽는다.
    //                             if (n <= 0)
    //                             {
    //                                 //데이터를 읽을 수 없다면 브라우저와 socket연결을 close하고 객체도 지운다.
    //                                 //감지목록에서도 지운다.
    //                             }
    //                             else
    //                             {
    //                                 //"client 객체에 recv한 데이터 저장" += std::string(buf, n);
    //                                 //**클라객체에서 추가적으로 수신 완료여부 검사 필요.
    //                             }
    //                             ////////////////////////
    //                             used = true
    //                             break;
    //                         }
    //                     }
    //                     if (used == true)
    //                         continue;
    //                     else //감지된 fd가 파일쪽 일 때.(map활용예정)
    //                     {
    //                         //파일 구조체에 담긴 정보로 파일을 읽는다.
    //                         //**파일 구조체: 클라이언트FD, 파일FD, 경로 및 파일이름, 읽기완료 여부, 쓰기완료 여부.
    //                         //읽기가 완료되면 주인 클라이언트객체에게 알린다.
    //                     }
    //                 }
    //                 else if (detecteds[i].flags & EVFILT_WRITE) //감지된 이벤트가 "쓰기가능"일 때.
    //                 {
    //                     if (detecteds[i].ident == "client_fd") //감지된 fd가 서버쪽 일 때.(map활용예정)
    //                     {
    //                         //클라이언트 객체가 완성된 response데이터를 전송.
    //                         //**버퍼를 사용해서 BLOCK되지 않도록 한다. (나눠보내기)
    //                     }
    //                     else if (detecteds[i].ident == "file_fd") //감지된 fd가 파일쪽 일 때.(map활용예정)
    //                     {
    //                         //파일 구조체에 담긴 정보로 파일을 작성한다.
    //                         //쓰기가 완료되면 주인 클라이언트객체에게 알린다.
    //                     }
    //                 }
    //                 else
    //                 {
    //                     //error
    //                 }
    //             }
    //         }
    //         catch(const std::exception& e)
    //         {
    //             std::cerr << e.what() << '\n';
    //         }
    //     }
    // }
};
