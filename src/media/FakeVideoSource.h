#pragma once

#include <thread>

#include "IVideoSource.h"

namespace otherside
{

class FakeVideoSource final : public IVideoSource
{
public:
    FakeVideoSource(size_t w, size_t h, uint8_t fps) : width(w), height(h), fps(fps)
    {
    }

    ~FakeVideoSource() override
    {
        stop();
    }

    void start() override;
    void stop() override;

private:
    size_t width;
    size_t height;
    uint8_t fps;
};
} // namespace otherside
