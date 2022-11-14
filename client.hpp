#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "location.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Client
{
private:
    int socket_fd;
    Request request;
    Response response;

public:
    Client(/* args */);
    ~Client();
    int getSocket_fd()
    {
        return this.socket_fd;
    }
    void setSocket_fd(int socket_fd)
    {
        this.socket_fd = socket_fd;
    }
    Request getRequest()
    {
        return this.request;
    }
    void setRequest(Request request)
    {
        this.request = request;
    }
    Response getResponse()
    {
        return this.response;
    }
    void setResponse(Response response)
    {
        this.response = response;
    }

};
