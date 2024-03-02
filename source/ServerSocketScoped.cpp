#include "../include/ServerSocketScoped.h"
#include "../common.h"

ServerSocketScoped::ServerSocketScoped() noexcept
    : _socket_id { socket(AF_INET, SOCK_STREAM, 0) }
{
    std::cout << "open socket id: " << _socket_id << "\n";
}

//-------------------------------------------------

ServerSocketScoped::~ServerSocketScoped() noexcept
{
    disconnect();
}

//-------------------------------------------------

ServerSocketScoped::ServerSocketScoped(ServerSocketScoped&& other) noexcept
{
    if(&other == this)
        return;

    std::swap(other._socket_id, _socket_id);
}

//-------------------------------------------------

ServerSocketScoped& ServerSocketScoped::operator == (ServerSocketScoped&& other) noexcept
{
    if(&other != this)
        std::swap(other._socket_id, _socket_id);

    return *this;
};

//-------------------------------------------------


[[nodiscard]]
int
ServerSocketScoped::getDescription() const noexcept
{
    return _socket_id;
}

//-------------------------------------------------

void ServerSocketScoped::disconnect() noexcept
{
    if(_socket_id != -1)
    {
        std::cout << "close socket id: " << _socket_id << "\n";
        close(_socket_id);
        _socket_id = -1;
    }
}

//-------------------------------------------------
