#include "FakeVideoSource.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace otherside
{
void FakeVideoSource::start()
{
    running_ = true;
    thread_ = std::thread([this] {
        uint32_t frameIndex = 0;

        auto toByte = [](int v) -> std::byte {
            v = std::clamp(v, 0, 255);
            return static_cast<std::byte>(static_cast<uint8_t>(v));
        };

        auto hash = [](uint32_t x) -> uint32_t {
            x ^= x >> 16;
            x *= 0x7feb352dU;
            x ^= x >> 15;
            x *= 0x846ca68bU;
            x ^= x >> 16;
            return x;
        };

        while (running_)
        {
            RawFrame frame;
            frame.width = static_cast<uint32_t>(width);
            frame.height = static_cast<uint32_t>(height);
            frame.format = PixelFormat::RGBA;
            frame.timestampMs = utils::nowMs();

            frame.data.resize(width * height * 4);
            for (uint32_t y = 0; y < frame.height; ++y)
            {
                for (uint32_t x = 0; x < frame.width; ++x)
                {
                    const uint32_t n = hash((x * 73856093U) ^ (y * 19349663U) ^ (frameIndex * 83492791U));
                    const int noise = static_cast<int>(n & 0x1F);
                    const int r = static_cast<int>((x + frameIndex * 3U) % 256U) + noise;
                    const int g = static_cast<int>((y * 2U + frameIndex * 5U) % 256U) - noise;
                    const int b = static_cast<int>((x + y + frameIndex * 7U) % 256U);

                    const size_t i = static_cast<size_t>(y) * frame.width * 4 + static_cast<size_t>(x) * 4;
                    frame.data[i + 0] = toByte(r);
                    frame.data[i + 1] = toByte(g);
                    frame.data[i + 2] = toByte(b);
                    frame.data[i + 3] = std::byte{255};
                }
            }

            const std::array<std::array<uint8_t, 3>, 3> colors = {
                std::array<uint8_t, 3>{255, 64, 64},
                std::array<uint8_t, 3>{64, 255, 64},
                std::array<uint8_t, 3>{64, 64, 255}};

            for (size_t s = 0; s < colors.size(); ++s)
            {
                const uint32_t boxW = std::max<uint32_t>(24U, frame.width / 6U);
                const uint32_t boxH = std::max<uint32_t>(24U, frame.height / 6U);

                const uint32_t maxX = frame.width > boxW ? frame.width - boxW : 0U;
                const uint32_t maxY = frame.height > boxH ? frame.height - boxH : 0U;
                const uint32_t speedX = static_cast<uint32_t>(2U + s);
                const uint32_t speedY = static_cast<uint32_t>(3U + s * 2U);

                const uint32_t px = maxX == 0U ? 0U : (frameIndex * speedX + static_cast<uint32_t>(37U * s)) % maxX;
                const uint32_t py = maxY == 0U ? 0U : (frameIndex * speedY + static_cast<uint32_t>(53U * s)) % maxY;

                for (uint32_t y = py; y < py + boxH && y < frame.height; ++y)
                {
                    for (uint32_t x = px; x < px + boxW && x < frame.width; ++x)
                    {
                        const size_t i = static_cast<size_t>(y) * frame.width * 4 + static_cast<size_t>(x) * 4;
                        frame.data[i + 0] = static_cast<std::byte>(colors[s][0]);
                        frame.data[i + 1] = static_cast<std::byte>(colors[s][1]);
                        frame.data[i + 2] = static_cast<std::byte>(colors[s][2]);
                        frame.data[i + 3] = std::byte{255};
                    }
                }
            }

            if (sink_)
            {
                sink_(frame);
            }

            ++frameIndex;
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
