#include "../include/HTTPServer.h"
#include "../common.h"

//-------------------------------------------------

HTTPServer::HTTPServer(HTTPServer::Port port, HTTPServer::ip_address_t ip_address, HTTPServer::MaxCountConnect max_count_connect) noexcept
    : _port { port.port }
    , _max_count_connect { max_count_connect.max_count_connect }
    , _start_access { false }
    , _ip_address { std::move(ip_address) }
    , _server_socket { }
    , _end_points { }
{
    std::cout << "Server started..." << std::endl;
    std::cout << "Server socked id: " << _server_socket.getDescription() << "" << std::endl;
    std::cout << "Server _port: " << _port << "" << std::endl;
    std::cout << "Server _max_count_connect: " << _max_count_connect << "" << std::endl;
}

//-------------------------------------------------

HTTPServer::~HTTPServer() noexcept
{
    std::cout << "Server closed..." << std::endl;
}


//-------------------------------------------------

[[nodiscard]]
bool
HTTPServer::start () noexcept
{
    _start_access = resetSocket() and Bind() and Listen();

    std::cout << "Server status: " << (_start_access ? "RUN" : "NOT RUN") << "" << std::endl;

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
    std::cerr << "HTTPServer::Bind" << std::endl;

    std::optional<sockaddr_in> server_addr_op { generateSockAddrIN() };

    if(server_addr_op.has_value() == false)
    {
        std::cerr << "server_addr_op.has_value() == false" << std::endl;
        return false;
    }

    sockaddr_in server_addr { *std::move(server_addr_op) };

    if(bind(_server_socket.getDescription(), std::bit_cast<sockaddr*>(&server_addr), sizeof server_addr) == -1) [[unlikely]]
    {
        std::cerr << "bind(...) == INVALID_SOCKET" << std::endl;
        return false;
    }

    return true;
}

//-------------------------------------------------

[[nodiscard]]
bool 
HTTPServer::Listen() noexcept
{
    std::cerr << "HTTPServer::Listen" << std::endl;

    if(listen(_server_socket.getDescription(), static_cast<int>(_max_count_connect)) == -1 ) [[unlikely]]
    {
        std::cerr << "listen(...) == INVALID_SOCKET" << std::endl;
        return false;
    }
    return true;
}

//-------------------------------------------------

[[nodiscard]]
uint32_t
HTTPServer::getRealIP(ip_address_t ip) noexcept
{
    std::cerr << "HTTPServer::getRealIP" << std::endl;

    if(std::string * str { std::get_if<std::string>(&ip) })
    {
        std::cerr << "ip str:" << *str << "" << std::endl;
        return INADDR_ANY;
    }
    else
    {
        std::cerr << "ip any type:" << INADDR_ANY << "" << std::endl;
        return INADDR_ANY;
    }
}

//-------------------------------------------------

[[nodiscard]]
bool
HTTPServer::resetSocket() const noexcept
{
    std::cerr << "HTTPServer::resetSocket" << std::endl;

    if(_server_socket.getDescription() == -1) [[unlikely]]
    {
        std::cerr << "_server_socket.getDescription() == -1" << std::endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(_server_socket.getDescription(), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) [[unlikely]]
    {
        std::cerr << "setsockopt" << std::endl;
        return false;
    }

    return true;
}

//-------------------------------------------------

void HTTPServer::newConnection(int client_socket_scoped, sockaddr_in&& addr_in)
{
    std::cerr << "-------------------------------" << std::endl;
    std::cerr << "HTTPServer::newConnection start" << std::endl;
    std::cerr << "-------------------------------" << std::endl;
    
    ClientSocketScoped client { client_socket_scoped, std::move(addr_in) };

    int client_socket { client.getDescription() };

    std::optional<std::string> request_opt { getRequest(client_socket) };
    if(request_opt.has_value() == false)
    {
        std::cerr << "request_opt.has_value() == false" << std::endl;
        return;
    }

    if(request_opt.value().size() == 0)
        return;

    const std::string request { *std::move(request_opt) };

    const std::unordered_map<std::string, std::string>  metainfo { parseRequest(request) };

    std::string body    {};
    std::string header  {};

    std::optional<std::string> endpoint_opt { getEndPoint(metainfo) };
    if(endpoint_opt.has_value() == false)
    {
        body = getErrorPageFromEndPoint(500);

        // if(body_endpoint_opt.has_value() == false)
        //     body = "<html><header><title>500</title></header><body><h1>500</h1> <a href =\"/\">go to home</a></body></html>";
        // else
        //     body = *std::move(body_endpoint_opt);
        // TODO save log
        header = "HTTP/1.1 500 OK\nContent-Length: "+std::to_string(body.size()) + "\n\n";
    }
    else
    {
        std::cerr << "client_socket id: " << client_socket << "" << std::endl;

        std::optional<std::string> body_endpoint_opt { getPageFromEndPoint(*std::move(endpoint_opt)) };
        if(body_endpoint_opt.has_value() == false)
        {
            body = getErrorPageFromEndPoint(404);
            header = "HTTP/1.1 404 OK\nContent-Length: "+std::to_string(body.size()) + "\n\n";
        }
        else
        {
            body = *std::move(body_endpoint_opt);
            header = "HTTP/1.1 200 OK\nContent-Length: "+std::to_string(body.size()) + "\n\n";
        }
    }

    // std::cerr << "metainfo: " << "" << std::endl;

    // for(auto&& [key, value] : metainfo)
    // {
    //     std::cerr << "key: " << std::quoted(key) << "" << std::endl;
    //     std::cerr << "value: " << value << "" << std::endl;
    //     std::cerr << "-------" << std::endl;
    // }
    //---------------

    

    std::cout << "client_socket: " << client_socket << "\n"
              << "send all bytes: " << (sendResponce(client_socket, std::move(header) + std::move(body)) ? "true" : "false") << "\n------" << std::endl;


    std::cerr << "-----------------------------" << std::endl;
    std::cerr << "HTTPServer::newConnection end" << std::endl;
    std::cerr << "-----------------------------" << std::endl;
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

    // std::string tmp;
    // std::move(std::begin(buffer), std::end(buffer), std::back_inserter(tmp));
    // result_str += tmp;

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
    std::cerr << "HTTPServer::generateSockAddrIN" << std::endl;
    sockaddr_in server_addr {};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);

    if(std::get_if<IPADDR_ANY>(&_ip_address))
    {
        std::cout << "ip any" << std::endl;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        return server_addr;
    }
    else if(std::string* ip_addr_ptr = std::get_if<std::string>(&_ip_address))
    {
        std::cout << "ip str" << std::endl;
        if(inet_aton((*ip_addr_ptr).c_str(), &server_addr.sin_addr) == 0)
        {
            std::cerr << "inet_aton(...) == 0" << std::endl;
            return std::nullopt;
        }

        return server_addr;
    }
    else
    {
        std::cout << "ip any default" << std::endl;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        return server_addr;
    }
}

//-------------------------------------------------

[[nodiscard]]
std::unordered_map<std::string, std::string>
HTTPServer::parseRequest(std::string_view request)  const
{
    std::cerr << "HTTPServer::parseRequest" << std::endl;
    request = Utils::trim(request);
    std::vector<std::string_view> request_as_lines { Utils::parseString(request, "\n") };

    if(request_as_lines.size() < 3)
    {
        std::cerr << "request_as_lines.size() < 3" << std::endl;
        std::cerr << "request: '" << request << "'" << std::endl;
        std::cerr << "request.size(): '" << request.size() << "'" << std::endl;

        std::cerr << "---" << std::endl;
        for(size_t i{}; char c : request)
        {
            std::cerr << static_cast<int>(c) << ( ++i % 8 == 0 ? "\n" : " ");
            std::cerr.flush();
        }
        std::cerr << "---" << std::endl;

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
    auto fn { _end_points.find(std::move(end_point)) };
    if(fn == std::end(_end_points))
        return std::nullopt;
    
    std::optional<std::string> body_opt { readFile("./www/" + fn->second) };

    if(body_opt.has_value() == false)
        return std::nullopt;

    return *std::move(body_opt);
}

//-------------------------------------------------

void HTTPServer::addEndPoint(std::string end_point, std::string filename)
{
    _end_points.insert(std::pair(std::move(end_point), std::move(filename)));
}

//-------------------------------------------------

[[nodiscard]]
std::optional<std::string>
HTTPServer::readFile(std::filesystem::path path) const
{
    std::ifstream fs { path, std::ios::binary };
    if(fs.is_open() == false)
        return std::nullopt;
    
    const auto sz { std::filesystem::file_size(path) };

    if(sz == 0)
        return std::nullopt;
    
    std::string result(sz, '\0');
    fs.read(result.data(), static_cast<long>(sz));

    return result;
}

//-------------------------------------------------

[[nodiscard]]
std::string
HTTPServer::getErrorPageFromEndPoint(int32_t error_code) const
{
    std::string error_code_as_str { std::to_string(error_code) };
    std::string error_file { "./www/errors/" + error_code_as_str + ".html" };
    std::optional<std::string> body_opt { readFile(error_file) };

    if(body_opt.has_value() == false)
       return "<html><header><title>" + error_code_as_str + "</title></header><body><h1>" + error_code_as_str + "</h1> <a href =\"/\">go to home</a></body></html>";
    
    return *std::move(body_opt);
}

//-------------------------------------------------
