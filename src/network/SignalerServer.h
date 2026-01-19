#pragma once

#include <memory>
#include <qlexnet.h>
#include <rtc/rtc.hpp>
#include <string>

#include "SignalerMessageType.h"
#include "logger/Logger.h"

namespace otherside
{
using asio::ip::tcp;

class SignalerServer : public qlexnet::ServerInterface<MsgType>
{
  public:
    SignalerServer(uint16_t port) : qlexnet::ServerInterface<MsgType>(port), running(true)
    {
    }

    ~SignalerServer() override
    {
        running = false;
        stop();
    }

    std::function<void(uint32_t)> onRequest;
    std::function<void(uint32_t, rtc::Description)> onReady;

    void run()
    {
        while (running)
        {
            update(-1, true);
        }
    }

    std::atomic<bool> running{false};

  protected:
    bool onClientConnect(std::shared_ptr<qlexnet::Connection<MsgType>> client_) override
    {
        qlexnet::Message<MsgType> msg;
        msg.header.id = MsgType::ACK_CONNECTION;
        client_->send(msg);
        return true;
    }

    void onClientDisconnect(std::shared_ptr<qlexnet::Connection<MsgType>> client_) override
    {
        _log->msg("Client disconnected");
    }

    void onMessage(std::shared_ptr<qlexnet::Connection<MsgType>> client_, qlexnet::Message<MsgType> &msg_) override
    {
        switch (msg_.header.id)
        {
        case MsgType::PING: {
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
        case MsgType::REQUEST: {
            onRequest(client_->GetID());
            break;
        }
        case MsgType::READY: {
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

} // namespace otherside