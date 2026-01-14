#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>
#include <string>

#include "logger/Logger.h"
#include "SignalerMessageType.h"

namespace otherside {

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
