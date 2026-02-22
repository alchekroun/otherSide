#include "VideoDecoder.h"

#include <cassert>
#include <iostream>

namespace otherside
{

VideoDecoder::VideoDecoder()
{
    av_log_set_level(AV_LOG_WARNING);
    initDecoder();
}

VideoDecoder::~VideoDecoder()
{
    if (_sws)
        sws_freeContext(_sws);

    if (_frame)
        av_frame_free(&_frame);

    if (_pkt)
        av_packet_free(&_pkt);

    if (_ctx)
        avcodec_free_context(&_ctx);
}

void VideoDecoder::setFrameCallback(FrameCallback cb)
{
    _onFrame = std::move(cb);
}

void VideoDecoder::initDecoder()
{
    _codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    assert(_codec && "H.264 decoder not found");

    _ctx = avcodec_alloc_context3(_codec);
    assert(_ctx);

    // Low-latency / WebRTC-friendly settings
    _ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    _ctx->thread_count = 1;
    _ctx->delay = 0;

    int ret = avcodec_open2(_ctx, _codec, nullptr);
    assert(ret >= 0 && "Failed to open H.264 decoder");

    _pkt = av_packet_alloc();
    _frame = av_frame_alloc();
}

void VideoDecoder::pushEncodedFrame(const uint8_t *data, size_t size)
{
    decodePacket(data, size);
}

void VideoDecoder::decodePacket(const uint8_t *data, size_t size)
{
    _pkt->data = const_cast<uint8_t *>(data);
    _pkt->size = static_cast<int>(size);

    int ret = avcodec_send_packet(_ctx, _pkt);
    if (ret < 0)
    {
        // Packet rejected (corrupt / late) → drop
        return;
    }

    while (true)
    {
        ret = avcodec_receive_frame(_ctx, _frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        if (ret < 0)
        {
            // Decoder error → drop frame
            break;
        }

        handleDecodedFrame(_frame);
    }
}

void VideoDecoder::handleDecodedFrame(AVFrame *frame)
{
    const int width = frame->width;
    const int height = frame->height;

    if (!_sws)
    {
        _sws =
            sws_getContext(width, height, static_cast<AVPixelFormat>(frame->format), width, height,
                           AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        assert(_sws);
    }

    const size_t rgbaSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    if (_rgbBuffer.size() != rgbaSize)
        _rgbBuffer.resize(rgbaSize);

    uint8_t *dstData[1] = {_rgbBuffer.data()};
    int dstLinesize[1] = {width * 4};

    sws_scale(_sws, frame->data, frame->linesize, 0, height, dstData, dstLinesize);

    if (_onFrame)
    {
        _onFrame(_rgbBuffer.data(), width, height);
    }
}

} // namespace otherside
