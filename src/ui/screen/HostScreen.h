#pragma once
#include <imgui.h>

#include "IScreen.h"
#include "../panel/MessagePanel.h"

namespace otherside {

class HostScreen : public IScreen {
public:
    HostScreen(EventSink sink, std::shared_ptr<IMessageFeed> feed) : IScreen(sink), _messagePanel(feed) {}

    void render() override {
        ImGui::Begin("Host");

        ImGui::TextColored(ImVec4(0,1,0,1), "Status : Connected");
        ImGui::Separator();

        _messagePanel.render();

        ImGui::End();
    }

private:
    MessagePanel _messagePanel;
};

}
