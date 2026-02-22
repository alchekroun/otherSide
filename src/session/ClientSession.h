#pragma once

#include <cstring>
#include <memory>
#include <qlexnet.h>
#include <rtc/rtc.hpp>
#include <string>

#include "ISession.h"
#include "logger/Logger.h"
#include "media/Frame.h"
#include "media/FrameFeed.h"
#include "media/VideoDecoder.h"
#include "message/DCMessageManager.h"
#include "message/NetMessageFeed.h"
#include "network/SignalerClient.h"
#include "utils.h"

namespace otherside
{

class ClientSession : public ISession, public ISessionControl
{
public:
    ClientSession(const std::string &ip_addr, uint16_t port,
                  const std::shared_ptr<UiMessageFeed> &rxMessageFeed,
                  const std::shared_ptr<UiMessageFeed> &txMessageFeed,
                  const std::shared_ptr<FrameFeed> &frameFeed_)
        : _rxMessageFeed(rxMessageFeed), _txMessageFeed(txMessageFeed), _frameFeed(frameFeed_)
    {
        _sc = std::make_unique<SignalerClient>(ip_addr, port);
        _sc->onOffer = [this](const rtc::Description &offer) { onOfferClb(offer); };
        _vd = std::make_unique<VideoDecoder>();
        _vd->setFrameCallback([this](const uint8_t *rgba, int w, int h) {
            RawFrame frame;
            frame.width = static_cast<uint32_t>(w);
            frame.height = static_cast<uint32_t>(h);
            frame.format = PixelFormat::RGBA;
            frame.timestampMs = utils::nowMs();
            frame.data.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
            std::memcpy(frame.data.data(), rgba, frame.data.size());
            _frameFeed->push(frame);
        });
    }
    ~ClientSession() override
    {
        stop();
    }

    void start() override;
    void stop() override;
    void update(float dt) override;

    // void sendText(std::string_view) override;
    void sendMessage(const UiMessage &msg) override;

private:
    void run() override;
    void onOfferClb(const rtc::Description &offer);
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(const rtc::Description &offer);

    void sendAnswer()
    {
        _pc->setLocalDescription();
    }

    std::shared_ptr<UiMessageFeed> _rxMessageFeed;
    std::shared_ptr<UiMessageFeed> _txMessageFeed;

    std::shared_ptr<FrameFeed> _frameFeed;

    std::thread _signaling_thread;
    std::unique_ptr<SignalerClient> _sc;
    std::shared_ptr<rtc::PeerConnection> _pc;
    std::unique_ptr<ClientDCMessageManager> _dcm;
    std::shared_ptr<rtc::Track> _track;
    std::unique_ptr<VideoDecoder> _vd;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("ClientSession");
};

} // namespace otherside
