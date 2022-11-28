#include <iostream>
#include "../webserv.hpp"

void debug_config_parse(Webserv &webserv)
{
    for(int i = 0; i < webserv.get_server_list().size(); i++)
    {
        // std::cerr << "==================== #" << i + 1 <<  " ====================" << std::endl;
        // std::cerr << "port :\t\t" << webserv.get_server_list()[i].get_port() << std::endl;
        // std::cerr << "server_name :\t" << webserv.get_server_list()[i].get_server_name() << std::endl;
        // std::cerr << "root :\t\t" << webserv.get_server_list()[i].get_root() << std::endl;
        // std::cerr << "index :\t\t";
        if (webserv.get_server_list()[i].get_index().size() != 0)
        {
            for(int k = 0; k < webserv.get_server_list()[i].get_index().size(); k++)
                // std::cerr << webserv.get_server_list()[i].get_index()[k]<< " ";
			// std::cerr << std::endl;
        }
        else
            // std::cerr << "NONE" << std::endl;
        // std::cerr << "location :\t";
        if (webserv.get_server_list()[i].get_loc().size() != 0)
        {
            // std::cerr << std::endl;
			for(int k = 0; k < webserv.get_server_list()[i].get_loc().size(); k++)
            {
                // std::cerr << "\t- path : " << webserv.get_server_list()[i].get_loc()[k].path << std::endl;
                // std::cerr << "\t\t- root : " << webserv.get_server_list()[i].get_loc()[k].root << std::endl;
                if (webserv.get_server_list()[i].get_loc()[k].redirection.empty())
                    // std::cerr << "\t\t- redirection : NONE\n";
				std::map<std::string, std::string> temp_map = webserv.get_server_list()[i].get_loc()[k].redirection;
				for(auto &iter : temp_map)
                    // std::cerr << "\t\t- redirection : " << iter.first << " -> " << iter.second << std::endl;
                // std::cerr << "\t\t- index : ";
                if (webserv.get_server_list()[i].get_loc()[k].index.size() != 0)
                {
                    for (int j = 0; j < webserv.get_server_list()[i].get_loc()[k].index.size(); j++)
                        // std::cerr << webserv.get_server_list()[i].get_loc()[k].index[j] << " ";
                    // std::cerr << std::endl;
                }
                else
                    // std::cerr << "NONE\n";
                // std::cerr << "\t\t- autoindex : " << std::boolalpha << webserv.get_server_list()[i].get_loc()[k].autoindex << std::endl;
                // std::cerr << "\t\t- accept_method : ";
                for(int j = 0; j < webserv.get_server_list()[i].get_loc()[k].accept_method.size(); j++)
                    // std::cerr << webserv.get_server_list()[i].get_loc()[k].accept_method[j] << " ";
                // std::cerr << std::endl;
            }
        }
        else
            // std::cerr << "NONE" << std::endl;
        // std::cerr << "autoindex :\t" << std::boolalpha << webserv.get_server_list()[i].get_autoindex() << std::endl;
        // std::cerr << "client_max_body_size :\t" << webserv.get_server_list()[i].get_max_body_size() << std::endl;
        // std::cerr << "default_error_pages :\t";
        if (webserv.get_server_list()[i].get_default_error_page().size() != 0)
        {
            for(auto &iter : webserv.get_server_list()[i].get_default_error_page())
                // std::cerr << "\t- " << iter.first << " -> " << iter.second << std::endl;
        }
        else
            // std::cerr << "NONE" << std::endl;
        // std::cerr << "cgi_map :\t";
        if (webserv.get_server_list()[i].get_cgi_map().size() != 0)
        {
            // std::cerr << std::endl;
			std::map<std::string, std::string> temp_map = webserv.get_server_list()[i].get_cgi_map();
			for(auto &iter : temp_map)
                // std::cerr << "\t- " << iter.first << " -> " << iter.second << std::endl;
        }
        else
            // std::cerr << "NONE" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    Webserv webserv;

    if (argc != 1)
        // std::cerr << "[ERROR] Need config file" << std::endl;
    else
    {
        if (!webserv.parsing("default.conf"))
            // std::cerr << "[ERROR] Config file error" << std::endl;
        else
        {
            // std::cerr << "[OK] Server Setup Complete" << std::endl;
            debug_config_parse(webserv);
        }
    }
    return (0);
}