#pragma once

#include "IVideoSource.h"
#include <atomic>
#include <chrono>
#include <thread>

class FakeVideoSource : public IVideoSource {
    public:
    FakeVideoSource(uint32_t w_ = 160, uint32_t h_ = 120, int fps_ = 5) : _width(w_), _height(h_), _fps(fps_) {};

    bool nextFrame(VideoFrame& out) override {
        const int sleepMs = 1000 / std::max(1, _fps);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));

        out.width = _width;
        out.height = _height;
        out.data.resize(_width * _height * 3);

        uint32_t f = frameCounter.fetch_add(1);
        for (uint32_t y = 0; y < _height; ++y) {
            for (uint32_t x = 0; x < _width; ++x) {
                size_t idx = (y * _width + x) * 3;
                out.data[idx + 0] = static_cast<uint8_t>((x + f) & 0xFF); // R
                out.data[idx + 1] = static_cast<uint8_t>((y + f) & 0xFF); // G
                out.data[idx + 2] = static_cast<uint8_t>((x + y + f) & 0xFF); // B
            }
        }
        return true;
    }

    private:
    uint32_t _width;
    uint32_t _height;
    int _fps;
    std::atomic<uint32_t> frameCounter{0};
};
