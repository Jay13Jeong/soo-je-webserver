#include <vector>
#include <iostream>
#include <fcntl.h>
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

#define DETECT_SIZE 1024; //한 횟차에 처리할 최대 이벤트 갯수.

class Webserv
{
private:
    std::vector<Server*> _server_list;
    std::vector<Client*> _client_list;

public:
    webserv(/* args */);
    ~webserv();
    std::vector<Server*> get_server_list()
    {
        return this->_server_list;
    };
    void set_server_list(Server *new_server)
    {
        this->_server_list.push_back(new_server);
    };
    std::vector<Client*> get_client_list()
    {
        return this->_client_list;
    };
    void set_client_list(Client *new_client)
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
    bool    check_config_validation(ifstream config_fd)
    {
        std::string line;
        stack<string> bracket_stack;

        while (!config_fd.eof())
        {
            getline(config_fd, line);
            // check 조건 추가 bracket, variable naming condition
        }
        return (true);
    }

    //conf파일을 인자를 받아 파싱 메소드.
    bool    parsing(std::string conf_file)
    {
        ifstream    config_fd;
    	std::string line;
        std::string options[5] = {"server", "listen", "root", "index", "location"};

    	config_fd = open(argv[2], O_RDONLY);
        // ifstream 실패했을 경우 -> good() false
        if (config_fd.good() == false)
            return (false);
        if (check_config_validation(config_fd) == false)
        {
            close(config_fd);
            return (false)
        }
        close(config_fd);
        // 유효성 확인된 경우 파싱 시작
        config_fd = open(argv[2], O_RDONLY);

	    while (!config_fd.eof())
	    {
		    getline(config_fd, line);
		    //함수 호출, 정규표현식은 없다.
		    //첫줄이 server 로 시작하면 server 리스트에 1개 추가
            if (std::find(line.begin(), line.end(), "server") == line.begin())
            {
                Server *new_server = new Server(); // 메모리 해제 신경써줘야함;;;
                set_server_list(new_server);
            }
			/*
                중괄호 숫자 +-세기, 0이면 함수 나오기
				한줄씩 읽어서 공백기준으로 나누기 -> ft_split 만들기.
                루트는 인자가 1개, but index의 인자가 여러개이면?,fastcgi는? -> 일단은 list로 처리
                각각의 부분에 대한 예외처리를 함수를 추가로 불러서 하도록 하자. 특히 location
			*/
		//내용물 채우고 마지막 } 만나면 나오기
		//함수호출 끝
    	}
        close(config_fd);
        return (true);
    }

    //서버를 실행하는 메소드.
    void start()
    {
        int kq_fd; //커널큐 fd.
        int detected_count; //감지된 이벤트 갯수.
        std::vector<struct kevent> detecteds; //감지 된 이벤트벡터.
        detecteds.resize(DETECT_SIZE); //오버헤드 방지.

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

        kq_fd = kqueue();
        while ("soo-je-webserv")
        {
            try
            {
                detected_count = kevent(kq_fd, g_detects, g_detects.size(), detecteds, DETECT_SIZE, NULL);
                for (int i(0); i < detected_count; i++)
                {
                    if (detecteds[i].flags & EVFILT_READ) //감지된 이벤트가 "읽기가능"일 때.
                    {
                        if (detecteds[i].ident == "server_fd") //감지된 fd가 서버쪽 일 때.(map활용예정)
                        {
                            //accept하고 나온 클라이언트 소켓으로 클라이언트 객체 생성.
                            //fcntl(socket_fd, F_SETFL, O_NONBLOCK); //NON-BLOCKING설정 (이외에 파일FD에도 해준다.)
                            
                            //클라이언트 객체에서 읽기,쓰기 모두 감지하도록 g_detects에 초기화해서 추가.
                            EV_SET(g_detects, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); //클라이언트객체로
                            EV_SET(g_detects, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); //클라이언트객체로
                            
                        }
                        else if (detecteds[i].ident == "client_fd") //감지된 fd가 클라쪽 일 때.(map활용예정)
                        {
                            /*클라이언트 객체로 이관예정*/     
                            char buf[1024];
                            int n = recv(detecteds[i].ident, buf, sizeof(buf)); //소켓데이터를 버퍼만큼 읽는다.

                            if (n <= 0)
                            {
                                //데이터를 읽을 수 없다면 브라우저와 socket연결을 close하고 객체도 지운다.
                                //감지목록에서도 지운다.
                            }
                            else
                            {
                                //"client 객체에 recv한 데이터 저장" += std::string(buf, n);
                                //**클라객체에서 추가적으로 수신 완료여부 검사 필요.
                            }
                        }
                        else if (detecteds[i].ident == "file_fd") //감지된 fd가 파일쪽 일 때.(map활용예정)
                        {
                            //파일 구조체에 담긴 정보로 파일을 읽는다.
                            //**파일 구조체: 클라이언트FD, 파일FD, 경로 및 파일이름, 읽기완료 여부, 쓰기완료 여부.
                            //읽기가 완료되면 주인 클라이언트객체에게 알린다.
                        }
                    }
                    else if (detecteds[i].flags & EVFILT_WRITE) //감지된 이벤트가 "쓰기가능"일 때.
                    {
                        if (detecteds[i].ident == "client_fd") //감지된 fd가 서버쪽 일 때.(map활용예정)
                        {
                            //클라이언트 객체가 완성된 response데이터를 전송.
                            //**버퍼를 사용해서 BLOCK되지 않도록 한다. (나눠보내기)
                        }
                        else if (detecteds[i].ident == "file_fd") //감지된 fd가 파일쪽 일 때.(map활용예정)
                        {
                            //파일 구조체에 담긴 정보로 파일을 작성한다.
                            //쓰기가 완료되면 주인 클라이언트객체에게 알린다.
                        }
                    }
                    else
                    {
                        //error
                    }
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        
    }



};
