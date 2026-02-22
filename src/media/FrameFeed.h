#pragma once

#include "Frame.h"

namespace otherside
{
class FrameFeed
{
public:
    void push(const RawFrame &frame)
    {
        std::scoped_lock lock(mutex_);
        latest_ = frame;
        hasNew_ = true;
    }

    bool pop(RawFrame &out)
    {
        std::scoped_lock lock(mutex_);
        if (!hasNew_)
        {
            return false;
        }
        out = latest_;
        hasNew_ = false;
        return true;
    }

private:
    std::mutex mutex_;
    RawFrame latest_;
    bool hasNew_ = false;
};
} // namespace otherside
