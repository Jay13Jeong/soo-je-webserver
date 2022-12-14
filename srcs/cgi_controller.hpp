#ifndef CGI_CONTROLLER_HPP
# define CGI_CONTROLLER_HPP

#include <string>
#include <map>
#include <sys/wait.h> // waitpid
#include <sys/event.h>	// kqueue
#include <fcntl.h>	// open
#include <sys/socket.h> // getsockname
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <limits.h>
#include "util.hpp"


// CGI_STATUS
#define END 1
#define NONE 0
#define ERROR -1
#define RUNNING 2

class CgiController
{
	public:
		// Constructor, Destructor
		CgiController()
		{
			this->status = NONE;
		}

		~CgiController()
		{
		}
		// getter
		int get_status(void)
		{
			if (this->status == RUNNING)
			{
				int status, ret;
				ret = waitpid(pid, &status, WNOHANG);
				if (ret == pid)
					this->status = END;
				else if (ret == -1)	// status code 500 설정 필요.
					this->status = ERROR;
			}
			return (this->status);
		}
		int get_file_fd(void)
		{
			return (this->file_fd);
		}
		// execute cgi
		// void initialize(std::string program, std::string script, Client &client);
		void initialize(std::string program, std::string script, std::string target, std::string root, std::map<std::string, std::string> &headers, std::string req_method, int sock_fd, std::string host, int port, std::string version, long sid)
		{
			this->program = program;
			this->script = script;
			make_target_info(target, root);
			make_req_info(headers, req_method, sock_fd, host, port, version, sid);
		}
		bool ready_body_file(void)
		{
			this->body_file = ".payload/cgi_ready_" + util::num_to_string(this);
			if (util::make_middle_pathes(this->body_file) == false)
				return false;
			if ((this->file_fd = open(this->body_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
				return false;
			return true;
		}
		bool execute(void)
		{
			int stdin_fd, result_fd;

			this->result_file = ".payload/cgi_result_" + util::num_to_string(this);
			// 필요한 파일들 open
			stdin_fd = open(this->body_file.c_str(), O_RDONLY, 0644);
			result_fd = open(this->result_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			this->file_fd = open(this->result_file.c_str(), O_RDONLY, 0644);
			if (stdin_fd == -1 || this->file_fd == -1 || this->file_fd == -1)//읽기전용.
			{
				this->status = ERROR;
				return (false);
			}
			int cgi_pid = -1;
			if ((cgi_pid = fork()) < 0)
			{
				this->status = ERROR;
				return (false);
			}
			if (cgi_pid == 0)	// child
			{
				close(this->file_fd);
				std::string file_realpath = make_realpath();
				char **env = init_env();
				dup2(stdin_fd, 0); //입력 리다이렉트.
				dup2(result_fd, 1); //출력 리다이렉트.
				char **arg = new char *[sizeof(char *) * 3];
				arg[0] = strdup(this->program.c_str()); //예시 "/usr/bin/python"
				arg[1] = strdup(file_realpath.c_str()); //실행할 파일의 절대경로.
				arg[2] = NULL;
				if (execve(arg[0], arg, env) == -1) //cgi 실행.
				{
					perror("execve cgi fail");
					close(result_fd);
					result_fd = open(this->result_file.c_str(), O_WRONLY | O_TRUNC, 0644);
					dup2(result_fd, 1);
					std::cout << "Content-type:text/html\r\n\r\nCGI ERROR";
					exit(1);
				}
				exit(0);
			}
			else				// parent
			{
				this->pid = cgi_pid;
				this->status = RUNNING;
				close(result_fd);
				close(stdin_fd);
				unlink(this->body_file.c_str());
				int status, ret;
				ret = waitpid(cgi_pid, &status, WNOHANG);
				if (ret == pid && WEXITSTATUS(status) != 0)
				{
					this->status = ERROR;
					return (false);
				}
				else if (ret == pid)
					this->status = END;
				else if (ret == -1)
				{
					this->status = ERROR;
					return (false);
				}
				return (true);
			}
		}

	private:
		std::string program;		// cgi를 실행할 프로그램 경로 (예시 "/usr/bin/python")	// cgi_program
		std::string script;			// cgi를 실행할 파일 경로 (my_loc.path 이후) (예시 "hello.py")	// cgi_file
		std::string result_file;	// cgi가 출력한 결과물을 담은 파일 이름.	// cgi_file_name
		std::string body_file;		// cgi 실행전 사용할 바디 파일 이름.		// cgi_body_file
		int file_fd;				// cgi가 출력한 결과물을 담는 파일의 fd.	// file_fd
		int pid;					// cgi가 실행되고 있는 자식프로세스의 pid	// cgi_pid
		int status;					// cgi가 실행되고 있는 자식프로세스의 상태 (END, ERROR, RUNNING)	// cgi_status

		typedef struct s_target_info
		{
			std::string target;	// request target
			std::string url;	// target에서 query를 제외한 부분
			std::string query;	// target에서 queuy (없으면 빈 문자열)
			std::string root;	// 이 target에 대한 root
		} t_target_info;
		t_target_info target_info;

		// ANCHOR - req_info
		typedef struct s_req_info
		{
			std::map<std::string, std::string> *header_map;
			std::string method;
			char *client_ip;
			std::string server_host;
			std::string server_port;
			std::string version;
			long sid;
		} t_req_info;
		t_req_info req_info;
		
		void make_target_info(std::string &target, std::string root)
		{
			size_t qmark_pos = target.find('?');
			std::string url;
			std::string query;
			if (qmark_pos == std::string::npos)
			{
				url = target;
				query = "";
			}
			else if (qmark_pos == target.length())
			{
				url = target.substr(0, target.length() - 1);
				query = "";
			}
			else
			{
				url = target.substr(0, qmark_pos);
				query = target.substr(qmark_pos + 1);
			}
			this->target_info.target = target;
			this->target_info.url = url;
			this->target_info.query = query;
			this->target_info.root = root;
		}
		// void make_req_info(Client &client);
		void make_req_info(std::map<std::string, std::string> &headers, \
	std::string method, int sock_fd, std::string host, int port, std::string version, \
	long sid)
		{
			this->req_info.header_map = &headers;
			this->req_info.method = method;
			struct sockaddr_in client_sockaddr;
			socklen_t client_sockaddr_len = sizeof(client_sockaddr);
			getsockname(sock_fd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_len);
			this->req_info.client_ip = inet_ntoa(client_sockaddr.sin_addr);
			this->req_info.server_host = host;
			this->req_info.server_port = util::num_to_string(port);
			this->req_info.version = version;
			this->req_info.sid = sid;
		}
		std::string make_realpath(void)
		{
			std::string file_path = this->target_info.root + this->script;
			char buf[PATH_MAX],buf2[PATH_MAX];
			realpath(file_path.c_str(), buf); //상대경로를 절대경로로 변경.
			file_path = std::string(buf); //실행할 경로를 절대경로로 재지정.
			realpath(this->program.c_str(), buf2);
			this->program = std::string(buf2);
			return (file_path);
		}

		void set_cgi_env_path(std::map<std::string, std::string> &env_map, std::string &target)
		{
			std::string intra_exe = ".bla";
			if (std::equal(intra_exe.rbegin(), intra_exe.rend(), this->script.rbegin()) == true)
			{
				env_map["PATH_INFO"] = this->target_info.target;
				env_map["PATH_TRANSLATED"] = "";
			}
			else
			{
				size_t dot_pos = target.rfind(this->script);
				env_map["PATH_INFO"] = std::string(target, dot_pos + this->script.length());
				if (env_map["PATH_INFO"].length() != 0)
				{
					char *buf = realpath(env_map["PATH_INFO"].c_str(), NULL);
					if (buf != NULL)
						env_map["PATH_TRANSLATED"] = std::string(buf);
					else
						return;
				}
				else
					env_map["PATH_TRANSLATED"] = "";
				env_map["SCRIPT_NAME"] = target.substr(0, dot_pos + this->script.length());
			}
			return;
		}
		void remove_lr_space(std::string &s)
		{
			// erase right space
			s.erase(s.find_last_not_of(' ') + 1);
			// erase left space
			s.erase(0, s.find_first_not_of(' '));
		}

		static char to_custom_header_format(char c)
		{
			if (c == '-')
				return ('_');
			if (std::islower(c))
				return (::toupper(c));
			return (c);
		}

		void set_cgi_custom_env(std::map<std::string, std::string> &env_map, std::map<std::string, std::string> &request_headers)
		{
			std::map<std::string, std::string>::iterator iter;
			for(iter = request_headers.begin(); iter != request_headers.end(); iter++)
			{
				if ((*iter).first.compare(0, 2, "X-") == 0)
				{
					std::string new_header = std::string((*iter).first.length(), '\0');
					std::transform((*iter).first.begin(), (*iter).first.end(), new_header.begin(), to_custom_header_format);
					remove_lr_space((*iter).second);
					env_map.insert(std::make_pair("HTTP_" + new_header, (*iter).second));
				}
			}
		}

		char **init_env(void)
		{
			// 1. 필요한 정보들 가공해서 map 에 넣기
			std::map<std::string, std::string> env_map;
			env_map["AUTH_TYPE"] = ""; // 인증과정 없으므로 NULL
			env_map["CONTENT_LENGTH"] = "-1"; // 길이 모른다면 -1
			if (this->req_info.header_map->find("Content-Type") != this->req_info.header_map->end())
				env_map["CONTENT_TYPE"] = this->req_info.header_map->at("Content-Type");
			else
				env_map["CONTENT_TYPE"] = "";
			env_map["UPLOAD_PATH"] = this->target_info.root;
			env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
			env_map["REQUEST_METHOD"] = this->req_info.method;
			env_map["QUERY_STRING"] = this->target_info.query;
			env_map["REMOTE_ADDR"] = std::string(this->req_info.client_ip);
			env_map["REMOTE_USER"] = ""; // 인증과정 없으므로 NULL
			env_map["SERVER_NAME"] = this->req_info.server_host + ":" + this->req_info.server_port;
			env_map["SERVER_PORT"] = this->req_info.server_port;
			env_map["SERVER_PROTOCOL"] = this->req_info.version;
			env_map["SERVER_SOFTWARE"] = "soo-je-webserv/1.0";
			if (this->req_info.sid != 0)
				env_map["HTTP_COOKIE"] = this->req_info.header_map->find("Cookie")->second;
			this->set_cgi_env_path(env_map, this->target_info.url);
			this->set_cgi_custom_env(env_map, *(this->req_info.header_map));
			char **cgi_env = new char *[sizeof(char *) * env_map.size() + 1];
			int i = 0;
			for(std::map<std::string, std::string>::iterator iter = env_map.begin(); iter != env_map.end(); iter++)
			{
				cgi_env[i] = strdup((iter->first + "=" + iter->second).c_str());
				i++;
			}
			#ifdef TEST
			std::cerr << "**** CGI ENV ****\n";
			for(size_t i = 0; i < env_map.size(); i++)
				std::cerr << cgi_env[i] << std::endl;
			std::cerr << "*****************\n";
			#endif
			cgi_env[env_map.size()] = NULL;
			return (cgi_env);
		}
};

#endif