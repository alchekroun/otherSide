#pragma once

#include <optional>
#include <rtc/rtc.hpp>

#include "Frame.h"
#include "VideoEncoder.h"

namespace otherside
{

class VideoSender
{
public:
    VideoSender(const std::shared_ptr<rtc::PeerConnection> &pc_,
                std::unique_ptr<IVideoEncoder> encoder_)
        : _encoder(std::move(encoder_))
    {
        rtc::Description::Video media("video", rtc::Description::Direction::SendOnly);
        media.addH264Codec(96);

        _track = pc_->addTrack(media);
    }

    VideoSender(const std::shared_ptr<rtc::Track> &track_, std::unique_ptr<IVideoEncoder> encoder_)
        : _encoder(std::move(encoder_))
    {
        _track = track_;
    }

    void onFrame(const RawFrame &frame_)
    {
        auto encodedFrame = _encoder->encode(frame_);
        if (!encodedFrame)
        {
            return;
        }

        rtc::binary payload(encodedFrame->data);

        if (!_baseTsMs)
        {
            _baseTsMs = frame_.timestampMs;
        }
        auto elapsedMs = frame_.timestampMs - *_baseTsMs;
        auto seconds = std::chrono::duration<double>(static_cast<double>(elapsedMs) / 1000.0);
        _track->sendFrame(payload, rtc::FrameInfo(seconds));
    }

private:
    std::shared_ptr<rtc::Track> _track;
    std::unique_ptr<IVideoEncoder> _encoder;
    std::optional<utils::TimestampMs> _baseTsMs;
};

} // namespace otherside
