#pragma once

#include <memory>
#include <optional>
#include <qlexnet.h>
#include <rtc/rtc.hpp>
#include <string>

#include "ClientConnection.h"
#include "ISession.h"
#include "media/FrameFeed.h"
#include "media/IVideoDecoder.h"
#include "media/IVideoSource.h"
#include "media/VideoSender.h"
#include "message/DCMessageManager.h"
#include "message/NetMessageFeed.h"
#include "network/SignalerServer.h"
#include "utils.h"

namespace otherside
{

class HostSession : public ISession, public ISessionControl
{
public:
    HostSession(uint16_t port, const std::shared_ptr<UiMessageFeed> &rxMessageFeed,
                const std::shared_ptr<UiMessageFeed> &txMessageFeed,
                const std::shared_ptr<FrameFeed> &rxFrameFeed_);
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

    std::shared_ptr<UiMessageFeed> _rxMessageFeed;
    std::shared_ptr<UiMessageFeed> _txMessageFeed;

    std::shared_ptr<FrameFeed> _rxFrameFeed;

    std::unique_ptr<IVideoDecoder> _videoDecoder;
    std::unique_ptr<IVideoSource> _source;

    std::thread _signaling_thread;
    std::unique_ptr<SignalerServer> _ss;
    rtc::Configuration _config;

    std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> _clients;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("HostSession");
};

} // namespace otherside
