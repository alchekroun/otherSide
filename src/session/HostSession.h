#pragma once

#include <memory>
#include <optional>
#include <qlexnet.h>
#include <rtc/rtc.hpp>
#include <string>

#include "ISession.h"
#include "message/DCMessageManager.h"
#include "message/NetMessageFeed.h"
#include "network/SignalerServer.h"
#include "utils.h"

namespace otherside
{

// template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }

struct ClientConnection
{
    ClientConnection(std::shared_ptr<rtc::PeerConnection> pc) : _peerConnection(pc) {};

    std::unique_ptr<HostDCMessageManager> dcm;
    const std::shared_ptr<rtc::PeerConnection> _peerConnection;

    void disconnect()
    {
        _peerConnection->close();
    }

  private:
    // const std::shared_ptr<qlexnet::Connection<MsgType>> _tcpConnection;
    // const std::shared_ptr<rtc::PeerConnection> _peerConnection;
};

class HostSession : public ISession, public ISessionControl
{
  public:
    HostSession(uint16_t port, std::shared_ptr<UiMessageFeed> rxMessageFeed,
                std::shared_ptr<UiMessageFeed> txMessageFeed)
        : _rxMessageFeed(rxMessageFeed), _txMessageFeed(txMessageFeed)
    {
        _ss = std::make_unique<SignalerServer>(port);
        _config.iceServers.clear();
        rtc::InitLogger(rtc::LogLevel::Debug);
        _ss->onRequest = [this](uint32_t clientId) { onRequestClb(clientId); };

        _ss->onReady = [this](uint32_t clientId, rtc::Description desc) {
            if (auto c = _clients.find(clientId); c != _clients.end())
            {
                auto pc = c->second->_peerConnection;
                pc->setRemoteDescription(desc);
            }
        };
    }
    ~HostSession()
    {
        stop();
    }

    void start() override;
    void stop() override;
    void update(float dt) override;

    void sendMessage(UiMessage msg) override;

  private:
    void run() override;

    void onRequestClb(uint32_t clientId);
    std::shared_ptr<ClientConnection> createClientConnection(const rtc::Configuration &_config, uint32_t clientId);
    void createDataChannels(std::shared_ptr<ClientConnection> cc);

    std::shared_ptr<UiMessageFeed> _rxMessageFeed;
    std::shared_ptr<UiMessageFeed> _txMessageFeed;

    std::thread _signaling_thread;
    std::unique_ptr<SignalerServer> _ss;
    rtc::Configuration _config;

    std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> _clients;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("HostSession");
};

} // namespace otherside