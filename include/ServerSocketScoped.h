#pragma once

class ServerSocketScoped final {

    public:
    ServerSocketScoped() noexcept;

    ~ServerSocketScoped() noexcept;

    explicit ServerSocketScoped(const ServerSocketScoped&) = delete;

    explicit ServerSocketScoped(ServerSocketScoped&& other) noexcept;

    ServerSocketScoped& operator == (const ServerSocketScoped&) = delete;

    ServerSocketScoped& operator == (ServerSocketScoped&& other) noexcept;

    [[nodiscard]]
    int
    getDescription() const noexcept;

    void disconnect() noexcept;

    private:
    
    int _socket_id;
};