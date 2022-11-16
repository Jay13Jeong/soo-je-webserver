#include <string>
#include "server.hpp"
#include "client.hpp"

class IO_manager
{
private:
    /* data */
public:
    IO_manager(/* args */);
    ~IO_manager();
    IO_manager(int fd, std::string type, bool done) : fd(fd), type(type), done(done) {};

public:
    int fd;
    std::string type; //fd의 종류. server client file 셋중에 하나.
    bool done; //작업완료 true 미완료 false.
    Client *client;
    Server *server;

    int getFd()
    {
        return this.fd;
    }
    void setFd(int fd)
    {
        this.fd = fd;
    }
    std::string getType()
    {
        return this.type;
    }
    void setType(std::string type)
    {
        this.type = type;
    }
    bool isDone()
    {
        return this.done;
    }
    void setDone(bool done)
    {
        this.done = done;
    }
    Client *get_client()
    {
        return this.client;
    }
    void set_client(Client *client)
    {
        this.client = client;
    }
    Server *get_server()
    {
        return this.server;
    }
    void set_server(Server *server)
    {
        this.server = server;
    }
};
