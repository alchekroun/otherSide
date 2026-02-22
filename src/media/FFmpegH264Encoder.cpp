#include "FFmpegH264Encoder.h"

#include <stdexcept>

namespace otherside
{

FFmpegH264Encoder::FFmpegH264Encoder(uint32_t width, uint32_t height, uint8_t fps)
    : _width(width), _height(height), _fps(fps)
{
    init();
}

FFmpegH264Encoder::~FFmpegH264Encoder()
{
    if (_rgbaToYuv)
        sws_freeContext(_rgbaToYuv);
    if (_pkt)
        av_packet_free(&_pkt);
    if (_yuvFrame)
        av_frame_free(&_yuvFrame);
    if (_ctx)
        avcodec_free_context(&_ctx);
}

void FFmpegH264Encoder::init()
{
    _codec = avcodec_find_encoder_by_name("libx264");
    if (!_codec)
    {
        _codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    }
    if (!_codec)
    {
        throw std::runtime_error("H264 encoder not found");
    }

    _ctx = avcodec_alloc_context3(_codec);
    if (!_ctx)
    {
        throw std::runtime_error("Failed to allocate H264 encoder context");
    }

    _ctx->width = static_cast<int>(_width);
    _ctx->height = static_cast<int>(_height);
    _ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    _ctx->time_base = AVRational{1, _fps};
    _ctx->framerate = AVRational{_fps, 1};
    _ctx->gop_size = static_cast<int>(_fps);
    _ctx->max_b_frames = 0;
    _ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;

    av_opt_set(_ctx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(_ctx->priv_data, "tune", "zerolatency", 0);
    av_opt_set(_ctx->priv_data, "repeat-headers", "1", 0);

    if (avcodec_open2(_ctx, _codec, nullptr) < 0)
    {
        throw std::runtime_error("Failed to open H264 encoder");
    }

    _yuvFrame = av_frame_alloc();
    if (!_yuvFrame)
    {
        throw std::runtime_error("Failed to allocate encoder frame");
    }

    _yuvFrame->format = _ctx->pix_fmt;
    _yuvFrame->width = _ctx->width;
    _yuvFrame->height = _ctx->height;
    if (av_frame_get_buffer(_yuvFrame, 32) < 0)
    {
        throw std::runtime_error("Failed to allocate YUV frame buffer");
    }

    _pkt = av_packet_alloc();
    if (!_pkt)
    {
        throw std::runtime_error("Failed to allocate encoder packet");
    }

    _rgbaToYuv = sws_getContext(_ctx->width, _ctx->height, AV_PIX_FMT_RGBA, _ctx->width,
                                _ctx->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, nullptr,
                                nullptr, nullptr);
    if (!_rgbaToYuv)
    {
        throw std::runtime_error("Failed to allocate RGBA->YUV conversion context");
    }
}

std::optional<EncodedFrame> FFmpegH264Encoder::encode(const RawFrame &frame)
{
    if (frame.format != PixelFormat::RGBA || frame.width != _width || frame.height != _height)
    {
        return std::nullopt;
    }

    if (av_frame_make_writable(_yuvFrame) < 0)
    {
        return std::nullopt;
    }

    const uint8_t *srcData[1] = {reinterpret_cast<const uint8_t *>(frame.data.data())};
    const int srcLinesize[1] = {static_cast<int>(_width * 4)};
    sws_scale(_rgbaToYuv, srcData, srcLinesize, 0, static_cast<int>(_height), _yuvFrame->data,
              _yuvFrame->linesize);

    _yuvFrame->pts = _nextPts++;

    if (avcodec_send_frame(_ctx, _yuvFrame) < 0)
    {
        return std::nullopt;
    }

    EncodedFrame out;
    out.timestampMs = frame.timestampMs;
    out.keyframe = true;

    while (avcodec_receive_packet(_ctx, _pkt) == 0)
    {
        out.data.insert(out.data.end(), reinterpret_cast<std::byte *>(_pkt->data),
                        reinterpret_cast<std::byte *>(_pkt->data + _pkt->size));
        av_packet_unref(_pkt);
    }

    if (out.data.empty())
    {
        return std::nullopt;
    }

    return out;
}

} // namespace otherside
