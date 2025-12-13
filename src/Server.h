#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>

#include "Signaler.h"
#include "MessageTypes.h"


namespace otherside
{
template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }

class Server {
    public:
    Server(uint16_t port) {
        sc = std::make_unique<SignalerServer>(port);
    }
    ~Server() { stop(); }

    bool start() {
        _log->msg("Start Signaler server");

        sc->start();
        signaling_thread = std::thread([this]{
            while(true) sc->update(-1, true);
        });
        std::this_thread::sleep_for(std::chrono::seconds(5));
        initPeerConnection();
        std::this_thread::sleep_for(std::chrono::seconds(30));
        return true;
    }

    void stop() {
        sc->stop();
    }

    private:

    void initPeerConnection() {
        _log->msg("Initialize peer connection");

        rtc::Configuration config;
        config.iceServers.clear();
        rtc::InitLogger(rtc::LogLevel::Debug);
        pc = std::make_unique<rtc::PeerConnection>(config);

        pc->onStateChange([this](rtc::PeerConnection::State state){
            _log->msg("PC State : ", state);
        });

        pc->onGatheringStateChange(
            [this] (rtc::PeerConnection::GatheringState state) {
                _log->msg("PC Gathering State : ", state);

                if (state == rtc::PeerConnection::GatheringState::Complete) {
                    auto sdp = pc->localDescription();
                    qlexnet::Message<MsgType> msg;
                    msg.header.id = MsgType::OFFER;
                    msg << sdp->typeString();
                    msg << sdp.value();
                    std::cout << sdp->typeString() << std::endl;
                    std::cout << sdp.value() << std::endl;
                    sc->messageAllClients(msg);
                }
            }
        );
        dc = pc->createDataChannel("SERVER");

        _log->msg("Local SDP about to be set");
        pc->setLocalDescription();
    }

    std::thread signaling_thread;
    std::unique_ptr<SignalerServer> sc;
    std::shared_ptr<rtc::DataChannel> dc;
    std::unique_ptr<rtc::PeerConnection> pc;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("Server");
};

}