#pragma once

#include "NetMessage.h"
#include "logger/Logger.h"
#include <rtc/rtc.hpp>
#include <unordered_set>

namespace otherside
{

class DCMessageManager
{
    using MsgClb = std::function<void(const UiMessage &)>;

  public:
    void send(DCMessageType type, const std::string &message);
    void sendBinary(DCMessageType type, const std::vector<uint8_t> &data);

    void addOnMessageClb(DCMessageType type, MsgClb callback)
    {
        _msgClbByChannelType[type] = callback;
    }

    std::unordered_map<std::string, DCMessageType> labelToType = {{"Heartbeat", DCMessageType::HEARTBEAT},
                                                                  {"Message", DCMessageType::MESSAGE}};

  protected:
    DCMessageManager(std::shared_ptr<rtc::PeerConnection> pc_) : _pc(pc_)
    {
    }

    std::unordered_map<DCMessageType, std::string> typeToLabel = {{DCMessageType::HEARTBEAT, "Heartbeat"},
                                                                  {DCMessageType::MESSAGE, "Message"}};
    std::shared_ptr<rtc::PeerConnection> _pc;
    std::unordered_map<DCMessageType, std::shared_ptr<rtc::DataChannel>> _channels;
    std::unordered_set<DCMessageType> _openChannels;
    std::unordered_map<DCMessageType, MsgClb> _msgClbByChannelType;
    std::unique_ptr<Logger> _log;
};

class HostDCMessageManager : public DCMessageManager
{
  public:
    HostDCMessageManager(std::shared_ptr<rtc::PeerConnection> pc_) : DCMessageManager(pc_)
    {
    }

    void createChannel(DCMessageType type, bool reliable, bool ordered);

  private:
    std::unique_ptr<Logger> _log = std::make_unique<Logger>("HostDCMessageManager");
};

class ClientDCMessageManager : public DCMessageManager
{
  public:
    ClientDCMessageManager(std::shared_ptr<rtc::PeerConnection> pc_) : DCMessageManager(pc_)
    {
    }

    void assignChannel(DCMessageType type, std::shared_ptr<rtc::DataChannel> dc);

  private:
    std::unique_ptr<Logger> _log = std::make_unique<Logger>("ClientDCMessageManager");
};

} // namespace otherside