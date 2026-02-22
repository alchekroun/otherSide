#include "VideoPanel.h"

namespace otherside
{

void VideoPanel::render()
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

void VideoPanel::updateTexture(RawFrame &frame)
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

} // namespace otherside
