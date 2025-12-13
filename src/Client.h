#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>

#include "Signaler.h"
#include "MessageTypes.h"
#include "Logger.h"

namespace otherside
{

class Client {
    public:
    Client(const std::string& ip_addr, uint16_t port) {
        sc = std::make_unique<SignalerClient>(ip_addr, port);
    }
    ~Client() { stop(); }

    bool start() {
        _log->msg("Send READY to Signaler");

        sc->connect();
        signaling_thread = std::thread([this]{sc->run();});
        while(true) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            sc->ping();
        }

        _log->msg("Set local SDP");
        return true;
    }

    void stop() {
        sc->disconnect();
        if (signaling_thread.joinable()) signaling_thread.join();
    }

    private:
    std::thread signaling_thread;
    std::unique_ptr<SignalerClient> sc;
    std::unique_ptr<rtc::DataChannel> dc;
    std::unique_ptr<rtc::PeerConnection> pc;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("Client");
};

}
