#pragma once

#include <asio.hpp>
#include <string>
#include <functional>

using asio::ip::tcp;

class Signaler {
    using OnLine = std::function<void(const std::string&)>;

    struct Impl {
        asio::io_context ctx;
        std::unique_ptr<tcp::acceptor> acceptor;
        std::unique_ptr<tcp::socket> socket;
        std::thread thread;
        std::mutex writeMutex;
        Signaler::OnLine onLine;
        std::atomic_bool running{false};
    };

    public:

    Signaler() : pImpl(new Impl()) {};
    ~Signaler() {
        stop();
        if (pImpl->thread.joinable()) {
            pImpl->thread.join();
        }
    };

    bool startHost(uint16_t port);
    bool startClient(const std::string &hostIp, uint16_t port);

    void sendLine(const std::string& line);
    void setOnLine(OnLine cb);

    void stop();
    void run();

    private:
    Impl* pImpl;
};
