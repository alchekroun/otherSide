#pragma once

#include "../utils.h"
#include "Frame.h"
#include <rtc/rtc.hpp>
#include <thread>

namespace otherside
{

class IVideoSource
{
public:
    using FrameSink = std::function<void(const RawFrame &)>;

    virtual ~IVideoSource() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setSink(const FrameSink &sink)
    {
        sink_ = sink;
    }

protected:
    FrameSink sink_ = nullptr;
    std::atomic<bool> running_ = false;
    std::thread thread_;
};

} // namespace otherside
