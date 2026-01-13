#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>
#include <optional>
#include <string>

#include "network/Signaler.h"
#include "utils.h"
#include "ISession.h"
#include "SessionThreaded.h"

namespace otherside {

// template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }

struct ClientConnection {
    ClientConnection(
        std::shared_ptr<rtc::PeerConnection> pc) : _peerConnection(pc) {};

    std::shared_ptr<rtc::DataChannel> dataChannel;
    const std::shared_ptr<rtc::PeerConnection> _peerConnection;

    void disconnect() {
        _peerConnection->close();
    }

    private:
    // const std::shared_ptr<qlexnet::Connection<MsgType>> _tcpConnection;
    // const std::shared_ptr<rtc::PeerConnection> _peerConnection;
};

class HostSession : public ISession, public SessionThreaded {
    public:
    HostSession(uint16_t port) {
        _ss = std::make_unique<SignalerServer>(port);
        _config.iceServers.clear();
        rtc::InitLogger(rtc::LogLevel::Debug);
        _ss->onRequest = [this](uint32_t clientId) {
            this->_clients.emplace(clientId, createPeerConnection(_config, clientId));
        };

        _ss->onReady = [this](uint32_t clientId, rtc::Description desc) {
            if (auto c = _clients.find(clientId); c != _clients.end()) {
                auto pc = c->second->_peerConnection;
                pc->setRemoteDescription(desc);
            }
        };
    }
    ~HostSession() { stop(); }

    void start() override {
        _log->msg("Start Signaler server");

        _ss->start();
        _signaling_thread = std::thread([this]{ while(true) _ss->update(-1, true); });
    }

    void stop() override {
        _ss->stop();
        if (_signaling_thread.joinable()) _signaling_thread.join();
        SessionThreaded::stop();
        for (auto const& [uid, pc] : _clients) {
            pc->disconnect();
        }
    }

    void update(float dt) override {}

    std::string statusText() const override {
        return "Hosting session";
    }

    private:
    void run() override {
        while(_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void onRequestClb(uint32_t clientId) {
        _clients.emplace(clientId, createPeerConnection(_config, clientId));
    }

    std::shared_ptr<ClientConnection> createPeerConnection(
        const rtc::Configuration& _config,
        uint32_t clientId
    ) {
        auto pc = std::make_shared<rtc::PeerConnection>(_config);
        auto client = std::make_shared<ClientConnection>(pc);

        pc->onStateChange([clientId, this](rtc::PeerConnection::State state) {
            _log->msg(clientId, " - State : ", state);
            if(state == rtc::PeerConnection::State::Connected) {
                if (auto c = _clients.find(clientId); c != _clients.end()) {
                    auto dc = c->second->dataChannel;
                    dc->send("ok?");
                }
            }
        });

        pc->onGatheringStateChange([clientId, wpc = make_weak_ptr(pc), this](rtc::PeerConnection::GatheringState state){
            _log->msg("Gathering State : ", state);
            if (state == rtc::PeerConnection::GatheringState::Complete) {
                if (auto pc = wpc.lock()) {
                    auto description = pc->localDescription();
                    qlexnet::Message<MsgType> msg;
                    msg.header.id = MsgType::OFFER;
                    qlexnet::MessageWriter mw(msg);
                    mw.writeString(description->typeString());
                    mw.writeString(description.value());
                    _ss->messageClient(clientId, msg);
                }
            }
        });

        auto dc = pc->createDataChannel("ping-pong");

        dc->onOpen([wdc = make_weak_ptr(dc)]() {
            if (auto dc = wdc.lock()) {
                dc->send("Ping");
            }
        });

        dc->onMessage(nullptr, [this, wdc = make_weak_ptr(dc)](std::string msg){
            _log->msg("onMessage : " + msg);
            if (auto dc = wdc.lock()) {
                dc->send("Ping");
            }
        });

        client->dataChannel = dc;
        pc->createOffer();
        return client;
    }

    std::thread _signaling_thread;
    std::unique_ptr<SignalerServer> _ss;
    rtc::Configuration _config;

    std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> _clients;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("Server");
};

}