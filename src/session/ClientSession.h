#pragma once

#include <rtc/rtc.hpp>
#include <qlexnet.h>
#include <memory>
#include <string>

#include "ISession.h"
#include "network/SignalerClient.h"
#include "message/NetMessageFeed.h"
#include "message/DCMessageManager.h"
#include "logger/Logger.h"
#include "utils.h"

namespace otherside {

class ClientSession : public ISession, public ISessionControl {
    public:
    ClientSession(
        const std::string& ip_addr,
        uint16_t port,
        std::shared_ptr<UiMessageFeed> rxMessageFeed,
        std::shared_ptr<UiMessageFeed> txMessageFeed
    ) :
    _rxMessageFeed(rxMessageFeed),
    _txMessageFeed(txMessageFeed)
    {
        _sc = std::make_unique<SignalerClient>(ip_addr, port);
        _sc->onOffer = [this](rtc::Description offer){ onOfferClb(offer); };
    }
    ~ClientSession() override { stop(); }

    void start() override;
    void stop() override;
    void update(float dt) override;

    // void sendText(std::string_view) override;
    void sendMessage(UiMessage msg) override;

    private:
    void run() override;
    void onOfferClb(rtc::Description offer);
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(rtc::Description offer);

    void sendAnswer() {
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

}

