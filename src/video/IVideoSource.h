#pragma once

#include <vector>

struct VideoFrame {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> data;
};

class IVideoSource {
    public:
    virtual ~IVideoSource() = default;

    virtual bool nextFrame(VideoFrame &out) = 0;
};
