#include "DCMessageManager.h"
#include "utils.h"

namespace otherside
{

void DCMessageManager::send(DCMessageType type, const std::string &message)
{
    if (_openChannels.contains(type) && _channels.contains(type))
    {
        _channels[type]->send(message);
    }
}

void DCMessageManager::sendBinary(DCMessageType type, const std::vector<uint8_t> &data)
{
    if (_openChannels.contains(type) && _channels.contains(type))
    {
        _channels[type]->sendBuffer(data);
    }
}

void HostDCMessageManager::createChannel(DCMessageType type, bool reliable, bool ordered)
{
    rtc::DataChannelInit cfg;
    rtc::Reliability rel;
    rel.unordered = !ordered;
    if (!reliable)
    {
        rel.maxRetransmits = 0;
    }
    cfg.reliability = rel;

    auto channel = _pc->createDataChannel(typeToLabel[type], cfg);

    channel->onOpen([this, type]() {
        _log->msg("DataChanel ", typeToLabel[type], " open.");
        _openChannels.insert(type);
    });

    channel->onClosed([this, type]() {
        _log->msg("DataChanel ", typeToLabel[type], " closed.");
        _openChannels.erase(type);
    });

    channel->onMessage([this, type](const rtc::message_variant &rawMsg) {
        if (!_msgClbByChannelType.contains(type))
        {
            return;
        }
        if (std::holds_alternative<rtc::string>(rawMsg))
        {
            _log->msg("Received string message. Prefer own protocol.");
            _log->msg("Message content : ", std::get<std::string>(rawMsg));
        }
        else
        {
            auto msg = deserialize(std::get<rtc::binary>(rawMsg));
            msg.receivedTs = utils::nowMs();
            _msgClbByChannelType[type](msg);
        }
    });

    channel->onError([type](const std::string &err) { std::cerr << "DC ERROR : " << err << "\n"; });

    _channels[type] = channel;
}

void ClientDCMessageManager::assignChannel(DCMessageType type,
                                           const std::shared_ptr<rtc::DataChannel> &dc)
{

    dc->onOpen([this, type]() {
        _log->msg("DataChanel ", typeToLabel[type], " open.");
        _openChannels.insert(type);
    });

    dc->onClosed([this, type]() {
        _log->msg("DataChanel ", typeToLabel[type], " closed.");
        _openChannels.erase(type);
    });

    dc->onMessage([this, type](const std::variant<rtc::binary, rtc::string> &rawMsg) {
        if (!_msgClbByChannelType.contains(type))
        {
            return;
        }
        if (std::holds_alternative<rtc::string>(rawMsg))
        {
            _log->msg("Received string message. Prefer own protocol.");
            _log->msg("Message content : ", std::get<std::string>(rawMsg));
        }
        else
        {
            auto msg = deserialize(std::get<rtc::binary>(rawMsg));
            msg.receivedTs = utils::nowMs();
            _msgClbByChannelType[type](msg);
        }
    });

    dc->onError([type](const std::string &err) { std::cerr << "DC ERROR : " << err << "\n"; });

    _channels[type] = dc;
}

} // namespace otherside