#pragma once

#include <qlexnet.h>
#include <memory>
#include <string>

#include "MessageTypes.h"
#include "Logger.h"


namespace otherside
{
using asio::ip::tcp;

class SignalerServer : public qlexnet::ServerInterface<MsgType> {
    public:
    SignalerServer(uint16_t port) : qlexnet::ServerInterface<MsgType>(port) {}

    protected:
    virtual bool onClientConnect(std::shared_ptr<qlexnet::Connection<MsgType>> client_) override {
        qlexnet::Message<MsgType> msg;
        msg.header.id = MsgType::ACK_CONNECTION;
        client_->send(msg);
        return true;
    }

    virtual void onClientDisconnect(std::shared_ptr<qlexnet::Connection<MsgType>> client_) override {
        _log->msg("Client disconnected");
    }

    virtual void onMessage(std::shared_ptr<qlexnet::Connection<MsgType>> client_, qlexnet::Message<MsgType> &msg_) override {
        switch (msg_.header.id)
        {
        case MsgType::PING:
            {
            std::chrono::steady_clock::time_point start;
            msg_ >> start;
            qlexnet::Message<MsgType> msg;
            msg.header.id = MsgType::PING;
            msg << start;
            client_->send(msg);
            break;
            }
        default:
            break;
        }
    }

    private:
    std::unique_ptr<Logger> _log = std::make_unique<Logger>("SignalerServer");
};

class SignalerClient : public qlexnet::ClientInterface<MsgType> {
    public:
    SignalerClient(const std::string& hostIp, uint16_t port) : qlexnet::ClientInterface<MsgType>(), port(port), hostIp(hostIp) {}

    bool connect() { return qlexnet::ClientInterface<MsgType>::connect(hostIp, port); }

    void run() {
        while (isConnected()) {
            if (incoming().empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            auto msg = incoming().pop_front().msg;

            switch (msg.header.id) {
                case MsgType::PING:
                {
                    std::chrono::steady_clock::time_point start;
                    msg >> start;
                    auto now = std::chrono::high_resolution_clock::now();
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
                    _log->msg("...Pong - ", elapsed_ms.count(), "ms");
                    break;
                }
                case MsgType::ACK_CONNECTION:
                    _log->msg("Server accepted the connnection.");
                    break;
                case MsgType::OFFER:
                {
                    _log->msg("Server published its sdp offer.");
                    std::string type;
                    std::string sdp;
                    msg >> type >> sdp;
                    _log->msg("Received : ", type, " - ", sdp);
                    qlexnet::Message<MsgType> msg;
                    msg.header.id = MsgType::READY;
                    send(msg);
                    break;
                }
                default:
                break;
            }
        }
        _log->msg("Server Down.");
    }

    void ping() {
        qlexnet::Message<MsgType> msg;
        msg.header.id = MsgType::PING;
        auto start = std::chrono::high_resolution_clock::now();
        msg << start;
        _log->msg("Ping...");
        send(msg);
    }

    private:
    uint16_t port;
    std::string hostIp;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("SignalerClient");
};

}