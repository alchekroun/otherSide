#pragma once

#include <functional>
#include <memory>
#include <string>

#include <rtc/rtc.hpp>

namespace otherside
{

struct TxTrackSetup
{
    std::shared_ptr<rtc::Track> track;
    std::shared_ptr<rtc::RtcpSrReporter> senderReport;
};

inline TxTrackSetup makeH264TxTrack(const std::shared_ptr<rtc::PeerConnection> &pc,
                                    uint8_t payloadType, uint32_t ssrc, const std::string &cname,
                                    const std::string &msid, rtc::Description::Direction direction,
                                    std::function<void()> onOpen = {})
{
    auto media = rtc::Description::Video(cname);
    media.setDirection(direction);
    media.addH264Codec(payloadType);
    media.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(media);

    auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
        ssrc, cname, payloadType, rtc::H264RtpPacketizer::ClockRate);
    auto packetizer =
        std::make_shared<rtc::H264RtpPacketizer>(rtc::NalUnit::Separator::StartSequence, rtpConfig);
    auto senderReport = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(senderReport);
    packetizer->addToChain(std::make_shared<rtc::RtcpNackResponder>());
    track->setMediaHandler(packetizer);

    if (onOpen)
    {
        track->onOpen(std::move(onOpen));
    }

    return {track, senderReport};
}

inline std::shared_ptr<rtc::Track> makeH264RxTrack(
    const std::shared_ptr<rtc::PeerConnection> &pc, uint8_t payloadType, const std::string &cname,
    rtc::Description::Direction direction, std::function<void(const rtc::binary &)> onPacket)
{
    auto media = rtc::Description::Video(cname);
    media.setDirection(direction);
    media.addH264Codec(payloadType);
    auto track = pc->addTrack(media);

    auto depacketizer =
        std::make_shared<rtc::H264RtpDepacketizer>(rtc::NalUnit::Separator::StartSequence);
    auto receivingSession = std::make_shared<rtc::RtcpReceivingSession>();
    depacketizer->addToChain(receivingSession);
    track->setMediaHandler(depacketizer);

    track->onFrame([onPacket = std::move(onPacket)](const rtc::binary &pkt, rtc::FrameInfo) {
        onPacket(pkt);
    });

    return track;
}

} // namespace otherside
