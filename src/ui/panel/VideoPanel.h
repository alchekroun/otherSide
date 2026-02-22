#pragma once

#include "IPanel.h"

#include <imgui.h>
#include <memory>
#include <raylib.h>

#include "media/FrameFeed.h"

namespace otherside
{

class VideoPanel : public IPanel
{
public:
    VideoPanel(const std::shared_ptr<FrameFeed> &frameFeed_) : _frameFeed(frameFeed_) {};

    void render() override;

private:
    void updateTexture(RawFrame &frame);

    std::shared_ptr<FrameFeed> _frameFeed;
    Texture2D _texture{};
    bool _textureReady = false;
};

} // namespace otherside
