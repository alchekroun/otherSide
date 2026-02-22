#include "HostSession.h"
#include "media/FakeVideoSource.h"
#include "media/H264/H264Decoder.h"
#include "media/H264/H264Encoder.h"
#include "message/DCMessageManager.h"
#include "session/TrackSetup.h"
#include "utils.h"

namespace otherside
{

HostSession::HostSession(uint16_t port, const std::shared_ptr<UiMessageFeed> &rxMessageFeed,
                         const std::shared_ptr<UiMessageFeed> &txMessageFeed,
                         const std::shared_ptr<FrameFeed> &rxFrameFeed_)
    : _rxMessageFeed(rxMessageFeed), _txMessageFeed(txMessageFeed), _rxFrameFeed(rxFrameFeed_)
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

void HostSession::start()
{
    _log->msg("Start Signaler server");

    _ss->start();
    _signaling_thread = std::thread([this] { _ss->run(); });
    startThread();
}

void HostSession::stop()
{
    _source->stop();
    _ss->running = false;
    _ss->stop();
    if (_signaling_thread.joinable())
    {
        _signaling_thread.join();
    }
    SessionThreaded::stop();
    for (auto const &[uid, pc] : _clients)
    {
        pc->disconnect();
    }
}

void HostSession::run()
{
    while (_running)
    {
        update(1.0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void HostSession::update(float dt)
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

void HostSession::onRequestClb(uint32_t clientId)
{
    auto cc = createClientConnection(_config, clientId);

    auto tx = makeH264TxTrack(cc->pc, 96, 1, "host-video", "host-stream",
                              rtc::Description::Direction::SendOnly,
                              [this]() {
                                  _log->msg("host tx track open.");
                                  _source->start();
                              });
    cc->txVideo = std::make_shared<ClientTrackData>(tx.track, tx.senderReport);
    cc->txVideoSender =
        std::make_unique<VideoSender>(cc->txVideo->track, std::make_unique<H264Encoder>(640, 480, 24));
    _source->setSink([cc](const RawFrame &frame) { cc->txVideoSender->onFrame(frame); });

    cc->rxVideoTrack = makeH264RxTrack(cc->pc, 96, "client-video",
                                       rtc::Description::Direction::RecvOnly,
                                       [this](const rtc::binary &pkt) {
                                           _videoDecoder->decode(
                                               reinterpret_cast<const uint8_t *>(pkt.data()),
                                               pkt.size());
                                       });
    cc->rxVideoTrack->onOpen([this]() { _log->msg("host rx track open."); });

    createDataChannels(cc);

    cc->pc->createOffer();
    _clients.emplace(clientId, cc);
}

void HostSession::createDataChannels(const std::shared_ptr<ClientConnection> &cc)
{
    cc->dcm->createChannel(DCMessageType::HEARTBEAT, true, true);
    cc->dcm->addOnMessageClb(DCMessageType::HEARTBEAT, [this](const UiMessage &msg) {
        _rxMessageFeed->push(msg);
        _txMessageFeed->push(
            UiMessage{DCMessageType::HEARTBEAT, PeerId::HOST, utils::nowMs(), 0, "Ping"});
        return;
    });

    cc->dcm->createChannel(DCMessageType::MESSAGE, true, true);
    cc->dcm->addOnMessageClb(DCMessageType::MESSAGE, [this](const UiMessage &msg) {
        _rxMessageFeed->push(msg);
        return;
    });
}

std::shared_ptr<ClientConnection> HostSession::createClientConnection(
    const rtc::Configuration &_config, uint32_t clientId)
{
    auto pc = std::make_shared<rtc::PeerConnection>(_config);
    auto client = std::make_shared<ClientConnection>(pc);

    pc->onStateChange([clientId, this](rtc::PeerConnection::State state) {
        _log->msg(clientId, " - State : ", state);
        if (state == rtc::PeerConnection::State::Connected)
        {
        }
    });

    pc->onGatheringStateChange([clientId, wpc = utils::make_weak_ptr(pc),
                                this](rtc::PeerConnection::GatheringState state) {
        _log->msg("Gathering State : ", state);
        if (state == rtc::PeerConnection::GatheringState::Complete)
        {
            if (auto pc = wpc.lock())
            {
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

    client->dcm = std::make_unique<HostDCMessageManager>(pc);
    return client;
}

void HostSession::sendMessage(const UiMessage &msg)
{
    auto bytes = serialize(msg);
    _log->msg("Sending : ", msg.text);
    _clients.begin()->second->dcm->sendBinary(msg.type, bytes);
}

} // namespace otherside
