#pragma once

class ServerSocketScoped final {

    public:
    ServerSocketScoped() noexcept;

    ~ServerSocketScoped() noexcept;

    ServerSocketScoped(const ServerSocketScoped&) = delete;

    ServerSocketScoped(ServerSocketScoped&& other) noexcept;

    ServerSocketScoped& operator = (const ServerSocketScoped&) = delete;

    ServerSocketScoped& operator = (ServerSocketScoped&& other) noexcept;

    [[nodiscard]]
    int
    getDescription() const noexcept;

    void disconnect() noexcept;

    void swap(ServerSocketScoped&& other) noexcept;

    private:
    
    int _socket_id;
};