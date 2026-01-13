#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>
#include <string>

#include "logger/Logger.h"


namespace otherside
{
using asio::ip::tcp;

enum class MsgType : uint32_t
{
    PING,
    ACK_CONNECTION,
    REQUEST,
    OFFER,
    READY,
    ANSWER
};

class SignalerServer : public qlexnet::ServerInterface<MsgType> {
    public:
    SignalerServer(uint16_t port) : qlexnet::ServerInterface<MsgType>(port) {}

    std::function<void(uint32_t)> onRequest;
    std::function<void(uint32_t, rtc::Description)> onReady;

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
            qlexnet::MessageReader mr(msg_);
            std::chrono::steady_clock::time_point start;
            auto ok = mr.readString();
            mr.read(start);
            _log->msg(ok);
            qlexnet::Message<MsgType> msg;
            msg.header.id = MsgType::PING;

            qlexnet::MessageWriter mw(msg);
            mw.write(start);
            client_->send(msg);
            break;
        }
        case MsgType::REQUEST:
        {
            onRequest(client_->GetID());
            break;
        }
        case MsgType::READY:
        {
            qlexnet::MessageReader mr(msg_);
            auto type = mr.readString();
            auto sdp = mr.readString();
            _log->msg(type, sdp);
            onReady(client_->GetID(), rtc::Description(sdp, type));
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

    std::function<void(rtc::Description)> onOffer;

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
                    qlexnet::MessageReader mr(msg);
                    mr.read(start);
                    auto now = std::chrono::high_resolution_clock::now();
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
                    _log->msg("...Pong - ", elapsed_ms.count(), "ms");
                    break;
                }
                case MsgType::ACK_CONNECTION:
                {
                    _log->msg("Server accepted the connnection.");
                    qlexnet::Message<MsgType> msg;
                    msg.header.id = MsgType::REQUEST;
                    send(msg);
                    break;
                }
                case MsgType::OFFER:
                {
                    _log->msg("Server published its SDP offer (size: ", msg.header.size, ")");
                    qlexnet::MessageReader mr(msg);
                    auto type = mr.readString();
                    auto sdp = mr.readString();
                    _log->msg("SDP offer: ", sdp);
                    onOffer(rtc::Description(sdp, type));
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
        std::string s("Ping!");
        qlexnet::MessageWriter<MsgType> mw(msg);
        mw.writeString(s);
        mw.write(start);
        _log->msg("Ping...");
        send(msg);
    }

    private:
    int count = 0;
    uint16_t port;
    std::string hostIp;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("SignalerClient");
};

}