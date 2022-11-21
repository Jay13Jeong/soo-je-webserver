#include <iostream>
#include "../webserv.hpp"

void debug_config_parse(Webserv &webserv)
{
    for(int i = 0; i < webserv.get_server_list().size(); i++)
    {
        std::cout << "==================== #" << i + 1 <<  " ====================" << std::endl;
        std::cout << "port :\t\t" << webserv.get_server_list()[i].get_port() << std::endl;
        std::cout << "server_name :\t" << webserv.get_server_list()[i].get_server_name() << std::endl;
        std::cout << "root :\t\t" << webserv.get_server_list()[i].get_root() << std::endl;
        std::cout << "index :\t\t";
        if (webserv.get_server_list()[i].get_index().size() != 0)
        {
            for(int k = 0; k < webserv.get_server_list()[i].get_index().size(); k++)
                std::cout << webserv.get_server_list()[i].get_index()[k]<< " ";
			std::cout << std::endl;
        }
        else
            std::cout << "NONE" << std::endl;
        std::cout << "location :\t";
        if (webserv.get_server_list()[i].get_loc().size() != 0)
        {
            std::cout << std::endl;
			for(int k = 0; k < webserv.get_server_list()[i].get_loc().size(); k++)
            {
                std::cout << "\t- path : " << webserv.get_server_list()[i].get_loc()[k].path << std::endl;
                std::cout << "\t\t- root : " << webserv.get_server_list()[i].get_loc()[k].root << std::endl;
                if (webserv.get_server_list()[i].get_loc()[k].redirection.empty())
                    std::cout << "\t\t- redirection : NONE\n";
				std::map<std::string, std::string> temp_map = webserv.get_server_list()[i].get_loc()[k].redirection;
				for(auto &iter : temp_map)
                    std::cout << "\t\t- redirection : " << iter.first << " -> " << iter.second << std::endl;
                std::cout << "\t\t- index : ";
                if (webserv.get_server_list()[i].get_loc()[k].index.size() != 0)
                {
                    for (int j = 0; j < webserv.get_server_list()[i].get_loc()[k].index.size(); j++)
                        std::cout << webserv.get_server_list()[i].get_loc()[k].index[j] << " ";
                    std::cout << std::endl;
                }
                else
                    std::cout << "NONE\n";
                std::cout << "\t\t- autoindex : " << std::boolalpha << webserv.get_server_list()[i].get_loc()[k].autoindex << std::endl;
                std::cout << "\t\t- accept_method : ";
                for(int j = 0; j < webserv.get_server_list()[i].get_loc()[k].accept_method.size(); j++)
                    std::cout << webserv.get_server_list()[i].get_loc()[k].accept_method[j] << " ";
                std::cout << std::endl;
            }
        }
        else
            std::cout << "NONE" << std::endl;
        std::cout << "autoindex :\t" << std::boolalpha << webserv.get_server_list()[i].get_autoindex() << std::endl;
        std::cout << "client_max_body_size :\t" << webserv.get_server_list()[i].get_max_body_size() << std::endl;
        std::cout << "default_error_pages :\t";
        if (webserv.get_server_list()[i].get_default_error_page().size() != 0)
        {
            for(auto &iter : webserv.get_server_list()[i].get_default_error_page())
                std::cout << "\t- " << iter.first << " -> " << iter.second << std::endl;
        }
        else
            std::cout << "NONE" << std::endl;
        std::cout << "cgi_map :\t";
        if (webserv.get_server_list()[i].get_cgi_map().size() != 0)
        {
            std::cout << std::endl;
			std::map<std::string, std::string> temp_map = webserv.get_server_list()[i].get_cgi_map();
			for(auto &iter : temp_map)
                std::cout << "\t- " << iter.first << " -> " << iter.second << std::endl;
        }
        else
            std::cout << "NONE" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    Webserv webserv;

    if (argc != 1)
        std::cout << "[ERROR] Need config file" << std::endl;
    else
    {
        if (!webserv.parsing("default.conf"))
            std::cout << "[ERROR] Config file error" << std::endl;
        else
        {
            std::cout << "[OK] Server Setup Complete" << std::endl;
            debug_config_parse(webserv);
        }
    }
    return (0);
}