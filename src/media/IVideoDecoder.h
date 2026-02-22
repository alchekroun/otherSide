#pragma once

#include <optional>

#include "Frame.h"

namespace otherside
{

class IVideoDecoder
{
public:
    using FrameCallback = std::function<void(const uint8_t *rgb, int width, int height)>;
    virtual ~IVideoDecoder() = default;
    virtual void decode(const uint8_t *data, size_t size) = 0;
    virtual void setFrameCallback(FrameCallback cb) = 0;
};

} // namespace otherside
