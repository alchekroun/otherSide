#pragma once

#include "IVideoSource.h"

class VideoProcessor {
    public:
    VideoProcessor() = default;

    bool process(const VideoFrame& in, VideoFrame& out) {
        out = in;
        return true;
    }
};
