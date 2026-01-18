#pragma once
#include "IScreen.h"
#include <imgui.h>

namespace otherside {

class ClientScreen : public IScreen {
public:
    ClientScreen(EventSink sink, std::shared_ptr<IMessageFeed> rxFeed_, std::shared_ptr<UiMessageFeed> txFeed_) :
    IScreen(sink), _messagePanel(PeerId::CLIENT, rxFeed_, txFeed_)
    {}

    void render() override {
        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Client", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0,1,0,1), "Status : Connected");

        ImGui::Separator();

        _messagePanel.render();

        ImGui::End();
    }

private:
    MessagePanel _messagePanel;
};

}
