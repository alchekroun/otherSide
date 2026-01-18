#include "DCMessageManager.h"

namespace otherside {

void DCMessageManager::send(DCMessageType type, const std::string& message) {
    if (_openChannels.count(type) != 0 && _channels.count(type) != 0) {
        _channels[type]->send(message);
    }
}

void DCMessageManager::sendBinary(DCMessageType type, const std::vector<uint8_t>& data) {
    if (_openChannels.count(type) != 0 && _channels.count(type) != 0) {
        _channels[type]->sendBuffer(data);
    }
}

void HostDCMessageManager::createChannel(DCMessageType type, bool reliable, bool ordered) {
    rtc::DataChannelInit cfg;
    rtc::Reliability rel;
    rel.unordered = !ordered;
    if (!reliable) {
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

    channel->onMessage([this, type](const std::variant<rtc::binary, rtc::string> &msg) {
        if (_msgClbByChannelType.count(type) == 0) return;
        if (std::holds_alternative<rtc::string>(msg)) {
            _msgClbByChannelType[type](std::get<std::string>(msg));
        } else {
            auto msgBytes = std::get<rtc::binary>(msg);
            // auto data = std::string(msgBytes.begin(), msgBytes.end());
            // _msgClbByChannelType[type](data);
        }
    });

    channel->onError([type](const std::string& err) {
        std::cerr << "DC ERROR : " << err << std::endl;
    });

    _channels[type] = channel;
}

void ClientDCMessageManager::assignChannel(DCMessageType type, std::shared_ptr<rtc::DataChannel> dc) {

    dc->onOpen([this, type]() {
        _log->msg("DataChanel ", typeToLabel[type], " open.");
        _openChannels.insert(type);
    });

    dc->onClosed([this, type]() {
        _log->msg("DataChanel ", typeToLabel[type], " closed.");
        _openChannels.erase(type);
    });

    dc->onMessage([this, type](const std::variant<rtc::binary, rtc::string> &msg) {
        if (_msgClbByChannelType.count(type) == 0) return;
        if (std::holds_alternative<rtc::string>(msg)) {
            _msgClbByChannelType[type](std::get<std::string>(msg));
        } else {
            auto msgBytes = std::get<rtc::binary>(msg);
            // auto data = std::string(msgBytes.begin(), msgBytes.end());
            // _msgClbByChannelType[type](data);
        }
    });

    dc->onError([type](const std::string& err) {
        std::cerr << "DC ERROR : " << err << std::endl;
    });

    _channels[type] = dc;

}

}