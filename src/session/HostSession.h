#pragma once

#include <memory>
#include <optional>
#include <qlexnet.h>
#include <rtc/rtc.hpp>
#include <string>

#include "ClientConnection.h"
#include "ISession.h"
#include "message/DCMessageManager.h"
#include "message/NetMessageFeed.h"
#include "network/SignalerServer.h"
#include "utils.h"
#include <FFmpegH264Encoder.h>
#include <VideoSender.h>
#include <VideoSource.h>

namespace otherside
{

class HostSession : public ISession, public ISessionControl
{
public:
    HostSession(uint16_t port, const std::shared_ptr<UiMessageFeed> &rxMessageFeed,
                const std::shared_ptr<UiMessageFeed> &txMessageFeed)
        : _rxMessageFeed(rxMessageFeed), _txMessageFeed(txMessageFeed)
    {
        _ss = std::make_unique<SignalerServer>(port);
        _config.iceServers.clear();
        rtc::InitLogger(rtc::LogLevel::Debug);
        _ss->onRequest = [this](uint32_t clientId) { onRequestClb(clientId); };

        _ss->onReady = [this](uint32_t clientId, const rtc::Description &desc) {
            if (auto c = _clients.find(clientId); c != _clients.end())
            {
                auto pc = c->second->pc;
                pc->setRemoteDescription(desc);
            }
        };
        _source = std::make_unique<FakeVideoSource>(640, 480, 24);
    }
    ~HostSession() override
    {
        stop();
    }

    void start() override;
    void stop() override;
    void update(float dt) override;

    void sendMessage(const UiMessage &msg) override;

private:
    void run() override;

    void onRequestClb(uint32_t clientId);
    std::shared_ptr<ClientConnection> createClientConnection(const rtc::Configuration &_config,
                                                             uint32_t clientId);
    void createDataChannels(const std::shared_ptr<ClientConnection> &cc);
    void createTrack(const std::shared_ptr<ClientConnection> &cc);

    std::shared_ptr<UiMessageFeed> _rxMessageFeed;
    std::shared_ptr<UiMessageFeed> _txMessageFeed;

    std::unique_ptr<FakeVideoSource> _source;

    std::thread _signaling_thread;
    std::unique_ptr<SignalerServer> _ss;
    rtc::Configuration _config;

    std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> _clients;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("HostSession");
};

} // namespace otherside
