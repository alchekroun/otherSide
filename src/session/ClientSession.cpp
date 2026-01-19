#include "ClientSession.h"
#include "message/NetMessage.h"

namespace otherside
{

void ClientSession::start()
{
    _sc->connect();
    _signaling_thread = std::thread([this] { _sc->run(); });
    startThread();
}

void ClientSession::stop()
{
    _sc->disconnect();
    if (_signaling_thread.joinable())
    {
        _signaling_thread.join();
    }
    SessionThreaded::stop();
    if (_pc)
    {
        _pc->close();
    }
}

void ClientSession::update(float dt)
{
    while (!_txMessageFeed->empty())
    {
        auto msgs = _txMessageFeed->consume();
        for (auto msg : msgs)
        {
            sendMessage(msg);
        }
    }
}

void ClientSession::run()
{
    while (_running)
    {
        _sc->ping();
        update(1.0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ClientSession::onOfferClb(rtc::Description offer)
{
    _pc = createPeerConnection(offer);
    _dcm = std::make_unique<ClientDCMessageManager>(_pc);
}

std::shared_ptr<rtc::PeerConnection> ClientSession::createPeerConnection(rtc::Description offer)
{
    rtc::Configuration config;
    config.iceServers.clear();
    auto pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onStateChange([this](rtc::PeerConnection::State state) { _log->msg("State : ", state); });

    pc->onGatheringStateChange([wpc = make_weak_ptr(pc), this](rtc::PeerConnection::GatheringState state) {
        _log->msg("Gathering State : ", state);
        if (state == rtc::PeerConnection::GatheringState::Complete)
        {
            if (auto pc = wpc.lock())
            {
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

    pc->onTrack([this](std::shared_ptr<rtc::Track> tr) { _log->msg("onTrack"); });

    pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
        _log->msg("Data Channel (", dc->label(), ") received.");
        switch (_dcm->labelToType[dc->label()])
        {
        case DCMessageType::HEARTBEAT:
            _dcm->assignChannel(DCMessageType::HEARTBEAT, dc);
            _dcm->addOnMessageClb(DCMessageType::HEARTBEAT, [this](const UiMessage &msg) {
                _rxMessageFeed->push(msg);
                _txMessageFeed->push(UiMessage{DCMessageType::HEARTBEAT, PeerId::CLIENT, nowMs(), 0, "Ping"});
                return;
            });
            break;
        case DCMessageType::MESSAGE:
            _dcm->assignChannel(DCMessageType::MESSAGE, dc);
            _dcm->addOnMessageClb(DCMessageType::MESSAGE, [this](const UiMessage &msg) {
                _rxMessageFeed->push(msg);
                return;
            });
            break;
        default:
            break;
        }
    });
    pc->setRemoteDescription(offer);
    pc->createAnswer();
    return pc;
}

void ClientSession::sendMessage(UiMessage msg)
{
    auto bytes = serialize(msg);
    _dcm->sendBinary(msg.type, bytes);
}

} // namespace otherside