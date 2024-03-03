#pragma once

class ClientSocketScoped final {

    public:

    ClientSocketScoped() = delete;
    explicit ClientSocketScoped(int client_socket, sockaddr_in&& addr_in) noexcept;

    ~ClientSocketScoped() noexcept;

    ClientSocketScoped(const ClientSocketScoped&) = delete;

    ClientSocketScoped(ClientSocketScoped&& other) noexcept;

    ClientSocketScoped& operator = (const ClientSocketScoped&) = delete;

    ClientSocketScoped& operator = (ClientSocketScoped&& other) noexcept;

    [[nodiscard]]
    int
    getDescription() const noexcept;

    [[nodiscard]]
    bool
    connected() const noexcept;

    void disconnect() noexcept;

    void swap(ClientSocketScoped&& other) noexcept;


    private:
    
    int _socket_id;
    sockaddr_in _addr_in;
};