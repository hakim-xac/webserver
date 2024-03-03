#include "../include/HTTPServer.h"
#include "../common.h"

//-------------------------------------------------

HTTPServer::HTTPServer(HTTPServer::Port port, HTTPServer::ip_address_t ip_address, HTTPServer::MaxCountConnect max_count_connect) noexcept
    : _port { port.port }
    , _max_count_connect { max_count_connect.max_count_connect }
    , _start_access { false }
    , _ip_address { std::move(ip_address) }
    , _server_socket { }
{
    std::cout << "Server started...\n";
}

//-------------------------------------------------

HTTPServer::~HTTPServer() noexcept
{
    std::cout << "Server closed...\n";
}


//-------------------------------------------------

[[nodiscard]]
bool
HTTPServer::start () noexcept
{
    _start_access = resetSocket() and Bind() and Listen();

    std::cout << "Server status: " << (_start_access ? "RUN" : "NOT RUN") << "\n";

    return _start_access;
}

//-------------------------------------------------

void HTTPServer::connectionLoop() noexcept
{
    for(;;)
    {
        if(_start_access == false)
            return;
        sockaddr_in client_addr{};
        uint32_t client_addr_size{};
        
        int client_socket { accept(_server_socket.getDescription(), std::bit_cast<sockaddr*>(&client_addr), &client_addr_size) };
        if(client_socket == -1)
            continue;
        
        std::thread { [&]{ newConnection(client_socket, std::move(client_addr)); } }.detach();

    }
}

//-------------------------------------------------

[[nodiscard]]
bool 
HTTPServer::Bind() noexcept
{
    std::optional<sockaddr_in> server_addr_op { generateSockAddrIN() };

    if(server_addr_op.has_value() == false)
    {
        std::cerr << "server_addr_op.has_value() == false\n";
        return false;
    }

    sockaddr_in server_addr { *std::move(server_addr_op) };

    if(bind(_server_socket.getDescription(), std::bit_cast<sockaddr*>(&server_addr), sizeof server_addr) == -1) [[unlikely]]
    {
        std::cerr << "bind(...) == INVALID_SOCKET\n";
        return false;
    }

    return true;
}

//-------------------------------------------------

[[nodiscard]]
bool 
HTTPServer::Listen() noexcept
{
    if(listen(_server_socket.getDescription(), static_cast<int>(_max_count_connect)) == -1 ) [[unlikely]]
    {
        std::cerr << "listen(...) == INVALID_SOCKET\n";
        return false;
    }
    return true;
}

//-------------------------------------------------

[[nodiscard]]
uint32_t
HTTPServer::getRealIP(ip_address_t ip) noexcept
{
    if(std::string * str { std::get_if<std::string>(&ip) })
    {
        std::cout << "ip str:" << *str << "\n";
        return INADDR_ANY;
    }
    else
    {
        std::cout << "ip any type:" << INADDR_ANY << "\n";
        return INADDR_ANY;
    }
}

//-------------------------------------------------

[[nodiscard]]
bool
HTTPServer::resetSocket() const noexcept
{
    if(_server_socket.getDescription() == -1) [[unlikely]]
    {
        std::cerr << "_server_socket.getDescription() == -1\n";
        return false;
    }

    int opt = 1;
    if (setsockopt(_server_socket.getDescription(), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) [[unlikely]]
    {
        std::cerr << "setsockopt\n";
        return false;
    }

    return true;
}

//-------------------------------------------------

void HTTPServer::newConnection(int client_socket, sockaddr_in&& addr_in)
{
    
    ClientSocketScoped client { client_socket, std::move(addr_in) };

    std::optional<std::string> request_opt { getRequest(client_socket) };
    if(request_opt.has_value() == false)
    {
        std::cerr << "request_opt.has_value() == false" << "\n";
        return;
    }

    if(request_opt.value().empty())
    {
        std::cerr << "request_opt.value().empty()" << "\n";
        return;
    }

    const std::string request { *std::move(request_opt) };

    const std::unordered_map<std::string, std::string>  metainfo { parseRequest(request) };

    std::string body    {};
    std::string header  {};

    std::optional<std::string> endpoint_opt { getEndPoint(metainfo) };
    if(endpoint_opt.has_value() == false)
    {
        body = "<html><header><title>404</title></header><body><h1>404</h1> <a href =\"/\">go to home</a></body></html>";
        header = "HTTP/1.1 404 OK\nContent-Length: "+std::to_string(body.size()) + "\n\n";
    }
    else
    {
        std::optional<std::string> body_endpoint_opt { getPageFromEndPoint(*std::move(endpoint_opt)) };
        if(body_endpoint_opt.has_value() == false)
        {
            body = "<html><header><title>404</title></header><body><h1>404</h1> <a href =\"/\">go to home</a></body></html>";
            header = "HTTP/1.1 404 OK\nContent-Length: "+std::to_string(body.size()) + "\n\n";
        }
        else
        {
            body = *std::move(body_endpoint_opt);
            header = "HTTP/1.1 200 OK\nContent-Length: "+std::to_string(body.size()) + "\n\n";
        }
    }

    // std::cerr << "metainfo: " << "\n";

    // for(auto&& [key, value] : metainfo)
    // {
    //     std::cerr << "key: " << std::quoted(key) << "\n";
    //     std::cerr << "value: " << value << "\n";
    //     std::cerr << "-------\n";
    // }
    //---------------

    

    std::cout << "client_socket: " << client_socket << "\n"
    "send all bytes: " << (sendResponce(client_socket, std::move(header) + std::move(body)) ? "true" : "false") << "\n------\n";
}

//-------------------------------------------------

[[nodiscard]]
std::optional<std::string>
HTTPServer::getRequest(int socket_id)
{
    constexpr size_t BUFFER_SIZE { 256 };

    std::array <char, BUFFER_SIZE> buffer{};
    ssize_t bytes_read{};
    std::string result_str{};  

    for(bytes_read = recv(socket_id, buffer.data(), buffer.size(), 0);
        bytes_read == BUFFER_SIZE;
        bytes_read = recv(socket_id, buffer.data(), buffer.size(), 0) )
    {

        std::string tmp;
        std::move(std::begin(buffer), std::end(buffer), std::back_inserter(tmp));
        result_str += tmp;
    }

    std::string tmp;
    std::move(std::begin(buffer), std::end(buffer), std::back_inserter(tmp));
    result_str += tmp;

    return result_str;
}

//-------------------------------------------------

[[nodiscard]]
bool
HTTPServer::sendResponce(int socket_id, std::string responce)
{
    if(socket_id == -1)
        return false;
    
    long result_send { send(socket_id, responce.c_str(), responce.size(), 0) };
    return result_send == responce.size();
}

//-------------------------------------------------

[[nodiscard]]
std::optional<sockaddr_in>
HTTPServer::generateSockAddrIN()
{
    sockaddr_in server_addr {};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);

    if(std::get_if<IPADDR_ANY>(&_ip_address))
    {
        std::cout << "ip any\n";
        server_addr.sin_addr.s_addr = INADDR_ANY;
        return server_addr;
    }
    else if(std::string* ip_addr_ptr = std::get_if<std::string>(&_ip_address))
    {
        std::cout << "ip str\n";
        if(inet_aton((*ip_addr_ptr).c_str(), &server_addr.sin_addr) == 0)
        {
            std::cerr << "inet_aton(...) == 0\n";
            return std::nullopt;
        }

        return server_addr;
    }
    else
    {
        std::cout << "ip any default\n";
        server_addr.sin_addr.s_addr = INADDR_ANY;
        return server_addr;
    }
}

//-------------------------------------------------

[[nodiscard]]
std::unordered_map<std::string, std::string>
HTTPServer::parseRequest(std::string_view request)  const
{
    std::vector<std::string_view> request_as_lines { Utils::parseString(request, "\n") };

    if(request_as_lines.size() < 3)
    {
        std::cerr << "request_as_lines.size() < 3\n";
        return {};
    }

    std::unordered_map<std::string, std::string>  metainfo;
    metainfo.reserve(request_as_lines.size() + 3); // adding other keys and values

    //-----------------
    for(size_t i{}; std::string_view line : request_as_lines)
    {
        if(line.size() < 2)
            continue;

        std::vector<std::string_view> line_as_tokens { Utils::parseString(line, ":") };

        if(line_as_tokens.size() == 1)
        {   
            std::vector<std::string_view> tokens { Utils::parseString(line_as_tokens[0], " ") };

            if(tokens.size() == 3 && checkPageInfo(tokens) )
            {
                // GET, POST ...
                metainfo.emplace("HTTP_METHOD" , std::string(tokens[0]));
                metainfo.emplace("HOST_PATH" , std::string(tokens[1]));
                metainfo.emplace("HTTP_VERSION" , std::string(tokens[2]));
            }
        }
        else if(line_as_tokens.size() == 2)
        {
            auto fn { std::find(std::begin(line), std::end(line), ':') };

            size_t pos { static_cast<size_t>(std::distance(std::begin(line), fn)) };

            if(pos >= line.size())
                return {};

            std::string_view key { Utils::trim(line.substr(0, pos+1)) };
            std::string_view value { Utils::trim(line.substr(pos+1, line.size() - pos-1)) };
            
            metainfo.emplace(std::string(key) , std::string(value));
        }
    }

    return metainfo;
}

//-------------------------------------------------

[[nodiscard]]
std::optional<std::string>
HTTPServer::getEndPoint(const std::unordered_map<std::string, std::string>& metainfo) const
{
    return getMetaInfo(metainfo, "HOST_PATH");
}

//-------------------------------------------------

[[nodiscard]]
std::optional<std::string>
HTTPServer::getMetaInfo(const std::unordered_map<std::string, std::string>& metainfo, std::string_view find_key) const
{
    auto fn { metainfo.find(std::string{find_key}) };
    if(fn == std::end(metainfo))
        return std::nullopt;
    return fn->second;
}

//-------------------------------------------------

[[nodiscard]]
std::optional<std::string>
HTTPServer::getPageFromEndPoint(std::string end_point) const
{
    if(end_point == "/" || end_point == "/index")
        return "<html><header><title>my page</title></header><body><h1>header</h1> <ul>"
        "<li><a href =\"/\">go to home</a>"
        "<li><a href =\"/exit\">go to exit</a>"
        "</ul></body></html>";
    else
        return std::nullopt;
}

//-------------------------------------------------
