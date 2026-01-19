#include "HostSession.h"
#include "message/DCMessageManager.h"

namespace otherside
{

void HostSession::start()
{
    _log->msg("Start Signaler server");

    _ss->start();
    _signaling_thread = std::thread([this] { _ss->run(); });
    startThread();
}

void HostSession::stop()
{
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
        _log->msg("About to send ", msgs.size(), " messages.");
        for (auto msg : msgs)
        {
            sendMessage(msg);
        }
    }
}

void HostSession::onRequestClb(uint32_t clientId)
{
    auto cc = createClientConnection(_config, clientId);

    createDataChannels(cc);

    cc->_peerConnection->createOffer();
    _clients.emplace(clientId, cc);
}

void HostSession::createDataChannels(std::shared_ptr<ClientConnection> cc)
{
    cc->dcm->createChannel(DCMessageType::HEARTBEAT, true, true);
    cc->dcm->addOnMessageClb(DCMessageType::HEARTBEAT, [this](const UiMessage &msg) {
        _rxMessageFeed->push(msg);
        _txMessageFeed->push(UiMessage{DCMessageType::HEARTBEAT, PeerId::HOST, nowMs(), 0, "Ping"});
        return;
    });

    cc->dcm->createChannel(DCMessageType::MESSAGE, true, true);
    cc->dcm->addOnMessageClb(DCMessageType::MESSAGE, [this](const UiMessage &msg) {
        _rxMessageFeed->push(msg);
        return;
    });
}

std::shared_ptr<ClientConnection> HostSession::createClientConnection(const rtc::Configuration &_config,
                                                                      uint32_t clientId)
{
    auto pc = std::make_shared<rtc::PeerConnection>(_config);
    auto client = std::make_shared<ClientConnection>(pc);

    pc->onStateChange(
        [clientId, this](rtc::PeerConnection::State state) { _log->msg(clientId, " - State : ", state); });

    pc->onGatheringStateChange([clientId, wpc = make_weak_ptr(pc), this](rtc::PeerConnection::GatheringState state) {
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

void HostSession::sendMessage(UiMessage msg)
{
    auto bytes = serialize(msg);
    _clients.begin()->second->dcm->sendBinary(msg.type, bytes);
}

} // namespace otherside