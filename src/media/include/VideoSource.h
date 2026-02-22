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

class FakeVideoSource final : public IVideoSource
{
public:
    FakeVideoSource(size_t w, size_t h, uint8_t fps) : width(w), height(h), fps(fps)
    {
    }

    void start() override
    {
        running_ = true;
        thread_ = std::thread([this] {
            while (running_)
            {
                RawFrame frame;
                frame.width = static_cast<uint32_t>(width);
                frame.height = static_cast<uint32_t>(height);
                frame.format = PixelFormat::RGBA;
                frame.timestampMs = utils::nowMs();

                frame.data.resize(width * height * 4);
                for (size_t i = 0; i + 3 < frame.data.size(); i += 4)
                {
                    frame.data[i + 0] = std::byte{128};
                    frame.data[i + 1] = std::byte{128};
                    frame.data[i + 2] = std::byte{128};
                    frame.data[i + 3] = std::byte{255};
                }

                if (sink_)
                {
                    sink_(frame);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
            }
        });
    }

    void stop() override
    {
        running_ = false;
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

private:
    size_t width;
    size_t height;
    uint8_t fps;
};

} // namespace otherside
