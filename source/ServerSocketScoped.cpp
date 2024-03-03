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
    : _socket_id { -1 }
{
    if(this == std::addressof(other))
        return;

    swap(std::move(other));
}

//-------------------------------------------------

ServerSocketScoped& ServerSocketScoped::operator = (ServerSocketScoped&& other) noexcept
{
    if(this != std::addressof(other))
        swap(std::move(other));

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

void ServerSocketScoped::swap(ServerSocketScoped&& other) noexcept
{
    std::swap(_socket_id, other._socket_id);
}

//-------------------------------------------------
