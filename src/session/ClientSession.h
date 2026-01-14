#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>
#include <string>

#include "ISession.h"
#include "SessionThreaded.h"
#include "network/SignalerClient.h"
#include "logger/Logger.h"
#include "utils.h"

namespace otherside {

class ClientSession : public ISession, public SessionThreaded {
    public:
    ClientSession(const std::string& ip_addr, uint16_t port) {
        _sc = std::make_unique<SignalerClient>(ip_addr, port);
        _sc->onOffer = [this](rtc::Description offer){
            _pc = createPeerConnection(offer);
        };
    }
    ~ClientSession() override { stop(); }

    void start() override {
        _log->msg("Send READY to Signaler");
        _sc->connect();
        _signaling_thread = std::thread([this]{ _sc->run(); });
        startThread();
        _log->msg("Set local SDP");
    }

    void stop() override {
        _sc->disconnect();
        if (_signaling_thread.joinable()) _signaling_thread.join();
        SessionThreaded::stop();
        if (_pc) _pc->close();
    }

    void update(float dt) override {

    }

    private:

    void run() override {
        while(_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            _sc->ping();
        }
    }

    std::shared_ptr<rtc::PeerConnection> createPeerConnection(rtc::Description offer) {
        rtc::Configuration config;
        config.iceServers.clear();
        auto pc = std::make_shared<rtc::PeerConnection>(config);

        pc->onStateChange([this](rtc::PeerConnection::State state) {
            _log->msg(" - State : ", state);
            if (state == rtc::PeerConnection::State::Connected) {
                _log->msg("should do smth?");
            }
        });

        pc->onGatheringStateChange([wpc = make_weak_ptr(pc), this](rtc::PeerConnection::GatheringState state){
            _log->msg("Gathering State : ", state);
            if (state == rtc::PeerConnection::GatheringState::Complete) {
                if (auto pc = wpc.lock()) {
                    auto description = pc->localDescription();
                    qlexnet::Message<MsgType> msg;
                    msg.header.id = MsgType::READY;
                    qlexnet::MessageWriter mw(msg);
                    mw.writeString(description->typeString());
                    mw.writeString(description.value());
                    _sc->send(msg);
                }
            }
        });

        pc->onTrack([this](std::shared_ptr<rtc::Track> tr){
            _log->msg("onTrack");
        });

        pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc){
            _log->msg("a data channel was received!");

            dc->onOpen([this, dc]() {_log->msg("THE DATA CHANNEL IS OPEN HERE!"); dc->send("Pong");});
            dc->onMessage(nullptr, [this, dc](std::string data){
                _log->msg("onMessage : " + data);
                dc->send("pong");
            });
            dc->onClosed([this]() {_log->msg("onClosed");});
        });
        pc->setRemoteDescription(offer);
        pc->createAnswer();
        return pc;
    }

    void sendAnswer() {
        _pc->setLocalDescription();
    }

    std::string statusText() const override {
        return "Connected as client";
    }

    std::thread _signaling_thread;
    std::unique_ptr<SignalerClient> _sc;
    std::shared_ptr<rtc::PeerConnection> _pc;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("Client");
};

}

