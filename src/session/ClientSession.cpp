#include "ClientSession.h"
#include "media/FakeVideoSource.h"
#include "media/H264/H264Decoder.h"
#include "media/H264/H264Encoder.h"
#include "message/NetMessage.h"
#include "session/TrackSetup.h"
#include "utils.h"

namespace otherside
{

ClientSession::ClientSession(const std::string &ip_addr, uint16_t port,
                             const std::shared_ptr<UiMessageFeed> &rxMessageFeed,
                             const std::shared_ptr<UiMessageFeed> &txMessageFeed,
                             const std::shared_ptr<FrameFeed> &rxFrameFeed_)
    : _rxMessageFeed(rxMessageFeed), _txMessageFeed(txMessageFeed), _rxFrameFeed(rxFrameFeed_)
{
    _sc = std::make_unique<SignalerClient>(ip_addr, port);
    _sc->onOffer = [this](const rtc::Description &offer) { onOfferClb(offer); };
    _videoDecoder = std::make_unique<H264Decoder>();
    _videoDecoder->setFrameCallback([this](const uint8_t *rgba, int w, int h) {
        RawFrame frame;
        frame.width = static_cast<uint32_t>(w);
        frame.height = static_cast<uint32_t>(h);
        frame.format = PixelFormat::RGBA;
        frame.timestampMs = utils::nowMs();
        frame.data.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
        std::memcpy(frame.data.data(), rgba, frame.data.size());
        _rxFrameFeed->push(frame);
    });
    _source = std::make_unique<FakeVideoSource>(640, 480, 24);
}

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
    auto tx = makeH264TxTrack(pc, 96, 2, "client-video", "client-stream",
                              rtc::Description::Direction::SendOnly, [this]() {
                                  _log->msg("client tx track open.");
                                  _source->start();
                              });
    _txVideoSender =
        std::make_shared<VideoSender>(tx.track, std::make_unique<H264Encoder>(640, 480, 24));
    _source->setSink([sender = _txVideoSender](const RawFrame &frame) { sender->onFrame(frame); });

    _track = makeH264RxTrack(pc, 96, "host-video", rtc::Description::Direction::RecvOnly,
                             [this](const rtc::binary &pkt) {
                                 _videoDecoder->decode(
                                     reinterpret_cast<const uint8_t *>(pkt.data()), pkt.size());
                             });
    _track->onOpen([this]() { _log->msg("client rx track open."); });
    _track->onClosed([this]() { _log->msg("client rx track closed."); });
    _track->onError([this](std::string e) { _log->msg("client rx track error: ", e); });

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
