#pragma once

#include <cstdint>
#include <functional>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace otherside
{
class VideoDecoder
{
public:
    using FrameCallback = std::function<void(const uint8_t *rgb, int width, int height)>;

    VideoDecoder();
    ~VideoDecoder();

    // Non-copyable
    VideoDecoder(const VideoDecoder &) = delete;
    VideoDecoder &operator=(const VideoDecoder &) = delete;

    void setFrameCallback(FrameCallback cb);

    // Call this directly from _track->onMessage()
    void pushEncodedFrame(const uint8_t *data, size_t size);

private:
    void initDecoder();
    void decodePacket(const uint8_t *data, size_t size);
    void handleDecodedFrame(AVFrame *frame);

    const AVCodec *_codec = nullptr;
    AVCodecContext *_ctx = nullptr;
    AVPacket *_pkt = nullptr;
    AVFrame *_frame = nullptr;
    SwsContext *_sws = nullptr;

    std::vector<uint8_t> _rgbBuffer;

    FrameCallback _onFrame;
};
} // namespace otherside