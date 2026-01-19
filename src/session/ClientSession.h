#pragma once

#include <memory>
#include <qlexnet.h>
#include <rtc/rtc.hpp>
#include <string>

#include "ISession.h"
#include "logger/Logger.h"
#include "message/DCMessageManager.h"
#include "message/NetMessageFeed.h"
#include "network/SignalerClient.h"
#include "utils.h"

namespace otherside
{

class ClientSession : public ISession, public ISessionControl
{
  public:
    ClientSession(const std::string &ip_addr, uint16_t port, const std::shared_ptr<UiMessageFeed> &rxMessageFeed,
                  const std::shared_ptr<UiMessageFeed> &txMessageFeed)
        : _rxMessageFeed(rxMessageFeed), _txMessageFeed(txMessageFeed)
    {
        _sc = std::make_unique<SignalerClient>(ip_addr, port);
        _sc->onOffer = [this](const rtc::Description &offer) { onOfferClb(offer); };
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

    std::thread _signaling_thread;
    std::unique_ptr<SignalerClient> _sc;
    std::shared_ptr<rtc::PeerConnection> _pc;
    std::unique_ptr<ClientDCMessageManager> _dcm;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("ClientSession");
};

} // namespace otherside
