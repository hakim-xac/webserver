#include "common.h"
#include "include/HTTPServer.h"

int main()
{

    constexpr size_t cr_port { 8000 };
    constexpr size_t cr_max_count_connectoin { 10 };
    HTTPServer server { HTTPServer::Port { cr_port }, HTTPServer::IPADDR_ANY{}, HTTPServer::MaxCountConnect { cr_max_count_connectoin } };
    //HTTPServer server { HTTPServer::Port { 8000 }, "127.0.0.1", HTTPServer::MaxCountConnect { 10 } };

    server.addEndPoint("/", "index.html");
    server.addEndPoint("/index", "index.html");
    server.addEndPoint("/about", "about.html");

    if(server.start() == false)
    {
        std::cout << "Error: Server not run!\n";
        return 1;
    }

    server.connectionLoop();

    return 0;
}