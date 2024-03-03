#pragma once

#include "../common.h"
#include "../include/Utils.h"
#include "ServerSocketScoped.h"
#include "ClientSocketScoped.h"

class HTTPServer final {
    
//------------------------------------------//
//----------------- public -----------------//
//------------------------------------------//
    public:
    
    struct IPADDR_ANY {};

    struct Port {
        size_t port;
    };

    struct MaxCountConnect {
        size_t max_count_connect;
    };

    using ip_address_t = std::variant<IPADDR_ANY, std::string>;

    //-------------------------------------------------
    
    explicit HTTPServer(Port port, ip_address_t ip_address, MaxCountConnect max_count_connect) noexcept;
    HTTPServer() = delete;

    ~HTTPServer() noexcept;

    [[nodiscard]]
    bool
    start () noexcept;

    void connectionLoop() noexcept;

//------------------------------------------//
//---------------- private -----------------//
//------------------------------------------//
    private:

    HTTPServer(const HTTPServer&) = delete;
    HTTPServer(HTTPServer&&) = delete;

    HTTPServer& operator = (const HTTPServer&) = delete;
    HTTPServer& operator = (HTTPServer&&) = delete;

    [[nodiscard]]
    bool 
    Bind() noexcept;

    [[nodiscard]]
    bool 
    Listen() noexcept;

    [[nodiscard]]
    uint32_t
    getRealIP(ip_address_t ip) noexcept;

    [[nodiscard]]
    bool
    resetSocket() const noexcept;

    void newConnection(int client_socket, sockaddr_in&& addr_in);

    [[nodiscard]]
    std::optional<std::string>
    getRequest(int socket_id);

    [[nodiscard]]
    bool
    sendResponce(int socket_id, std::string responce);

    [[nodiscard]]
    std::optional<sockaddr_in>
    generateSockAddrIN();

    [[nodiscard]]
    std::unordered_map<std::string, std::string>
    parseRequest(std::string_view request) const;

    [[nodiscard]]
    constexpr
    bool
    checkPageInfo(std::vector<std::string_view> dataes_tokens) const noexcept;

    [[nodiscard]]
    std::optional<std::string>
    getEndPoint(const std::unordered_map<std::string, std::string>& metainfo) const;

    [[nodiscard]]
    std::optional<std::string>
    getMetaInfo(const std::unordered_map<std::string, std::string>& metainfo, std::string_view find_key) const;

    [[nodiscard]]
    std::optional<std::string>
    getPageFromEndPoint(std::string end_point) const;

    //------------------------

    size_t _port;
    size_t _max_count_connect;
    bool _start_access;
    ip_address_t _ip_address;
    ServerSocketScoped _server_socket;
};


//----------------------------------------------------

[[nodiscard]]
constexpr
bool
HTTPServer::checkPageInfo(std::vector<std::string_view> dataes_tokens) const noexcept
{
    if(dataes_tokens.size() != 3)
        return false;

    std::array<std::string_view, 3> data { "GET", "POST", "HEAD" };
    
    auto fn { std::find(std::begin(data), std::end(data), dataes_tokens[0]) };
    if(fn == std::end(data))
    {
        std::cerr << "fn == std::end(data)\n";
        std::cerr << "dataes_tokens[0]: " << dataes_tokens[0] << "\n";
        return false;
    }

    std::vector<std::string_view> parse_http_version { Utils::parseString(dataes_tokens[2], "/") };
    if(parse_http_version.size() != 2)
    {
        std::cerr << "dataes_tokens[2].start_with('HTTP') == false\n";
        return false;
    }
    if(parse_http_version[0] != "HTTP")
    {
        std::cerr << "parse_http_version[0] != 'HTTP'\n";
        return false;
    }

    return true;
}