#pragma once

#include "media/IVideoEncoder.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace otherside
{

class H264Encoder : public IVideoEncoder
{
public:
    H264Encoder(uint32_t width, uint32_t height, uint8_t fps);
    ~H264Encoder() override;

    H264Encoder(const H264Encoder &) = delete;
    H264Encoder &operator=(const H264Encoder &) = delete;

    std::optional<EncodedFrame> encode(const RawFrame &frame) override;

private:
    void init();

    uint32_t _width;
    uint32_t _height;
    uint8_t _fps;

    const AVCodec *_codec = nullptr;
    AVCodecContext *_ctx = nullptr;
    AVFrame *_yuvFrame = nullptr;
    AVPacket *_pkt = nullptr;
    SwsContext *_rgbaToYuv = nullptr;
    int64_t _nextPts = 0;
};

} // namespace otherside
