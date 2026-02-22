#pragma once
#include <imgui.h>

#include "../panel/MessagePanel.h"
#include "IScreen.h"

namespace otherside
{

class HostScreen : public IScreen
{
public:
    HostScreen(EventSink sink, const std::shared_ptr<IMessageFeed> &rxFeed_,
               const std::shared_ptr<UiMessageFeed> &txFeed_,
               const std::shared_ptr<FrameFeed> &frameFeed_)
        : IScreen(std::move(sink)), _messagePanel(PeerId::HOST, rxFeed_, txFeed_),
          _videoPanel(frameFeed_)
    {
    }

    void render() override
    {
        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Host", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status : Connected");

        ImGui::Separator();

        _messagePanel.render();
        _videoPanel.render();

        ImGui::End();
    }

private:
    MessagePanel _messagePanel;
    VideoPanel _videoPanel;
};

} // namespace otherside
