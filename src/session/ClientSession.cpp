#include "ClientSession.h"
#include "message/NetMessage.h"
#include "utils.h"

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
        for (const auto &msg : msgs)
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

void ClientSession::onOfferClb(const rtc::Description &offer)
{
    _pc = createPeerConnection(offer);
}

std::shared_ptr<rtc::PeerConnection> ClientSession::createPeerConnection(
    const rtc::Description &offer)
{
    rtc::Configuration config;
    config.iceServers.clear();
    auto pc = std::make_shared<rtc::PeerConnection>(config);
    _dcm = std::make_unique<ClientDCMessageManager>(pc);

    pc->onStateChange([this](rtc::PeerConnection::State state) { _log->msg("State : ", state); });

    pc->onGatheringStateChange(
        [wpc = utils::make_weak_ptr(pc), this](rtc::PeerConnection::GatheringState state) {
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

    pc->onDataChannel([this](const std::shared_ptr<rtc::DataChannel> &dc) {
        _log->msg("Data Channel (", dc->label(), ") received.");
        switch (_dcm->labelToType[dc->label()])
        {
        case DCMessageType::HEARTBEAT:
            _dcm->assignChannel(DCMessageType::HEARTBEAT, dc);
            _dcm->addOnMessageClb(DCMessageType::HEARTBEAT, [this](const UiMessage &msg) {
                _rxMessageFeed->push(msg);
                _txMessageFeed->push(
                    UiMessage{DCMessageType::HEARTBEAT, PeerId::CLIENT, utils::nowMs(), 0, "Ping"});
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
    uint8_t payloadType = 96;
    auto recvVideo = rtc::Description::Video("video");
    recvVideo.setDirection(rtc::Description::Direction::RecvOnly);
    recvVideo.addH264Codec(payloadType);
    _track = pc->addTrack(recvVideo);
    auto session = std::make_shared<rtc::RtcpReceivingSession>();
    _track->setMediaHandler(session);
    _track->onOpen([this]() { _log->msg("Video track open."); });
    _track->onClosed([this]() { _log->msg("Video track closed."); });
    _track->onError([this](std::string e) { _log->msg("Video track error: ", e); });
    _track->onMessage([this](rtc::message_variant rawMsg) {
        if (std::holds_alternative<rtc::binary>(rawMsg))
        {
            const auto &pkt = std::get<rtc::binary>(rawMsg);
            _vd->pushEncodedFrame(reinterpret_cast<const uint8_t *>(pkt.data()), pkt.size());
        }
    });

    pc->setRemoteDescription(offer);
    pc->createAnswer();
    return pc;
}

void ClientSession::sendMessage(const UiMessage &msg)
{
    auto bytes = serialize(msg);
    _log->msg("Sending : ", msg.text);
    _dcm->sendBinary(msg.type, bytes);
}

} // namespace otherside
