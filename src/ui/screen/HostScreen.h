#pragma once
#include <imgui.h>

#include "IScreen.h"
#include "../panel/MessagePanel.h"

namespace otherside {

class HostScreen : public IScreen {
public:
    HostScreen(EventSink sink, std::shared_ptr<IMessageFeed> rxFeed, std::shared_ptr<UiMessageFeed> txFeed) :
    IScreen(sink), _messagePanel(rxFeed), _inputMessagePanel(txFeed)
    {}

    void render() override {
        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Host", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0,1,0,1), "Status : Connected");

        ImGui::Separator();

        _messagePanel.render();

        ImGui::Separator();

        _inputMessagePanel.render();

        ImGui::End();
    }

private:
    MessagePanel _messagePanel;
    InputMessagePanel _inputMessagePanel;
};

}
