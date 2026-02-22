#pragma once

#include <memory>
#include <rtc/rtc.hpp>

#include "media/VideoSender.h"
#include "message/DCMessageManager.h"

namespace otherside
{

struct ClientTrackData
{
    std::shared_ptr<rtc::Track> track;
    std::shared_ptr<rtc::RtcpSrReporter> sender;

    ClientTrackData(const std::shared_ptr<rtc::Track> &track_,
                    const std::shared_ptr<rtc::RtcpSrReporter> &sender_)
        : track(track_), sender(sender_)
    {
    }
};

struct ClientConnection
{
    ClientConnection(const std::shared_ptr<rtc::PeerConnection> &pc_) : pc(pc_) {};

    std::unique_ptr<HostDCMessageManager> dcm;
    const std::shared_ptr<rtc::PeerConnection> pc;
    std::unique_ptr<VideoSender> videoSender;
    std::shared_ptr<ClientTrackData> video;

    void disconnect()
    {
        pc->close();
    }

private:
    // const std::shared_ptr<qlexnet::Connection<MsgType>> _tcpConnection;
    // const std::shared_ptr<rtc::PeerConnection> _peerConnection;
};

} // namespace otherside