#pragma once

#include <vector>

#include "utils.h"

namespace otherside
{

enum class PixelFormat : uint8_t
{
    RGBA,
    I420
};

struct RawFrame
{
    uint32_t width;
    uint32_t height;
    PixelFormat format;             // e.g. RGBA, I420, NV12
    utils::TimestampMs timestampMs; // capture time
    rtc::binary data;
};

struct EncodedFrame
{
    rtc::binary data;
    utils::TimestampMs timestampMs;
    bool keyframe;
};

} // namespace otherside
