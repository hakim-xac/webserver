#include "common.h"
#include "include/HTTPServer.h"

int main()
{

    HTTPServer server { HTTPServer::Port { 8000 }, HTTPServer::IPADDR_ANY{}, HTTPServer::MaxCountConnect { 10 } };
    //HTTPServer server { HTTPServer::Port { 8000 }, "127.0.0.1", HTTPServer::MaxCountConnect { 10 } };

    if(server.start() == false)
    {
        std::cout << "Error: Server not run!\n";
        return 1;
    }

    server.connectionLoop();

    return 0;
}