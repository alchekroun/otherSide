#pragma once

#include "IPanel.h"

#include <imgui.h>
#include <memory>
#include <raylib.h>

#include "media/include/FrameFeed.h"

namespace otherside
{

class VideoPanel : public IPanel
{
public:
    VideoPanel(const std::shared_ptr<FrameFeed> &frameFeed_) : _frameFeed(frameFeed_) {};

    void render() override
    {
        ImGui::BeginChild("VideoPanel", ImVec2(0, 0));
        RawFrame frame;
        if (_frameFeed->pop(frame))
        {
            updateTexture(frame);
        }

        if (_textureReady)
        {
            DrawTexture(_texture, 0, 0, WHITE);
        }
        ImGui::EndChild();
    }

private:
    void updateTexture(RawFrame &frame)
    {
        Image image{.data = frame.data.data(),
                    .width = (int)frame.width,
                    .height = (int)frame.height,
                    .mipmaps = 1,
                    .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};

        if (!_textureReady || _texture.width != image.width || _texture.height != image.height)
        {
            if (_textureReady)
            {
                UnloadTexture(_texture);
            }
            _texture = LoadTextureFromImage(image);
            _textureReady = true;
            return;
        }

        UpdateTexture(_texture, image.data);
    }

    std::shared_ptr<FrameFeed> _frameFeed;
    Texture2D _texture{};
    bool _textureReady = false;
};

} // namespace otherside
