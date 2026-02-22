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

} // namespace otherside
