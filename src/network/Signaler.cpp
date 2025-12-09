#include "Signaler.h"
#include <asio.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

using asio::ip::tcp;

bool Signaler::startHost(uint16_t port) {
    try {
        pImpl->acceptor = std::make_unique<tcp::acceptor>(pImpl->ctx, tcp::endpoint(tcp::v4(), port));
        pImpl->socket = std::make_unique<tcp::socket>(pImpl->ctx);
        pImpl->running = true;

        pImpl->thread = std::thread([this, port]() {
            try {
                std::cout << "[Signaler] Host listening on port " << std::to_string(port) << std::endl;
                pImpl->acceptor->accept(*pImpl->socket);
                std::cout << "[Signaler] Client connected" << std::endl;

                asio::streambuf buf;
                while (pImpl->running) {
                    std::size_t n = asio::read_until(*pImpl->socket, buf, '\n');
                    std::istream is(&buf);
                    std::string line;
                    std:getline(is, line);
                    if (pImpl->onLine) pImpl->onLine(line);
                }
            } catch (std::exception& e) {
                std::cerr << "[Signaler] host thread exception: " << e.what() << std::endl;
            }
        });
        return true;
    } catch (std::exception& e) {
        std::cerr << "[Signaler] startHost error: " << e.what() << std::endl;
        return false;
    }
}

bool Signaler::startClient(const std::string &hostIp, uint16_t port) {
    try {
        pImpl->socket = std::make_unique<tcp::socket>(pImpl->ctx);
        pImpl->running = true;
        pImpl->thread = std::thread([this, hostIp, port]() {
            try {
                tcp::endpoint ep(asio::ip::make_address(hostIp), port);
                pImpl->socket->connect(ep);
                std::cout << "[Signaler] Connected to host " << hostIp << ":" << port << std::endl;
                asio::streambuf buf;
                while (pImpl->running) {
                    std::size_t n = asio::read_until(*pImpl->socket, buf, '\n');
                    std::istream is(&buf);
                    std::string line;
                    std::getline(is, line);
                    if (pImpl->onLine) pImpl->onLine(line);
                }
            } catch (std::exception &e) {
                std::cerr << "[Signaler] Client thread exception: " << e.what() << std::endl;
            }
        });
        return true;
    } catch (std::exception &e) {
        std::cerr << "[Signaler] startClient error: " << e.what() << std::endl;
        return false;
    }
}

void Signaler::sendLine(const std::string& line) {
    std::lock_guard<std::mutex> lg(pImpl->writeMutex);
    if (!pImpl->socket || !pImpl->socket->is_open()) return;
    asio::write(*pImpl->socket, asio::buffer(line + '\n'));
}

void Signaler::setOnLine(OnLine cb) {
    pImpl->onLine = std::move(cb);
}

void Signaler::stop() {
    pImpl->running = false;
    try { if (pImpl->socket) pImpl->socket->close(); } catch(...) {}
    try { if (pImpl->acceptor) pImpl->acceptor->close(); } catch(...) {}
}