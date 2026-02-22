#include "FakeVideoSource.h"

namespace otherside
{
void FakeVideoSource::start()
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

void FakeVideoSource::stop()
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

} // namespace otherside
