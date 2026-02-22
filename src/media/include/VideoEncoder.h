#pragma once

#include <rtc/rtc.hpp>

#include "Frame.h"

namespace otherside
{

class IVideoEncoder
{
public:
    virtual ~IVideoEncoder() = default;
    virtual std::optional<EncodedFrame> encode(const RawFrame &frame) = 0;
};

class DummyH264Encoder : public IVideoEncoder
{
public:
    std::optional<EncodedFrame> encode(const RawFrame &frame) override
    {
        EncodedFrame out;
        out.timestampMs = frame.timestampMs;
        out.keyframe = true;

        out.data.assign({std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x05},
                         std::byte{0x65}, std::byte{0x88}, std::byte{0x84}, std::byte{0x21},
                         std::byte{0xA0}});
        return out;
    }
};

} // namespace otherside
