#ifndef CONF_PARSER_HPP
# define CONF_PARSER_HPP
#include "webserv.hpp"

class Parser
{
private:
    Webserv & webserv;

public:
    Parser(Webserv & webserv_) : webserv(webserv_) {};
    ~Parser(){};

    // exception
    class InvalidConfigFile : public std::exception
    {
        public:
            const char *what() const throw()
            {
                return ("Error : Invalid config file.");
            }
    };

    bool    check_config_validation(std::string filename)
    {
        std::ifstream config_fs;
        std::string line;
        std::stack<std::string> bracket_stack;
        bool set = false;
        try {
            config_fs.open(filename);
            if (!config_fs.is_open())
                throw (Parser::InvalidConfigFile());
            while (!config_fs.eof())
            {
                std::vector<std::string> split_result;
                getline(config_fs, line);
                int semicolon_cnt = util::count_char(line, ';');
                if (semicolon_cnt >= 2)
                    throw (Parser::InvalidConfigFile());
                split_result = util::ft_split(line, ";");
                if (split_result.size() == 0)
                    continue;
                split_result = util::ft_split(split_result[0], "\t \n");
                if (split_result.size() == 0 || split_result[0][0] == '#')
                    continue;
                if (split_result[0] == "server")
                {
                    set = true;
                    if (semicolon_cnt != 0)
                        throw (Parser::InvalidConfigFile());

                    if (split_result.size() != 2 || split_result[1] != "{")
                        throw (Parser::InvalidConfigFile());
                    bracket_stack.push("{");
                }
                else if (split_result[0] == "listen")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 2) //
                        throw (Parser::InvalidConfigFile());
                    if (util::is_numeric(split_result[1]) == false)
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "host")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 2) //
                        throw (Parser::InvalidConfigFile());
                    std::string host = split_result[1];
                    if (util::count_char(host, '.') != 3)
                        throw (Parser::InvalidConfigFile());
                    split_result = util::ft_split(host, ".");
                    if (split_result.size() != 4)
                        throw (Parser::InvalidConfigFile());
                    for(int i = 0; i < 4; i++)
                    {
                        if (util::is_numeric(split_result[i]) == false)
                            throw (Parser::InvalidConfigFile());
                    }
                }
                else if (split_result[0] == "server_name")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "root")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "index")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() < 2)
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "default_error_pages")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 3)
                        throw (Parser::InvalidConfigFile());
                    if (util::is_numeric(split_result[1]) == false)
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "autoindex")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                        throw (Parser::InvalidConfigFile());
                    if (split_result[1] != "on" && split_result[1] != "off")
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "client_max_body_size")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 2)
                        throw (Parser::InvalidConfigFile());
                    if (util::is_numeric(split_result[1]) == false)
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "cgi")
                {
                    if (set == false || semicolon_cnt != 1 || split_result.size() != 3)
                        throw (Parser::InvalidConfigFile());
                    if (split_result[1][0] != '.' || split_result[1] != util::to_lower_string(split_result[1]))
                        throw (Parser::InvalidConfigFile());
                }
                else if (split_result[0] == "location")
                {
                    if (set == false || semicolon_cnt != 0 || split_result.size() != 3)
                        throw (Parser::InvalidConfigFile());
                    if (split_result[2] != "{")
                        throw (Parser::InvalidConfigFile());
                    bracket_stack.push("{");
                    while (!config_fs.eof())
                    {
                        getline(config_fs, line);
                        semicolon_cnt = util::count_char(line, ';');
                        if (semicolon_cnt > 1)
                            throw (Parser::InvalidConfigFile());
                        split_result = util::ft_split(line, ";");
                        if (split_result.empty())
                            continue;
                        split_result = util::ft_split(split_result[0], "\t \n");
                        if (split_result.size() == 0 || split_result[0][0] == '#')
                            continue;
                        if (split_result[0] == "}")
                        {
                            if (semicolon_cnt != 0)
                                throw (Parser::InvalidConfigFile());
                            if (bracket_stack.empty())
                                throw (Parser::InvalidConfigFile());
                            bracket_stack.pop();
                            break;
                        }
                        if (split_result[0] == "root" || split_result[0] == "autoindex" || split_result[0] == "client_max_body_size")
                        {
                            if (semicolon_cnt != 1 || split_result.size() != 2)
                                throw (Parser::InvalidConfigFile());
                            if (split_result[0] == "autoindex" && split_result[1] != "on" && split_result[1] != "off")
                                throw (Parser::InvalidConfigFile());
                            if (split_result[0] == "client_max_body_size" && util::is_numeric(split_result[1]) == false)
                                throw (Parser::InvalidConfigFile());
                        }
                        else if (split_result[0] == "return")
                        {
                            if (semicolon_cnt != 1 || split_result.size() != 3)
                                throw (Parser::InvalidConfigFile());
                            if (util::is_numeric(split_result[1]) == false)
                                throw (Parser::InvalidConfigFile());
                        }
                        else if (split_result[0] == "index" || split_result[0] == "accept_method")
                        {
                            if (semicolon_cnt != 1 || split_result.size() == 1)
                                throw (Parser::InvalidConfigFile());
                        }
                        else if (split_result[0] == "cgi")
                        {
                            if (semicolon_cnt != 1 || split_result.size() != 3)
                                throw (Parser::InvalidConfigFile());
                            if (split_result[1][0] != '.' || split_result[1] != util::to_lower_string(split_result[1]))
                                throw (Parser::InvalidConfigFile());
                        }
                        else
                            throw (Parser::InvalidConfigFile());
                    }
                }
                else if (split_result[0] == "}")
                {
                    if (set == false || semicolon_cnt != 0)
                        throw (Parser::InvalidConfigFile());
                    if (bracket_stack.empty() == true)
                        throw (Parser::InvalidConfigFile());
                    bracket_stack.pop();
                }
                else
                    throw (Parser::InvalidConfigFile());
            }
            if (!bracket_stack.empty())
                throw (Parser::InvalidConfigFile());
        }
        catch(std::exception & e)
        {
            config_fs.close();
            std::cerr << RED << "[!] " << e.what() << RESET << std::endl;
            return (false);
        }
        config_fs.close();
        return (true);
    }

    //conf파일을 인자를 받아 파싱 메소드.
    bool    parsing(std::string conf_file)
    {
        std::ifstream config_fs;
    	std::string line;

        if (check_config_validation(conf_file) == false)
             return (false);
        try
        {
            config_fs.open(conf_file);
            if (config_fs.is_open() == false)
                throw (Parser::InvalidConfigFile());
            while (!config_fs.eof())
            {
                std::vector<std::string> split_result;
                getline(config_fs, line);
                split_result = util::ft_split(line, "\t \n");
                if (split_result.size() == 0 || split_result[0][0] == '#')
                    continue;
                if (split_result[0] == "server")
                {
                    Server new_server;
                    webserv.set_server_list(new_server);
                }
                else if (split_result[0] == "listen")
                {
                    util::remove_last_semicolon(split_result[1]);
                    int port = util::string_to_num<int>(split_result[1]);
                    for(size_t i = 0; i < webserv.get_server_list().size() - 1; i++)
                    {
                        if (webserv.get_server_list()[i].port == port)
                            throw (Parser::InvalidConfigFile());
                    }
                    webserv.get_server_list().back().port = port;
                }
                else if (split_result[0] == "server_name")
                {
                    util::remove_last_semicolon(split_result[1]);
                    webserv.get_server_list().back().server_name = split_result[1];
                }
                else if (split_result[0] == "root")
                {
                    util::remove_last_semicolon(split_result[1]);
                    webserv.get_server_list().back().root = split_result[1];
                }
                else if (split_result[0] == "index")
                {
                    webserv.get_server_list().back().get_index().clear();
                    for (size_t i = 1; i < split_result.size(); i++)
                    {
                        if (split_result[i] == ";")
                            break;
                        util::remove_last_semicolon(split_result[i]);
                        webserv.get_server_list().back().index.push_back(split_result[i]);
                    }
                }
                else if (split_result[0] == "autoindex")
                {
                    util::remove_last_semicolon(split_result[1]);
                    webserv.get_server_list().back().autoindex = (split_result[1] == "on");
                }
                else if (split_result[0] == "client_max_body_size")
                {
                    util::remove_last_semicolon(split_result[1]);
                    webserv.get_server_list().back().client_max_body_size = util::string_to_num<size_t>(split_result[1]);
                }
                else if (split_result[0] == "default_error_pages")
                {
                    util::remove_last_semicolon(split_result[1]);
                    util::remove_last_semicolon(split_result[2]);
                    webserv.get_server_list().back().default_error_pages.insert(std::make_pair(split_result[1], split_result[2]));
                }
                else if (split_result[0] == "cgi")
                {
                    if (split_result.size() >= 3)
                    {
                        util::remove_last_semicolon(split_result[1]);
                        util::remove_last_semicolon(split_result[2]);
                        webserv.get_server_list().back().cgi_map.insert(std::make_pair(split_result[1],split_result[2]));
                    }
                }
                else if (split_result[0] == "location")
                {
                    Location loc_temp(webserv.get_server_list().back().get_root(), \
                        webserv.get_server_list().back().get_index(), \
                        webserv.get_server_list().back().get_autoindex(), \
                        webserv.get_server_list().back().get_max_body_size(), \
                        webserv.get_server_list().back().get_cgi_map());
                    loc_temp.path = split_result[1];
                    while (1)
                    {
                        bool cgi_set = false;
                        getline(config_fs, line);
                        util::remove_last_semicolon(line);
                        split_result = util::ft_split(line, "\t \n");
                        if (split_result.size() == 0 || split_result[0][0] == '#')
                            continue;
                        if (split_result[0] == "}"){
                            webserv.get_server_list().back().loc.push_back(loc_temp);
                            break ;
                        }
                        else if (split_result[0] == "return")
                            loc_temp.redirection.insert(std::make_pair(split_result[1], split_result[2]));
                        else if (split_result[0] == "accept_method")
                        {
                            size_t i = 1;
                            loc_temp.accept_method.clear();
                            while (i < split_result.size())
                            {
                                if (split_result[i] == "GET" || split_result[i] == "POST" || split_result[i] == "DELETE" || split_result[i] == "PUT" || split_result[i] == "HEAD")
                                    loc_temp.accept_method.push_back(split_result[i]);
                                else
                                    throw (Parser::InvalidConfigFile());
                                i++;
                            }
                        }
                        else if (split_result[0] == "index")
                        {
                            loc_temp.index.clear();
                            size_t i = 1;
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
                        else if (split_result[0] == "client_max_body_size")
                        {
                            util::remove_last_semicolon(split_result[1]);
                            loc_temp.client_max_body_size = util::string_to_num<size_t>(split_result[1]);
                        }
                        else if (split_result[0] == "cgi")
                        {
                            if (cgi_set == false)
                            {
                                loc_temp.cgi_map.clear();
                                cgi_set = true;
                            }
                            if (split_result.size() >= 3)
                            {
                                util::remove_last_semicolon(split_result[1]);
                                util::remove_last_semicolon(split_result[2]);
                                loc_temp.cgi_map.insert(std::make_pair(split_result[1],split_result[2]));
                            }
                        }
                        else
                            throw (Parser::InvalidConfigFile());
                    }
                }
                else if (split_result[0] == "host")
                {
                    util::remove_last_semicolon(split_result[1]);
                    webserv.get_server_list().back().set_host(split_result[1]);
                }
                else if (split_result[0] == "}")
                    continue;
                else
                    throw (Parser::InvalidConfigFile());
            }
        }
        catch(std::exception & e)
        {
            config_fs.close();
            std::cerr << RED << "[!] " << e.what() << RESET << std::endl;
            return (false);
        }
        config_fs.close();
        return (true);
    }
};

#endif