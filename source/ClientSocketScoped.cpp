#include "../common.h"
#include "../include/ClientSocketScoped.h"

ClientSocketScoped::ClientSocketScoped(int client_socket, sockaddr_in&& addr_in) noexcept
    : _socket_id { client_socket }
    , _addr_in { std::move(addr_in) }
{
    std::cout << "client open socket id: " << _socket_id << "\n";
}

//-------------------------------------------------

ClientSocketScoped::~ClientSocketScoped() noexcept
{
    disconnect();
}

//-------------------------------------------------

ClientSocketScoped::ClientSocketScoped(ClientSocketScoped&& other) noexcept
    : _socket_id { -1 }
    , _addr_in { }
{
    if(this == std::addressof(other))
        return;

    swap(std::move(other));
}

//-------------------------------------------------

ClientSocketScoped& ClientSocketScoped::operator = (ClientSocketScoped&& other) noexcept
{
    if(this != std::addressof(other))
        swap(std::move(other));

    return *this;
};

//-------------------------------------------------


[[nodiscard]]
int
ClientSocketScoped::getDescription() const noexcept
{
    return _socket_id;
}

//-------------------------------------------------

void ClientSocketScoped::disconnect() noexcept
{
    if(_socket_id != -1)
    {
        std::cout << "client close socket id: " << _socket_id << "\n";
        close(_socket_id);
        _socket_id = -1;
    }
}

//-------------------------------------------------

[[nodiscard]]
bool
ClientSocketScoped::connected() const noexcept
{
    return _socket_id != -1;
}

//-------------------------------------------------

void ClientSocketScoped::swap(ClientSocketScoped&& other) noexcept
{
    ClientSocketScoped&& tmp { std::move(other) };

    std::swap(_socket_id, tmp._socket_id);
    std::swap(_addr_in, tmp._addr_in);
}

//-------------------------------------------------
